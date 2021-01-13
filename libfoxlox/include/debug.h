#pragma once

#include <string_view>

#include <chunk.h>

namespace foxlox 
{
  gsl::index disassemble_inst(const Chunk& chunk, const Closure& closure, gsl::index index);
  void disassemble_chunk(const Chunk& chunk, const Closure& closure, std::string_view name);
}
