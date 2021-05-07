module;
#include <fmt/format.h>

#ifdef FOXLOX_USE_WINSDK_ICU
#include <icu.h>
#pragma comment(lib, "icu.lib") 
#undef FALSE
#undef TRUE
#else
#include <unicode/ustring.h>
#endif
export module foxlox:util;

import <string>;
import <string_view>;
import <concepts>;
import <sstream>;
import <cassert>;
import <array>;

import <gsl/gsl>;

namespace foxlox
{
  export std::string u32_to_u8(std::u32string_view in);
  export std::string u32_to_u8(char32_t in);
  export std::u32string u8_to_u32(std::string_view in);

  export template<typename T> requires std::integral<T> || std::floating_point<T>
  std::string num_to_str(T v)
  {
    std::stringstream strm;
    strm << v;
    return strm.str();
  }

  // from https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
  export template <class T, template <class...> class Template>
  struct is_specialization : std::false_type {};

  export template <template <class...> class Template, class... Args>
  struct is_specialization<Template<Args...>, Template> : std::true_type {};

  export template <typename T, template <class...> class Template>
  inline constexpr bool is_specialization_v = is_specialization<T, Template>::value;

  export constexpr std::array<char, 8> BINARY_HEADER = { '\004', '\002', 'F', 'O', 'X', 'L', 'O', 'X' };
}

namespace foxlox
{
  std::string u32_to_u8(std::u32string_view in)
  {
    std::u16string buffer(in.size() * 2, u'0');
    {
      UErrorCode err = U_ZERO_ERROR;
      int32_t u16_len;
      u_strFromUTF32(
        buffer.data(),
        gsl::narrow_cast<int32_t>(ssize(buffer)),
        &u16_len,
        reinterpret_cast<const UChar32*>(in.data()),
        gsl::narrow_cast<int32_t>(ssize(in)),
        &err
      );
      buffer.resize(u16_len);
      assert(!U_FAILURE(err));
    }
    std::string result(in.size() * 4, '0');
    {
      UErrorCode err = U_ZERO_ERROR;
      int32_t u8_len;
      u_strToUTF8(
        result.data(),
        gsl::narrow_cast<int32_t>(ssize(result)),
        &u8_len,
        buffer.data(),
        gsl::narrow_cast<int32_t>(ssize(buffer)),
        &err
      );
      assert(!U_FAILURE(err));
      result.resize(u8_len);
    }
    return result;
  }
  std::string u32_to_u8(char32_t in)
  {
    return u32_to_u8(std::u32string_view(&in, 1));
  }
  std::u32string u8_to_u32(std::string_view in)
  {
    std::u16string buffer(in.size() * 2, u'0');
    {
      UErrorCode err = U_ZERO_ERROR;
      int32_t u16_len;
      u_strFromUTF8Lenient(
        buffer.data(),
        gsl::narrow_cast<int32_t>(ssize(buffer)),
        &u16_len,
        in.data(),
        gsl::narrow_cast<int32_t>(ssize(in)),
        &err
      );
      buffer.resize(u16_len);
      assert(!U_FAILURE(err));
    }
    std::u32string result(in.size(), U'0');
    {
      UErrorCode err = U_ZERO_ERROR;
      int32_t u32_len;
      u_strToUTF32(
        reinterpret_cast<int32_t*>(result.data()),
        gsl::narrow_cast<int32_t>(ssize(result)),
        &u32_len,
        buffer.data(),
        gsl::narrow_cast<int32_t>(ssize(buffer)),
        &err
      );
      assert(!U_FAILURE(err));
      result.resize(u32_len);
    }
    return result;
  }
}