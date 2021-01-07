#include <string_view>

#include <magic_enum.hpp>
#include <fmt/format.h>

#include "chunk.h"
#include "opcode.h"

namespace foxlox
{
  const std::map<OpCode, OpType> optype
  {
    {OpCode::OP_RETURN, OpType::N},
    {OpCode::OP_NEGATE, OpType::N},
    {OpCode::OP_ADD, OpType::N},
    {OpCode::OP_SUBTRACT, OpType::N},
    {OpCode::OP_MULTIPLY, OpType::N},
    {OpCode::OP_DIVIDE, OpType::N},
    {OpCode::OP_INTDIV, OpType::N},
    {OpCode::OP_NIL, OpType::N},

    {OpCode::OP_CONSTANT, OpType::uA},
    {OpCode::OP_STRING, OpType::uA},
    {OpCode::OP_BOOL, OpType::uA},

    {OpCode::OP_INT, OpType::iA},
  };

  Inst::Inst(OpCode o)
  {
    assert(optype.at(o) == OpType::N);
    N.op = o;
  }
  Inst::Inst(OpCode o, uint32_t a)
  {
    assert(optype.at(o) == OpType::uA);
    assert(a <= INST_UA_MAX);
    uA.op = o;
    uA.ua = a;
  }
  Inst::Inst(OpCode o, int32_t ia)
  {
    assert(optype.at(o) == OpType::iA);
    assert(INST_IA_MIN <= ia && ia <= INST_IA_MAX);
    iA.op = o;
    iA.ia = ia;
  }
  std::string Inst::to_string(const Chunk& chunk) const
  {
    // N
    if (optype.at(N.op) == OpType::N)
    {
      return std::string(magic_enum::enum_name(N.op));
    }
    switch (N.op)
    {
    // uA
    case OpCode::OP_CONSTANT:
    {
      const auto constant = uA.ua;
      return fmt::format("{:<16} {:>4} `{}'",
        magic_enum::enum_name(uA.op), constant, chunk.get_constants().at(constant).to_string());
    }
    case OpCode::OP_STRING:
    {
      const auto str_idx = uA.ua;
      const String* str = chunk.get_strings().at(str_idx);
      return fmt::format("{:<16} {:>4} `{}'",
        magic_enum::enum_name(uA.op), str_idx, std::string_view(str->str, str->length));
    }
    case OpCode::OP_BOOL:
    {
      return fmt::format("{:<16} {:>4} `{}'",
        magic_enum::enum_name(uA.op), "", bool(uA.ua) ? "true" : "false");
    }
    // iA
    case OpCode::OP_INT:
    {
      return fmt::format("{:<16} {:>4} `{}'",
        magic_enum::enum_name(iA.op), "", iA.ia);
    }
    default:
      assert(false);
      return "";
    }
  }
}