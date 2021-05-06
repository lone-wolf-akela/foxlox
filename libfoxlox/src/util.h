#pragma once

import <string>;
import <string_view>;
import <concepts>;
import <sstream>;

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

  // from https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
  template <class T, template <class...> class Template>
  struct is_specialization : std::false_type {};

  template <template <class...> class Template, class... Args>
  struct is_specialization<Template<Args...>, Template> : std::true_type {};

  template <typename T, template <class...> class Template>
  inline constexpr bool is_specialization_v = is_specialization<T, Template>::value;
}
