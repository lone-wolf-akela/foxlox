#ifndef FOXLOX_DEBUG_H
#define FOXLOX_DEBUG_H

#include <string_view>

#include <chunk.h>

namespace foxlox 
{
  void disassemble_inst(const Chunk& chunk, const Closure& closure, Inst inst, gsl::index index);
  void disassemble_chunk(const Chunk& chunk, const Closure& closure, std::string_view name);
}
#endif // FOXLOX_DEBUG_H