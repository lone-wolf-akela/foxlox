module;
module foxlox:runtimelib;

import <iostream>;
import <chrono>;
import <ranges>;
import <algorithm>;
import <numbers>;

import :runtimelibs.algorithm;
import :runtimelibs.io;
import :runtimelibs.math;
import :runtimelibs.profiler;

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
