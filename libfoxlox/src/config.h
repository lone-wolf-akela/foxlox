#pragma once

//#define DEBUG_TRACE_STACK
//#define DEBUG_TRACE_INST
//#define DEBUG_TRACE_SRC
//#define DEBUG_STRESS_GC
//#define DEBUG_LOG_GC

constexpr auto STACK_MAX = 1024;
constexpr auto CALLTRACE_MAX = 256;
constexpr auto FIRST_GC_HEAP_SIZE = 1024 * 1024;
constexpr auto GC_HEAP_GROW_FACTOR = 2;