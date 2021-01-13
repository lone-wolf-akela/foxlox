#pragma once

#include <string>
#include <string_view>
#include <concepts>
#include <sstream>

namespace foxlox
{
  std::string u32_to_u8(std::u32string_view in);
  std::string u32_to_u8(char32_t in);
  std::u32string u8_to_u32(std::string_view in);

  template<typename T> requires std::integral<T> || std::floating_point<T>
  std::string num_to_str(T v)
  {
    std::stringstream strm;
    strm << v;
    return strm.str();
  }
}
