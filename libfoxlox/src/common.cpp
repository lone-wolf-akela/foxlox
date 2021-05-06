#include "common.h"

#include <fmt/format.h>

namespace foxlox
{
  void format_error(Token token, std::string_view message)
  {
    fmt::print(stderr, "[line {}] Error{}: {}\n",
      token.line,
      token.type == TokenType::TKEOF ? " at end" :
      token.type == TokenType::TKERROR ? "" :
      fmt::format(" at `{}'", token.lexeme),
      token.type == TokenType::TKERROR ? token.lexeme : message
    );
  }
}