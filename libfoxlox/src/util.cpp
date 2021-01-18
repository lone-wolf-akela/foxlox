#include <iostream>
#include <vector>

#include <unicode/unistr.h>
#include <gsl/gsl>

#include "util.h"

namespace foxlox
{
  std::string u32_to_u8(std::u32string_view in)
  {
    auto u16string = icu::UnicodeString::fromUTF32(
      reinterpret_cast<const UChar32*>(in.data()), 
      gsl::narrow_cast<int32_t>(in.size())
    );
    std::string u8string;
    u16string.toUTF8String(u8string);
    return u8string;
  }
  std::string u32_to_u8(char32_t in)
  {
    return u32_to_u8(std::u32string(1, in));
  }
  std::u32string u8_to_u32(std::string_view in)
  {
    auto u16string = icu::UnicodeString::fromUTF8(in);
    std::u32string u32string(u16string.length(), U'\0');
    UErrorCode err = U_ZERO_ERROR;
    auto len = u16string.toUTF32(
      reinterpret_cast<UChar32*>(u32string.data()),
      gsl::narrow_cast<int32_t>(u32string.size()),
      err
    );
    if (U_FAILURE(err))
    {
      std::cerr << "String encoding conversion failed.\n";
    }
    u32string.resize(len);
    return u32string;
  }
}