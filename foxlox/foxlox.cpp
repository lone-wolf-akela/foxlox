#include "debug.h"
#include "chunk.h"
#include "vm.h"

int main(int /*argc*/, const char* /*argv[]*/)
{
  using namespace foxlox;
  VM vm;

  Chunk chunk;
  auto constant = chunk.add_constant(1.2);
  chunk.add_code(Inst(OpCode::OP_CONSTANT, constant), 100);
  constant = chunk.add_constant(3.4);
  chunk.add_code(Inst(OpCode::OP_CONSTANT, constant), 100);
  chunk.add_code(Inst(OpCode::OP_ADD), 100);

  constant = chunk.add_constant(int64_t(2));
  chunk.add_code(Inst(OpCode::OP_CONSTANT, constant), 100);
  chunk.add_code(Inst(OpCode::OP_MULTIPLY), 100);

  chunk.add_code(Inst(OpCode::OP_NEGATE), 100);

  chunk.add_code(Inst(OpCode::OP_RETURN), 100);

  vm.interpret(chunk);

  return 0;
}
