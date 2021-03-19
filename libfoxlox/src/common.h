#pragma once
#include <cassert>
#include <string_view>
#include <array>

#include "token.h"

#if defined(NDEBUG) && defined(_MSC_VER)
#define UNREACHABLE __assume(0)
#endif
#if defined(NDEBUG) && !defined(_MSC_VER)
#define UNREACHABLE __builtin_unreachable()
#endif
#if !defined(NDEBUG) && defined(_MSC_VER)
#define UNREACHABLE assert(false); __assume(0)
#endif
#if !defined(NDEBUG) && !defined(_MSC_VER)
#define UNREACHABLE assert(false); __builtin_unreachable()
#endif

namespace foxlox
{
  [[maybe_unused]] constexpr std::array BINARY_HEADER = { '\004', '\002', 'F', 'O', 'X', 'L', 'O', 'X' };
  void format_error(Token token, std::string_view message);
}
