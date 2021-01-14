#pragma once

#include <string_view>

#include <chunk.h>

namespace foxlox 
{
  gsl::index disassemble_inst(const Chunk& chunk, const Subroutine& closure, gsl::index index);
  void disassemble_chunk(const Chunk& chunk, const Subroutine& closure, std::string_view name);
}
