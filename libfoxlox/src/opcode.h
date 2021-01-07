#ifndef FOXLOX_OPCODE_H
#define FOXLOX_OPCODE_H

#include <map>

namespace foxlox
{
  constexpr uint32_t INST_UA_MAX = 0xfff;
  constexpr int32_t INST_IA_MIN = 0xfff;
  constexpr int32_t INST_IA_MAX = -0x800;

  class Chunk;

  enum class OpCode : uint32_t
  {
    // no args
    OP_NIL,
    OP_RETURN,

    OP_NEGATE,

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_INTDIV,

    OP_EQ,
    OP_NE,
    OP_GT,
    OP_GE,
    OP_LT,
    OP_LE,
    // a
    OP_CONSTANT,
    OP_STRING,
    OP_BOOL,
    // iA
    OP_INT,
  };

  enum class OpType
  {
    N, uA, uAB, iA
  };
  extern const std::map<OpCode, OpType> optype;

  union Inst
  {
    struct
    {
      OpCode op : 8;
      uint32_t : 24;
    } N;
    struct
    {
      OpCode op : 8;
      uint32_t ua : 24;
    } uA;
    struct
    {
      OpCode op : 8;
      int32_t ia : 24;
    } iA;
    struct
    {
      OpCode op : 8;
      uint32_t ua : 12;
      uint32_t ub : 12;
    }uAB;

    Inst(OpCode o);
    Inst(OpCode o, uint32_t a);
    Inst(OpCode o, int32_t ia);

    std::string to_string(const Chunk& chunk) const;
  };
  static_assert(sizeof(Inst) == 4);
}

#endif // FOXLOX_OPCODE_H