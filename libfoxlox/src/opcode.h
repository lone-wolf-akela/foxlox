#ifndef FOXLOX_OPCODE_H
#define FOXLOX_OPCODE_H

#include <cstdint>

namespace foxlox
{
  enum OpCode : uint8_t
  {
    OP_NOP,
    OP_NIL,
    OP_RETURN,

    OP_NEGATE,
    OP_NOT,

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

    OP_CONSTANT,
    OP_STRING,
    OP_BOOL,

    OP_LOAD,
    OP_STORE,
  };
}

#endif // FOXLOX_OPCODE_H