#pragma once

#include <cstdint>

namespace foxlox
{
  enum class OP : uint8_t
  {
    NOP, NIL, RETURN, RETURN_V, POP, POP_N,
    CALL,
    NEGATE, NOT,
    ADD, SUBTRACT, MULTIPLY, DIVIDE, INTDIV,
    EQ, NE, GT, GE, LT, LE,
    CONSTANT, STRING, BOOL, TUPLE, FUNC, CLASS,
    LOAD_STACK, STORE_STACK, LOAD_STATIC, STORE_STATIC,
    JUMP, JUMP_IF_TRUE, JUMP_IF_FALSE, JUMP_IF_TRUE_NO_POP, JUMP_IF_FALSE_NO_POP,
    SET_PROPERTY, GET_PROPERTY, INHERIT, GET_SUPER_METHOD,
  };
}
