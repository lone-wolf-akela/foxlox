#pragma once

#define DEBUG_TRACE_EXECUTION
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC
constexpr auto STACK_MAX = 4096;
constexpr auto CALLTRACE_MAX = 1024;
constexpr auto FIRST_GC_HEAP_SIZE = 1024 * 1024;
constexpr auto GC_HEAP_GROW_FACTOR = 2;