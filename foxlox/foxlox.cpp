#include "debug.h"
#include "chunk.h"

int main(int /*argc*/, const char* /*argv[]*/)
{
  using namespace foxlox;
  Chunk chunk;
  chunk.add_code({ OpCode::OP_RETURN }, 0);
  const auto constant = chunk.add_constant(1.2);
  chunk.add_code({ OpCode::OP_CONSTANT, constant }, 1);
  const auto constant2 = chunk.add_constant(2.2);
  chunk.add_code({ OpCode::OP_CONSTANT, constant2 }, 1);

  disassemble_chunk(chunk, "test chunk");

  return 0;
}
