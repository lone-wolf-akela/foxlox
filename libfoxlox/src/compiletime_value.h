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
    std::variant<std::nullptr_t, double, int64_t, std::string, bool, CppFunc*> v;
    CompiletimeValue() noexcept;
    CompiletimeValue(double f64) noexcept;
    CompiletimeValue(int64_t i64) noexcept;
    CompiletimeValue(std::string_view str);
    CompiletimeValue(bool b) noexcept;
    CompiletimeValue(CppFunc* cppfunc) noexcept;

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
