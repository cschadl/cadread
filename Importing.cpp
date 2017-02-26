#include "Importing.h"

#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSControl_TransferReader.hxx>
#include <Transfer_TransientProcess.hxx>
#include <Message_ProgressIndicator.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <BRep_Tool.hxx>

#include <triangle_mesh.h>

#include <make_unique.h>

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

	if (res != IFSelect_RetDone)
	{
		return make_pair(false, TopoDS_Shape());
	}

	if (!indicator.IsNull())
	{
		auto ws = reader.WS();
		ws->TransferReader()->TransientProcess()->SetProgress(indicator);

		indicator->NewScope(80, "Importing");
		indicator->Show();
	}

	reader.TransferRoots();

	if (!indicator.IsNull())
		indicator->EndScope();

	return make_pair(true, reader.Shape());
}

cad_read_result_t cadread::ReadSTEP(const string& filename, Handle(Message_ProgressIndicator) progress_indicator)
{
	STEPControl_Reader reader;
	return read_cad_file(reader, filename, progress_indicator);
}

cad_read_result_t cadread::ReadIGES(const string& filename, Handle(Message_ProgressIndicator) progress_indicator)
{
	IGESControl_Reader reader;
	cad_read_result_t res = read_cad_file(reader, filename, progress_indicator);

	// TODO - Heal

	return res;
}

unique_ptr<triangle_mesh> cadread::tessellate_BRep(const TopoDS_Shape& shape, const brep_mesh_params& params)
{
	if (shape.IsNull())
		return nullptr;

	// have to do it like this for compatibility between OCCT versions
	BRepMesh_IncrementalMesh mesher(shape,
									params.linear_deflection,
									params.use_relative_discretization,
									params.angle_deflection,
									params.use_parallel_meshing);

	auto mesh = stlutil::make_unique<triangle_mesh>();

	// Get the triangluation of each face, send to mesh

	GProp_GProps shape_gprops;
	BRepGProp::VolumeProperties(shape, shape_gprops);
	bool const is_inside_out = shape_gprops.Mass() < 0.0;

	TopExp_Explorer exp_brep_faces(shape.Oriented(is_inside_out ? TopAbs_REVERSED : TopAbs_FORWARD), TopAbs_FACE);
	for (; exp_brep_faces.More() ; exp_brep_faces.Next())
	{
		const TopoDS_Face & face = TopoDS::Face(exp_brep_faces.Current());
		const bool face_reversed = face.Orientation() != TopAbs_FORWARD;

		TopLoc_Location face_location;
		Handle(Poly_Triangulation) face_triangulation = BRep_Tool::Triangulation(face, face_location);
		if (face_triangulation.IsNull())
			continue;

		const Poly_Array1OfTriangle & face_triangles = face_triangulation->Triangles();
		const TColgp_Array1OfPnt & face_nodes = face_triangulation->Nodes();

		for (Standard_Integer i = 1 ; i <= face_triangulation->NbTriangles() ; i++)
		{
			Standard_Integer i1, i2, i3;
			const Poly_Triangle & ft = face_triangles.Value(i);
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






















