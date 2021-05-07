module;
export module foxlox:runtimelibs.math;

import <numbers>;

import :runtimelib;
import :value;

namespace foxlox::lib
{
  export const RuntimeLib math = {
    { "pi", std::numbers::pi },
  };
}