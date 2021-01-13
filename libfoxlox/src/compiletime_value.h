#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

namespace foxlox
{
  struct CompiletimeValue
  {
    std::variant<std::monostate, double, int64_t, std::string, bool> v;
    CompiletimeValue();
    CompiletimeValue(double f64);
    CompiletimeValue(int64_t i64);
    CompiletimeValue(std::string_view str);
    CompiletimeValue(bool b);

    std::string to_string() const;
  };
}
