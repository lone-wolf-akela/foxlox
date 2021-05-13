export module foxlox:runtimelibs.profiler;

import <chrono>;
import <span>;

import :runtimelib;
import :vm;
import :value;

namespace foxlox::lib
{
  export foxlox::Value clock(foxlox::VM& /*vm*/, std::span<foxlox::Value> values)
  {
      if (values.size() != 0)
      {
          throw RuntimeLibError("[clock]: This function does not need any paramters.");
      }
      using namespace std::chrono;
      const auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
      return ms.count() / 1000.0;
  }

  export RuntimeLib profiler()
  {
      return RuntimeLib
      {
        { "clock", clock },
      };
  };
}