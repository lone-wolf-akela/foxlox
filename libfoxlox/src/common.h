#pragma once

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

