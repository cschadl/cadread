#ifndef _CADREAD_IMPORTING_H_
#define _CADREAD_IMPORTING_H_

#include <Standard_DefineHandle.hxx>
#include <TopoDS_Shape.hxx>

#include <string>
#include <utility>
#include <memory>

class XSControl_Reader;
class TopoDS_Shape;
class Message_ProgressRange;
class triangle_mesh;

typedef std::pair<bool, TopoDS_Shape> cad_read_result_t;

struct brep_mesh_params
{
	double linear_deflection;		// Linear deflection
	bool use_relative_discretization;
	double angle_deflection_rad;	// Angular deflection in radians
	bool use_parallel_meshing;
};

namespace cadread
{

cad_read_result_t read_cad_file(XSControl_Reader& reader, const std::string& filename, Message_ProgressRange& progress_indicator);

cad_read_result_t ReadSTEP(const std::string& filename, Message_ProgressRange& progress_indicator);
cad_read_result_t ReadIGES(const std::string& filename, Message_ProgressRange& progress_indicator);

Standard_Real compute_optimal_linear_deflection(const TopoDS_Shape& shape);

TopoDS_Shape heal_BRep(const TopoDS_Shape& shape, Message_ProgressRange& progress_indicator);

std::unique_ptr<triangle_mesh> tessellate_BRep(const TopoDS_Shape& shape, brep_mesh_params params);

};
#endif
