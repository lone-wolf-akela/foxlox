#pragma once

import <cstdint>;

// from https://bullno1.com/blog/switched-goto

#define OPCODE(X) \
	X(NOP) \
	X(NIL) \
	X(RETURN) \
	X(RETURN_V) \
  X(POP) \
  X(POP_N) \
  X(CALL) \
  X(NEGATE) \
  X(NOT) \
  X(ADD) \
  X(SUBTRACT) \
  X(MULTIPLY) \
  X(DIVIDE) \
  X(INTDIV) \
  X(EQ) \
  X(NE) \
  X(GT) \
  X(GE) \
  X(LT) \
  X(LE) \
  X(CONSTANT) \
  X(STRING) \
  X(BOOL) \
  X(TUPLE) \
  X(FUNC) \
  X(CLASS) \
  X(LOAD_STACK) \
  X(STORE_STACK) \
  X(LOAD_STATIC) \
  X(STORE_STATIC) \
  X(JUMP) \
  X(JUMP_IF_TRUE) \
  X(JUMP_IF_FALSE) \
  X(JUMP_IF_TRUE_NO_POP) \
  X(JUMP_IF_FALSE_NO_POP) \
  X(SET_PROPERTY) \
  X(GET_PROPERTY) \
  X(INHERIT) \
  X(GET_SUPER_METHOD) \
  X(IMPORT) \
  X(UNPACK)

namespace foxlox
{
#define DEFINE_ENUM(NAME, ENUMX) enum class NAME : uint8_t { ENUMX(ENUM_ENTRY) }
#define ENUM_ENTRY(ENTRY) ENTRY,

  DEFINE_ENUM(OP, OPCODE);

#undef DEFINE_ENUM
#undef ENUM_ENTRY
}
