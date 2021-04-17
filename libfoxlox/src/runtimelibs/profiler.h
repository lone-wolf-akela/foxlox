#pragma once

#include "../runtimelib.h"

namespace foxlox::lib
{
  FOXLOX_LIB_FUN(clock);

  const RuntimeLib profiler = {
    { "clock", clock },
  };
}