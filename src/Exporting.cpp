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
#include <Interface_Static.hxx>
#include <TDF_ChildIDIterator.hxx>

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

namespace
{
    bool get_label_name(TDF_Label const& L, Handle(TCollection_HAsciiString) & str)
    {
        Handle(TDataStd_Name) N;
        if ( ! L.FindAttribute ( TDataStd_Name::GetID(), N ) ) return Standard_False;
        TCollection_ExtendedString name = N->Get();
        if ( name.Length() <=0 ) return Standard_False;

        // set name, removing spaces around it
        TCollection_AsciiString buf(name);
        buf.LeftAdjust();
        buf.RightAdjust();
        str->AssignCat ( buf.ToCString() );
        return Standard_True;
    }
}

bool cadread::ExportSTEPXDE(Handle(TDocStd_Document) doc, std::filesystem::path const& out_path)
{
    if (!XCAFDoc_DocumentTool::IsXCAFDocument(doc))
    {
        std::cerr << "Document must be an XCAF doc" << std::endl;
        return false;
    }

    Handle(XSControl_WorkSession) WS(new XSControl_WorkSession);
    STEPCAFControl_Writer writer;

    writer.Transfer(doc);

    auto fp = WS->TransferWriter()->FinderProcess();
    auto shape_tool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());

    WS->ComputeGraph(Standard_True);
    
    TDF_LabelSequence freeShapeLabels;
    shape_tool->GetFreeShapes(freeShapeLabels);

    for (TDF_Label const& fsLabel : freeShapeLabels)
    {
        Handle(TCollection_HAsciiString) fsLabelName(new TCollection_HAsciiString);
        if (get_label_name(fsLabel, fsLabelName))
            std::cout << "Free-shape label name is \"" << fsLabelName->ToCString() << "\"" << std::endl;

        TopoDS_Shape fs = shape_tool->GetShape(fsLabel);

        auto fsMapper = TransferBRep::ShapeMapper(fp, fs);
        Handle(Standard_Transient) fsEntity = fp->FindTransient(fsMapper);
        if (!fsEntity.IsNull())
        {
            std::cout << "Got Free Shape entity " << fsEntity->DynamicType()->Name() << std::endl;
        }
        
        TDF_LabelSequence subLabels;
        shape_tool->GetSubShapes(fsLabel, subLabels);
        for (TDF_Label const& subLabel : subLabels)
        {
            Handle(TCollection_HAsciiString) ssLabelName(new TCollection_HAsciiString);
            if (get_label_name(subLabel, ssLabelName))
                std::cout << "Sublabel name is \"" << ssLabelName->ToCString() << "\"" << std::endl;

            for (TDF_ChildIterator ss_it(subLabel, Standard_True); ss_it.More(); ss_it.Next())
            {
                TopoDS_Shape subShape = shape_tool->GetShape(ss_it.Value());

                auto ssMapper = TransferBRep::ShapeMapper(fp, subShape);
                Handle(Standard_Transient) ssEntity = fp->FindTransient(ssMapper);
                if (!ssEntity.IsNull())
                {
                    std::cout << "Got subshape entity " 
                        << ssEntity->DynamicType()->Name() << std::endl;
                }
            }
        }
    }

    auto writeResult = writer.Write(out_path.c_str());

    return writeResult == IFSelect_RetDone;
}