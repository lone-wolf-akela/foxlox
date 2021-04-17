#pragma once

#include "../runtimelib.h"

namespace foxlox::lib
{
  FOXLOX_LIB_FUN(max);
  FOXLOX_LIB_FUN(min);

  const RuntimeLib algorithm = {
    { "max", max },
    { "min", min},
  };
}