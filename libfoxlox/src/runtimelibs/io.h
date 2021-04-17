#pragma once

#include "../runtimelib.h"

namespace foxlox::lib
{
  FOXLOX_LIB_FUN(print);
  FOXLOX_LIB_FUN(println);

  const RuntimeLib io = {
    { "print", print },
    { "println", println},
  };
}