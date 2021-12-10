#include "Importing.h"
#include "brep_utils.h"

#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSControl_TransferReader.hxx>
#include <Transfer_TransientProcess.hxx>
#include <Message_ProgressIndicator.hxx>
#include <TopoDS.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeAnalysis.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <NCollection_Vector.hxx>
#include <NCollection_StlIterator.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Precision.hxx>
#include <BRepTools_ReShape.hxx>
#include <StepRepr_Representation.hxx>
#include <StepShape_ConnectedFaceSet.hxx>
#include <StepShape_AdvancedFace.hxx>
#include <StepShape_ManifoldSolidBrep.hxx>
#include <StepShape_Face.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDF_Label.hxx>
#include <TDataStd.hxx>
#include <TDataStd_Name.hxx>

#include <triangle_mesh.h>

#include <geom.h>

#include <stlutil/make_unique.h>
#include <stlutil/finally.h>

#include <algorithm>
#include <memory>

using namespace std;

cad_read_result_t cadread::read_cad_file(XSControl_Reader& reader, const string& filename, Handle(Message_ProgressIndicator) indicator)
{
	if (!indicator.IsNull())
	{
		//auto ws = reader.WS();
		//ws->MapReader()->SetProgress(indicator);

		indicator->SetScale("Reading CAD file", 0.0, 100.0, 1.0);
		indicator->NewScope(20, "Reading");

		indicator->Show();
	}

	IFSelect_ReturnStatus res = reader.ReadFile(filename.c_str());

	if (!indicator.IsNull())
		indicator->EndScope();

	// Try to read entity names from the model
	Handle(XSControl_WorkSession) ws = reader.WS();
	Handle(Interface_InterfaceModel) model = ws->Model();

	auto nEntities = model->NbEntities();
	for (Standard_Integer i = 1 ; i <= nEntities ; i++)
	{
		auto entity = model->Value(i);
		auto stepEntity = Handle(StepRepr_Representation)::DownCast(entity);
		if (!stepEntity.IsNull())
		{
			if (!stepEntity->Name().IsNull())
				std::cout << "Got entity " << stepEntity->DynamicType()->Name()
					<< " name \"" << stepEntity->Name()->ToCString() << "\"" << std::endl;

			for (auto j = 1 ; j <= stepEntity->NbItems() ; j++)
			{
				auto stepReprItem = stepEntity->ItemsValue(j);
				if (!stepReprItem->Name().IsNull())
					std::cout << "\tGot item " << stepReprItem->DynamicType()->Name()
						<< " name \"" << stepReprItem->Name()->ToCString() << "\"" << std::endl;	

				// If this is a solid then get the faces
				auto manifoldSolidBrep = 
					Handle(StepShape_ManifoldSolidBrep)::DownCast(stepReprItem);

				if (!manifoldSolidBrep.IsNull())
				{
					auto outerFaceSet = manifoldSolidBrep->Outer();
					
					for (Standard_Integer k = 1 ; k < outerFaceSet->NbCfsFaces() ; k++)
					{
						auto face = outerFaceSet->CfsFacesValue(k);
						if (!face->Name().IsNull())
						{
							std::cout << "\t\tGot face " << face->DynamicType()->Name()
								<< " name \"" << face->Name()->ToCString() << "\"" << std::endl;
						}
					}
				}
			}
		}
	}

	if (res != IFSelect_RetDone)
	{
		return make_pair(false, TopoDS_Shape());
	}

	if (!indicator.IsNull())
	{
		ws->TransferReader()->TransientProcess()->SetProgress(indicator);

		indicator->NewScope(80, "Importing");
		indicator->Show();
	}

	reader.TransferRoots();

	if (!indicator.IsNull())
		indicator->EndScope();

	return make_pair(true, reader.OneShape());
}

cad_read_result_t cadread::ReadSTEP(const string& filename, Handle(Message_ProgressIndicator) progress_indicator)
{
	STEPControl_Reader reader;
	cad_read_result_t res = read_cad_file(reader, filename, progress_indicator);

	const TopoDS_Shape& shape = res.second;
	if (!shape.IsNull())
		res.second = heal_BRep(shape, progress_indicator);

	return res;
}

