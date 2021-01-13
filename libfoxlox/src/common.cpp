#include <fmt/format.h>

#include "common.h"

namespace foxlox
{
  void format_error(Token token, std::string_view message)
  {
    fmt::print(stderr, "[line {}] Error{}: {}\n",
      token.line,
      token.type == TokenType::TKEOF ? " at end" :
      token.type == TokenType::TKERROR ? "" :
      fmt::format(" at `{}'", token.lexeme),
      message
    );
  }
}