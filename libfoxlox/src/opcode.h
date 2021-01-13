#pragma once

#include <cstdint>

namespace foxlox
{
  enum OpCode : uint8_t
  {
    OP_NOP,
    OP_NIL,
    OP_RETURN,
    OP_RETURN_V,
    OP_POP,
    OP_POP_N,

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

    OP_JUMP,
    OP_JUMP_IF_TRUE,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE_NO_POP,
    OP_JUMP_IF_FALSE_NO_POP,
  };
}
