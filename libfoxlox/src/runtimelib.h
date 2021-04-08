#pragma once
#include "parser.h"

namespace foxlox
{
  struct ExportedLibElem
  {
    std::string name;
    Value val;
  };
  std::vector<ExportedLibElem> find_lib(std::span<const std::string_view> libpath);
}