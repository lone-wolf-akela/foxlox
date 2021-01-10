#ifndef FOXLOX_COMMON_H
#define FOXLOX_COMMON_H

#include <string_view>

#include "token.h"

namespace foxlox
{
  void format_error(Token token, std::string_view message);
}

#endif