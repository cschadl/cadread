#include "Exporting.h"
#include "brep_utils.h"

#include <string>
#include <sstream>

#include <TopoDS_Shape.hxx>
#include <XCAFApp_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSControl_TransferWriter.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <STEPControl_Writer.hxx>
#include <Interface_InterfaceModel.hxx>
#include <StepShape_ManifoldSolidBrep.hxx>
#include <StepShape_Face.hxx>
#include <StepShape_ConnectedFaceSet.hxx>
#include <StepRepr_RepresentationItem.hxx>
#include <StepRepr_Representation.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelSequence.hxx>
#include <Transfer_FinderProcess.hxx>
#include <TransferBRep.hxx>
#include <TransferBRep_ShapeMapper.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Attribute.hxx>

bool cadread::ExportSTEP(TopoDS_Shape const& shape, std::filesystem::path const& out_path)
{
    /*
    Handle(TDocStd_Document) doc;
    auto app = XCAFApp_Application::GetApplication();
    app->NewDocument("MDTV-XCAF", doc);

    auto shapeTool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
    auto shapeLabel = shapeTool->AddShape(shape);
    */
    
    Handle(XSControl_WorkSession) WS(new XSControl_WorkSession);
    STEPControl_Writer writer(WS);

    writer.Transfer(shape, STEPControl_StepModelType::STEPControl_AsIs);

    auto model = WS->Model();
    auto nEntities = model->NbEntities();
	for (Standard_Integer i = 1 ; i <= nEntities ; i++)
	{
		auto entity = model->Value(i);
		auto stepEntity = Handle(StepRepr_Representation)::DownCast(entity);
		if (!stepEntity.IsNull())
		{
            for (Standard_Integer j = 1 ; j <= stepEntity->NbItems() ; j++)
            {
                auto stepReprItem = stepEntity->ItemsValue(j);

                // If this is a solid then get the faces
				auto manifoldSolidBrep = 
					Handle(StepShape_ManifoldSolidBrep)::DownCast(stepReprItem);

				if (!manifoldSolidBrep.IsNull())
				{
					auto outerFaceSet = manifoldSolidBrep->Outer();
					
					for (Standard_Integer k = 1 ; k < outerFaceSet->NbCfsFaces() ; k++)
					{
                        // Assign a name to each face
						auto face = outerFaceSet->CfsFacesValue(k);
                        
                        std::ostringstream oss;
                        oss << "FACE" << k;

                        face->SetName(new TCollection_HAsciiString(oss.str().c_str()));
                    }
                }
            }
        }
    }

    auto write_staus = writer.Write(out_path.c_str());

    return write_staus == IFSelect_ReturnStatus::IFSelect_RetDone;
}

bool cadread::ExportSTEPXDE(Handle(TDocStd_Document) doc, std::filesystem::path const& out_path)
{
    if (!XCAFDoc_DocumentTool::IsXCAFDocument(doc))
    {
        std::cerr << "Document must be an XCAF doc" << std::endl;
        return false;
    }

    auto shape_tool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());

    Handle(XSControl_WorkSession) WS(new XSControl_WorkSession);
    STEPCAFControl_Writer writer;

    writer.Transfer(doc);

    TDF_LabelSequence shapeLabels;
	shape_tool->GetFreeShapes(shapeLabels);

    auto fp = WS->TransferWriter()->FinderProcess();

    for (TDF_Label const& label_S : shapeLabels)
    {
        TopoDS_Shape S = shape_tool->GetShape(label_S);

        for (TopoDS_Face const& f : brep_utils::get_topo<TopoDS_Face>(S))
        {
            auto shapeMapper = TransferBRep::ShapeMapper(fp, f);
            TDF_Label f_label = shape_tool->FindShape(f);
            if (f_label.IsNull())
                continue;

            Handle(Standard_Transient) faceEntity = fp->FindElseBind(shapeMapper);
            if (!faceEntity.IsNull())
            {
                auto faceItem = Handle(StepRepr_RepresentationItem)::DownCast(faceEntity);
                if (!faceItem.IsNull())
                {
                    Handle(TDataStd_Name) nameAttr(new TDataStd_Name);
                    if (f_label.FindAttribute(TDataStd_Name::GetID(), nameAttr))
                        faceItem->SetName(new TCollection_HAsciiString(nameAttr->Get()));
                }
            }
        }
    }

    auto writeResult = writer.Write(out_path.c_str());

    return writeResult == IFSelect_RetDone;
}