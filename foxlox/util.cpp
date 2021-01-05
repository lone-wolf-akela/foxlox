#include <cassert>
#include <vector>

#ifdef _WIN32
#include <icu.h>
#pragma comment(lib, "icu.lib") 
#undef FALSE
#undef TRUE
#endif

#include <gsl/gsl>

#include "util.h"

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
    return u32_to_u8(std::u32string(1, in));
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