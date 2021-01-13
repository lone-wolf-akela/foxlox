#pragma once

#include <string_view>

#include "token.h"

namespace foxlox
{
  void format_error(Token token, std::string_view message);
}
