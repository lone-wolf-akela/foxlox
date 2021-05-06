module;
#include "../value.h"
export module foxlox.runtimelibs.math;

import <numbers>;

import foxlox.runtimelib;

namespace foxlox::lib
{
  export const RuntimeLib math = {
    { "pi", std::numbers::pi },
  };
}