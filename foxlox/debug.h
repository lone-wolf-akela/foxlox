#ifndef FOXLOC_DEBUG_H
#define FOXLOC_DEBUG_H

#include <string_view>

#include "chunk.h"

namespace foxlox 
{
  void disassemble_chunk(const Chunk& chunk, std::string_view name);
}
#endif // FOXLOC_DEBUG_H