module;
export module foxlox:runtimelibs.algorithm;

import <algorithm>;
import <span>;

import :runtimelib;
import :vm;
import :value;

namespace foxlox::lib
{
  export foxlox::Value max(foxlox::VM& /*vm*/, std::span<foxlox::Value> values)
  {
      return *std::ranges::max_element(values);
  }
  export foxlox::Value min(foxlox::VM& /*vm*/ , std::span<foxlox::Value> values)
  {
      return *std::ranges::min_element(values);
  }

  export const RuntimeLib algorithm = {
    { "max", max },
    { "min", min},
  };
}