cad_read_result_t cadread::ReadIGES(const string& filename, Handle(Message_ProgressIndicator) progress_indicator)
{
	IGESControl_Reader reader;
	cad_read_result_t res = read_cad_file(reader, filename, progress_indicator);

	const TopoDS_Shape& shape = res.second;
	if (!shape.IsNull())
		res.second = heal_BRep(shape, progress_indicator);

	return res;
}

void cadread::TagFaces(Handle(TDocStd_Document) doc)
{
	auto shape_tool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
	
	TDF_LabelSequence shapeLabels;
	shape_tool->GetFreeShapes(shapeLabels);

	int i = 0;
	for (TDF_Label & label : shapeLabels)
	{
		TopoDS_Shape S = shape_tool->GetShape(label);

		auto faces = brep_utils::get_topo<TopoDS_Face>(S);
		for (TopoDS_Face & f : faces)
		{
			TDF_Label fLabel = shape_tool->AddSubShape(label, f);

			std::ostringstream oss;
			oss << "face_" << (i++);

			TDataStd_Name::Set(fLabel, oss.str().c_str());
		}
	}
}

TopoDS_Shape cadread::heal_BRep(const TopoDS_Shape& shape, Handle(Message_ProgressIndicator) indicator)
{
	Handle(ShapeFix_Shape) shape_fix(new ShapeFix_Shape);
	shape_fix->Init(shape);

	auto vertices = brep_utils::get_topo<TopoDS_Vertex>(shape);

	if (std::distance(vertices.begin(), vertices.end()) < 2)
		return shape;

	auto minmax_vert_tolerance = std::minmax_element(vertices.begin(), vertices.end(),
		[](const TopoDS_Vertex& v1, const TopoDS_Vertex& v2)
		{
			return BRep_Tool::Tolerance(v1) < BRep_Tool::Tolerance(v2);
		});

	Standard_Real min_vert_tolerance = BRep_Tool::Tolerance(*minmax_vert_tolerance.first);
	Standard_Real max_vert_tolerance = BRep_Tool::Tolerance(*minmax_vert_tolerance.second);

	shape_fix->SetMinTolerance(min_vert_tolerance);
	shape_fix->SetMaxTolerance(max_vert_tolerance);

	if (!indicator.IsNull())
	{
		indicator->Reset();
		indicator->SetScale("Healing", 0, 100, 1);
		indicator->NewScope(50, "Fixing shape");
	}

	shape_fix->Perform(indicator);
	TopoDS_Shape fixed_shape = shape_fix->Shape();

	if (!indicator.IsNull())
	{
		indicator->EndScope();
		indicator->NewScope(50, "Fixing wires");
	}

	BRepTools_ReShape reshaper;
	for (const TopoDS_Face& face : brep_utils::get_topo<TopoDS_Face>(fixed_shape))
	{
		stlutil::finally f([&indicator]() { indicator->Increment(); });

		BRepCheck_Analyzer face_analyzer(face);
		if (face_analyzer.IsValid())
			continue;

		TopoDS_Wire outer_loop = ShapeAnalysis::OuterWire(face);
		TopTools_IndexedMapOfShape shape_map;
		shape_map.Add(outer_loop);

		auto wires = brep_utils::get_topo<TopoDS_Wire>(face);

		ShapeFix_Wire fix_wire;
		fix_wire.SetFace(face);
		fix_wire.Load(outer_loop);
		fix_wire.Perform();

		BRepBuilderAPI_MakeFace make_face(fix_wire.WireAPIMake());

		for (const TopoDS_Wire& wire : wires)
		{
			if (!shape_map.Contains(wire))
			{
				fix_wire.Load(wire);
				fix_wire.Perform();

				make_face.Add(fix_wire.WireAPIMake());
			}
		}

		auto mk_face = make_face.Face();
		if (mk_face.IsNull())
			continue;

		face_analyzer.Init(make_face.Face());
		if (face_analyzer.IsValid())
		{
			reshaper.Apply(make_face.Face());
			continue;
		}

		ShapeFix_Shape fix(make_face.Face());
		fix.SetPrecision(Precision::Confusion());
		fix.SetMinTolerance(Precision::Confusion());
		fix.SetMaxTolerance(Precision::Confusion());

		fix.Perform();
		fix.FixWireTool()->Perform();
		fix.FixFaceTool()->Perform();

		fixed_shape = reshaper.Apply(fix.Shape());
	}

	return fixed_shape;
}

