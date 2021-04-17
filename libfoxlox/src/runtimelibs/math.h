#pragma once
#include <numbers>

#include "../runtimelib.h"

namespace foxlox::lib
{
  const RuntimeLib math = {
    { "pi", std::numbers::pi },
  };
}