#pragma once

//#define FOXLOX_DEBUG_TRACE_STACK
//#define FOXLOX_DEBUG_TRACE_INST
//#define FOXLOX_DEBUG_TRACE_SRC
//#define FOXLOX_DEBUG_STRESS_GC
//#define FOXLOX_DEBUG_LOG_GC

#define FOXLOX_USE_MIMALLOC
#define FOXLOX_USE_SWITCHED_GOTO
//#define USE_COMPUTED_GOTO
/* if neither FOXLOX_USE_SWITCHED_GOTO nor USE_COMPUTED_GOTO is set
 * we will use the plain old while(true) and switch case in the vm
 * Note: FOXLOX_USE_SWITCHED_GOTO is faster than USE_COMPUTED_GOTO during my testing
 * Note2: USE_COMPUTED_GOTO does not support MSVC compiler
 */

constexpr auto STACK_MAX = 1024;
constexpr auto CALLTRACE_MAX = 256;
constexpr auto FIRST_GC_HEAP_SIZE = 1024 * 1024;
constexpr auto GC_HEAP_GROW_FACTOR = 2;
constexpr auto STRING_POOL_MAX_LOAD = 0.75;
constexpr auto HASH_TABLE_START_BUCKET = 1 << 3;
