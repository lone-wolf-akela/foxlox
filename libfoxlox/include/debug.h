#pragma once

#include <string_view>

#include <chunk.h>

namespace foxlox 
{
  gsl::index disassemble_inst(const Chunk& chunk, const Subroutine& subroutine, gsl::index index);
  void disassemble_chunk(const Chunk& chunk, const Subroutine& subroutine, std::string_view name);
}
