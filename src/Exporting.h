#pragma once

#include <filesystem>

#include <Standard_Handle.hxx>

class TopoDS_Shape;
class TDocStd_Document;

namespace cadread
{
    bool ExportSTEP(TopoDS_Shape const& S, std::filesystem::path const& out_path);
    bool ExportSTEPXDE(Handle(TDocStd_Document) doc, std::filesystem::path const& out_path);
}