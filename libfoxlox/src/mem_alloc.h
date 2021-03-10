#pragma once

#ifdef FOXLOX_USE_MIMALLOC
#include <mimalloc.h>
#define MALLOC mi_malloc
#define FREE mi_free
#else
#include <cstdlib>
#define MALLOC std::malloc
#define FREE std::free
#endif