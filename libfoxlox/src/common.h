#pragma once
#include <cassert>
#include <string_view>

#include "token.h"

#ifndef NDEBUG
#define UNREACHABLE assert(false)
#else
#ifdef _MSC_VER
#define UNREACHABLE __assume(0)
#else
#define UNREACHABLE __builtin_unreachable()
#endif
#endif
namespace foxlox
{
  void format_error(Token token, std::string_view message);
}
