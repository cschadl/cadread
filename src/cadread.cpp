#include <iostream>
#include <algorithm>
#include <cctype>
#include <memory>
#include <fstream>

// for M_PI
#undef __STRICT_ANSI__
#include <cmath>

#include <triangle_mesh.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <TopoDS_Shape.hxx>
#include <Message_ProgressIndicator.hxx>
#include <Standard_Version.hxx>

#include "Importing.h"
#include "cadread_ConsoleProgressIndicator.h"

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

static const char* INPUT_FILE_ARG = "input_file";
static const char* OUTPUT_FILE_ARG = "output_file";
static const char* LINEAR_DEFLECTION_ARG = "linear_deflection,d";
static const char* ANGULAR_DEFLECTION_ARG = "angular_deflection,a";
static const char* RELATIVE_ARG = "relative";
static const char* NO_PARALLEL_ARG = "no_parallel";
static const char* HELP_ARG = "help";

double deg_to_rad(double r)
{
	return r * M_PI / 180.0;
}

int main(int argc, char** argv)
{
	brep_mesh_params mesh_params;

	double angular_deflection_deg;

	po::options_description options("Options");
	options.add_options()
		(INPUT_FILE_ARG, po::value<string>(), "Input CAD file")
		(OUTPUT_FILE_ARG, po::value<string>(), "Output STL file")
		(LINEAR_DEFLECTION_ARG, po::value<double>(&(mesh_params.linear_deflection))->default_value(0), 
		 								"Linear deflection, 0 automatically computes value")
		(ANGULAR_DEFLECTION_ARG, po::value<double>(&angular_deflection_deg)->default_value(30), 
		 								 "Angular deflection in degrees")
		(RELATIVE_ARG, "Use relative discretization during meshing")
		(NO_PARALLEL_ARG, "Do not use multiple threads for building mesh")
		(HELP_ARG, "Print help");

	po::positional_options_description pos_options;
	pos_options.add(INPUT_FILE_ARG, 1);
	pos_options.add(OUTPUT_FILE_ARG, 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(options).positional(pos_options).run(), vm);
	po::notify(vm);

	if (!vm.count(INPUT_FILE_ARG) || !vm.count(OUTPUT_FILE_ARG) || vm.count("help"))
	{
		cout << "cadread built with OpenCascade version " << string(OCC_VERSION_COMPLETE) << endl << endl;
		cout << "Usage: " << string(argv[0]) << " <input file> <output file> [options]" << endl;
		cout << options << endl;

		return 0;
	}

	string input_path_str = vm[INPUT_FILE_ARG].as<string>();
	string output_path_str = vm[OUTPUT_FILE_ARG].as<string>();

	fs::path input_path(input_path_str);
	fs::path output_path(output_path_str);

	string in_ext = input_path.extension().string();
	transform(in_ext.begin(), in_ext.end(), in_ext.begin(), ::tolower);

	mesh_params.angle_deflection_rad = ::deg_to_rad(angular_deflection_deg);
	mesh_params.use_relative_discretization = !!vm.count(RELATIVE_ARG);
	mesh_params.use_parallel_meshing = !vm.count(NO_PARALLEL_ARG);

	Handle(Message_ProgressIndicator) indicator(new cadread_ConsoleProgressIndicator(1));

	cad_read_result_t read_result;
	if (in_ext == ".stp" || in_ext == ".step")
		read_result = cadread::ReadSTEP(input_path.string(), indicator);
	else if (in_ext == ".igs" || in_ext == ".iges")
		read_result = cadread::ReadIGES(input_path.string(), indicator);
	else
	{
		cout << "Unknown input file type: " << in_ext << endl;
		return 1;
	}

	if (!read_result.first)
	{
		cout << "Error loading input file" << endl;
		return 1;
	}

	const TopoDS_Shape& brep = read_result.second;
	if (brep.IsNull())
	{
		cout << "Got empty BRep!" << endl;
		return 1;
	}

	unique_ptr<triangle_mesh> mesh = cadread::tessellate_BRep(brep, mesh_params);
	if (!mesh)
	{
		cout << "Error creating mesh from BRep!" << endl;
		return 1;
	}

	// Just write a ASCII STL file for now...
	ofstream out_stream;
	out_stream.open(output_path.string().c_str(), ios::out | ios::trunc);
	if (!out_stream.good())
	{
		cout << "Error opening output stream" << endl;
		return 1;
	}

	out_stream << *mesh;

	cout << "Done!" << endl;

	return 0;
}
