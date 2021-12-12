#ifndef _CADREAD_IMPORTING_H_
#define _CADREAD_IMPORTING_H_

#include <Standard_DefineHandle.hxx>
#include <Message_ProgressIndicator.hxx>
#include <TopoDS_Shape.hxx>

#include <string>
#include <utility>
#include <memory>

class XSControl_Reader;
class TopoDS_Shape;
class triangle_mesh;
class TDocStd_Document;

typedef std::pair<bool, TopoDS_Shape> cad_read_result_t;

struct brep_mesh_params
{
    double linear_deflection;           // Linear deflection
    bool use_relative_discretization;
    double angle_deflection_rad;        // Angular deflection in radians
    bool use_parallel_meshing;
};

namespace cadread
{

cad_read_result_t read_cad_file(XSControl_Reader& reader, const std::string& filename, Handle(Message_ProgressIndicator) progress_indicator);

cad_read_result_t ReadSTEP(const std::string& filename, Handle(Message_ProgressIndicator) progress_indicator);
cad_read_result_t ReadIGES(const std::string& filename, Handle(Message_ProgressIndicator) progress_indicator);

void TagShapes(Handle(TDocStd_Document) shapeDoc);

Standard_Real compute_optimal_linear_deflection(const TopoDS_Shape& shape);

TopoDS_Shape heal_BRep(const TopoDS_Shape& shape, Handle(Message_ProgressIndicator) progress_indicator);

std::unique_ptr<triangle_mesh> tessellate_BRep(const TopoDS_Shape& shape, brep_mesh_params params);

};
#endif
