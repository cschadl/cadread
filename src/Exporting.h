#pragma once

#include <filesystem>

class TopoDS_Shape;

namespace cadread
{
    bool ExportSTEP(TopoDS_Shape const& S, std::filesystem::path const& out_path);
}