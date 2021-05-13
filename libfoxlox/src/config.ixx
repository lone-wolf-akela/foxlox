export module foxlox:config;

import <array>;

/*
#define FOXLOX_DEBUG_TRACE_STACK
#define FOXLOX_DEBUG_TRACE_INST
#define FOXLOX_DEBUG_TRACE_SRC
#define FOXLOX_DEBUG_LOG_GC
#define FOXLOX_DEBUG_STRESS_GC
*/

export constexpr auto STACK_MAX = 1024;
export constexpr auto CALLTRACE_MAX = 256;
export constexpr auto FIRST_GC_HEAP_SIZE = 1024 * 1024;
export constexpr auto GC_HEAP_GROW_FACTOR = 2;
export constexpr auto STRING_POOL_MAX_LOAD = 0.75;
export constexpr auto HASH_TABLE_START_BUCKET = 1 << 3;

export constexpr std::array<char, 8> BINARY_HEADER = { '\004', '\002', 'F', 'O', 'X', 'L', 'O', 'X' };
