#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

#include "value.h"

namespace foxlox
{
  struct CompiletimeValue
  {
    std::variant<nullptr_t, double, int64_t, std::string, bool, CppFunc*> v;
    CompiletimeValue();
    CompiletimeValue(double f64);
    CompiletimeValue(int64_t i64);
    CompiletimeValue(std::string_view str);
    CompiletimeValue(bool b);
    CompiletimeValue(CppFunc* cppfunc);

    std::string to_string() const;
  };
}
