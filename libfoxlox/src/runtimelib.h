#pragma once
#include <unordered_map>
#include <vector>
#include <span>
#include <tuple>

#include <foxlox/except.h>
#include "value.h"
#include "object.h"

namespace foxlox
{
  struct RuntimeLibElem
  {
    std::string name;
    Value val;
  };
  using RuntimeLib = std::vector<RuntimeLibElem>;

  extern const std::unordered_map<std::string, RuntimeLib> g_default_libs;
}
namespace foxlox
{
  class VM;
}
#define FOXLOX_LIB_FUN(name) foxlox::Value name(foxlox::VM& vm, std::span<foxlox::Value> values)
