module;
#include "../value.h"
export module foxlox.runtimelibs.algorithm;

import <algorithm>;
import <span>;

import foxlox.runtimelib;
import foxlox.vm;

namespace foxlox::lib
{
  export foxlox::Value max(foxlox::VM& vm, std::span<foxlox::Value> values)
  {
      return *std::ranges::max_element(values);
  }
  export foxlox::Value min(foxlox::VM& vm, std::span<foxlox::Value> values)
  {
      return *std::ranges::min_element(values);
  }

  export const RuntimeLib algorithm = {
    { "max", max },
    { "min", min},
  };
}