#include <gtest/gtest.h>
#include <magic_enum.hpp>
#include <fmt/format.h>

#include <foxexcept.h>
#include <chunk.h>
#include "../../libfoxlox/src/opcode.h"

using namespace foxlox;

TEST(Basic, OpCode_Type) 
{
  for (auto opcode : magic_enum::enum_values<OpCode>())
  {
    ASSERT_TRUE(optype.contains(opcode))
      << fmt::format("foxlox::optype does not contain `{}'", magic_enum::enum_name(opcode));
  }
}

TEST(Basic, OpCode_ToString)
{
  Chunk chunk;
  chunk.add_constant(Value());
  chunk.add_string("");
  for (auto opcode : magic_enum::enum_values<OpCode>())
  {
    Inst inst(OpCode::OP_NOP);
    switch (optype.at(opcode))
    {
    case OpType::N:
      inst = foxlox::Inst(opcode);
      break;
    case OpType::uA:
      inst = Inst(opcode, uint32_t{});
      break;
    case OpType::uAB:
      inst = Inst(opcode, uint32_t{}, uint32_t{});
      break;
    case OpType::iA:
      inst = Inst(opcode, int32_t{});
      break;
    }
    ASSERT_NO_THROW(inst.to_string(chunk))
      << fmt::format("error casting inst `{}' to string", magic_enum::enum_name(opcode));
  }
}