Standard_Real cadread::compute_optimal_linear_deflection(const TopoDS_Shape& shape)
{
	double const default_dev_coef = 1e-3;
	double const max_chordial_dev = 5000000;

	Bnd_Box occt_bbox;
	BRepBndLib::Add(shape, occt_bbox);

	if (occt_bbox.IsVoid())
		return max_chordial_dev;

	double xmin, xmax, ymin, ymax, zmin, zmax;
	occt_bbox.Get(xmin, xmax, ymin, ymax, zmin, zmax);
	maths::bbox3d bbox(maths::vector3d(xmin, ymin, zmin), maths::vector3d(xmax, ymax, zmax));

	return bbox.max_extent() * default_dev_coef * 4.0;
}

unique_ptr<triangle_mesh> cadread::tessellate_BRep(const TopoDS_Shape& shape, brep_mesh_params params)
{
	if (shape.IsNull())
		return nullptr;

	if (params.linear_deflection == 0)
		params.linear_deflection = compute_optimal_linear_deflection(shape);

	if (params.linear_deflection <= 0)
		throw std::runtime_error("Bad linear deflection value!");
	if (params.angle_deflection_rad <= 0)
		throw std::runtime_error("Bad angular deflection value!");

	// have to do it like this for compatibility between OCCT versions
	BRepMesh_IncrementalMesh mesher(shape,
									params.linear_deflection,
									params.use_relative_discretization,
									params.angle_deflection_rad,
									params.use_parallel_meshing);

	auto mesh = std::make_unique<triangle_mesh>();

	GProp_GProps shape_gprops;
	BRepGProp::VolumeProperties(shape, shape_gprops);
	bool const is_inside_out = shape_gprops.Mass() < 0.0;

	// Get the triangulation of each face, send to mesh
	auto faces = brep_utils::get_topo<TopoDS_Face>(shape.Oriented(is_inside_out ? TopAbs_REVERSED : TopAbs_FORWARD));

	for (const TopoDS_Face& face : faces)
	{
		const bool face_reversed = face.Orientation() != TopAbs_FORWARD;

		TopLoc_Location face_location;
		Handle(Poly_Triangulation) face_triangulation = BRep_Tool::Triangulation(face, face_location);
		if (face_triangulation.IsNull())
			continue;

		const Poly_Array1OfTriangle& face_triangles = face_triangulation->Triangles();
		const TColgp_Array1OfPnt& face_nodes = face_triangulation->Nodes();

		for (Standard_Integer i = 1 ; i <= face_triangulation->NbTriangles() ; i++)
		{
			Standard_Integer i1, i2, i3;
			const Poly_Triangle& ft = face_triangles.Value(i);
			ft.Get(i1, i2, i3);

			gp_Pnt t_p1 = face_nodes.Value(!face_reversed ? i1 : i2).Transformed(face_location);
			gp_Pnt t_p2 = face_nodes.Value(!face_reversed ? i2 : i1).Transformed(face_location);
			gp_Pnt t_p3 = face_nodes.Value(i3).Transformed(face_location);

			maths::triangle3d t(maths::vector3d(t_p1.X(), t_p1.Y(), t_p1.Z()),
								maths::vector3d(t_p2.X(), t_p2.Y(), t_p2.Z()),
								maths::vector3d(t_p3.X(), t_p3.Y(), t_p3.Z()));

			mesh->add_triangle(t);
		}
	}

	return mesh;
}
