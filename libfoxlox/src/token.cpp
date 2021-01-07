#include <fmt/format.h>
#include <magic_enum.hpp>

#include "token.h"

namespace foxlox
{
  Token::Token(TokenType type, std::string_view lexeme, CompiletimeValue literal, int line) :
    type(type),
    lexeme(lexeme),
    literal(literal),
    line(line)
  {
  }
  std::string Token::to_string() const
  {
    return fmt::format("{} {} {}", magic_enum::enum_name(type), lexeme, literal.to_string());
  }
}