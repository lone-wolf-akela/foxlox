export module foxlox:runtimelibs.math;

import <numbers>;

import :runtimelib;
import :value;

namespace foxlox::lib
{
  export RuntimeLib math()
  {
    return RuntimeLib{
      { "pi", std::numbers::pi },
    };
  };
}