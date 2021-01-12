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
    OP_RETURN_V,

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

    OP_LOAD_STACK,
    OP_STORE_STACK,
    OP_LOAD_STATIC,
    OP_STORE_STATIC,
  };
}

#endif // FOXLOX_OPCODE_H