export module foxlox:runtimelib;

import <unordered_map>;
import <vector>;
import <string>;

import :except;
import :value;

namespace foxlox
{
  export struct RuntimeLibElem
  {
    std::string name;
    Value val;
  };
  export using RuntimeLib = std::vector<RuntimeLibElem>;

  export std::unordered_map<std::string, RuntimeLib> default_libs();
}
