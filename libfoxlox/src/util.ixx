module;
#include <gsl/gsl>
export module foxlox:util;

import <string>;
import <string_view>;
import <sstream>;
import <cassert>;
import <type_traits>;
import <vector>;
import <iterator>;


import :libicu;

namespace foxlox
{
  export std::string u32_to_u8(std::u32string_view in);
  export std::string u32_to_u8(char32_t in);
  export std::u32string u8_to_u32(std::string_view in);

  // from https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
  export template <class T, template <class...> class Template>
    struct is_specialization : std::false_type {};

  export template <template <class...> class Template, class... Args>
    struct is_specialization<Template<Args...>, Template> : std::true_type {};

  export template <typename T, template <class...> class Template>
    inline constexpr bool is_specialization_v = is_specialization<T, Template>::value;

  export template <typename T>
    std::vector<T> make_vector(T&& a)
  {
    std::vector<T> result;
    result.push_back(std::move(a));
    return result;
  }

  export template <typename T>
    std::vector<T> vec_concat(std::vector<T>&& a, std::vector<T>&& b)
  {
    std::vector<T> result = std::move(a);
    result.insert(result.end(),
      std::make_move_iterator(b.begin()),
      std::make_move_iterator(b.end()));
    return result;
  }

  export template <typename T>
    std::vector<T> vec_concat(T&& a, std::vector<T>&& b)
  {
    std::vector<T> result = make_vector(std::move(a));
    result.insert(result.end(),
      std::make_move_iterator(b.begin()),
      std::make_move_iterator(b.end()));
    return result;
  }

  export template <typename T>
    std::vector<T> vec_concat(std::vector<T>&& a, T&& b)
  {
    std::vector<T> result = std::move(a);
    result.push_back(std::move(b));
    return result;
  }

#if defined(NDEBUG) && defined(_MSC_VER)
  export [[noreturn]] void UNREACHABLE()
  {
    __assume(0);
  }
#endif
#if defined(NDEBUG) && !defined(_MSC_VER)
  export [[noreturn]] void UNREACHABLE()
  {
    __builtin_unreachable();
  }
#endif
#if !defined(NDEBUG) && defined(_MSC_VER)
  export [[noreturn]] void UNREACHABLE()
  {
    assert(false);
  }
#endif
#if !defined(NDEBUG) && !defined(_MSC_VER)
  export [[noreturn]] inline void UNREACHABLE()
  {
    assert(false);
  }
#endif

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
      assert(err <= U_ZERO_ERROR);
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
      assert(err <= U_ZERO_ERROR);
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
      assert(err <= U_ZERO_ERROR);
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
      assert(err <= U_ZERO_ERROR);
      result.resize(u32_len);
    }
    return result;
  }
}