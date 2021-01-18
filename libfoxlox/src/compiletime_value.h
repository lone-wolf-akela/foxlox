#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

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

  class CompiletimeClass
  {
  public:
    CompiletimeClass(std::string_view name);
    void add_method(uint16_t name_idx, uint16_t subroutine_idx);
    std::string_view get_name() const noexcept;
    std::span<const std::pair<uint16_t, uint16_t>> get_methods() const noexcept;
  private:
    std::string classname;
    std::vector<std::pair<uint16_t, uint16_t>> methods; // store the name str idx & subroutine idx
  };
}
