#include <algorithm>

#include <foxlox/vm.h>

#include "algorithm.h"

namespace foxlox::lib
{
  FOXLOX_LIB_FUN(max)
  {
    std::ignore = vm;
    return *std::ranges::max_element(values);
  }
  FOXLOX_LIB_FUN(min)
  {
    std::ignore = vm;
    return *std::ranges::min_element(values);
  }
}