#include <iostream>
#include <chrono>
#include <ranges>
#include <algorithm>
#include <numbers>

#include "runtimelib.h"

#include "runtimelibs/algorithm.h"
#include "runtimelibs/io.h"
#include "runtimelibs/math.h"
#include "runtimelibs/profiler.h"

namespace foxlox
{
  const std::unordered_map<std::string, RuntimeLib> g_default_libs =
  {
    {"fox.algorithm", lib::algorithm},
    {"fox.io", lib::io},
    {"fox.math", lib::math},
    {"fox.profiler", lib::profiler},
  };
}
