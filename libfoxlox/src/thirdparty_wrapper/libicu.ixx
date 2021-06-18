module;
#ifdef FOXLOX_USE_WINSDK_ICU
#include <icu.h>
#pragma comment(lib, "icu.lib") 
#else
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#endif
export module foxlox:libicu;

namespace foxlox
{
  export using ::UErrorCode;
  export using ::U_ZERO_ERROR;
  export using ::UChar32;
  export using ::u_strFromUTF32;
  export using ::u_strToUTF8;
  export using ::u_strFromUTF8Lenient;
  export using ::u_strToUTF32;
  export using ::u_isdigit;
  export using ::u_isWhitespace;
  export using ::u_isalpha;
  export using ::u_isalnum;
}