#include <chrono>

#include <foxlox/vm.h>

#include "profiler.h"

namespace foxlox::lib
{
  FOXLOX_LIB_FUN(clock)
  {
    std::ignore = vm;

    if (values.size() != 0)
    {
      throw RuntimeLibError("[clock]: This function does not need any paramters.");
    }
    using namespace std::chrono;
    const auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return ms.count() / 1000.0;
  }
}