module;
#include "value.h"
export module foxlox.runtimelib;

import <unordered_map>;
import <vector>;
import <string>;

import foxlox.except;

namespace foxlox
{
  export struct RuntimeLibElem
  {
    std::string name;
    Value val;
  };
  export using RuntimeLib = std::vector<RuntimeLibElem>;

  export extern const std::unordered_map<std::string, RuntimeLib> g_default_libs;
}
