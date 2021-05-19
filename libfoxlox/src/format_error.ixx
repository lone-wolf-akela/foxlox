export module foxlox:format_error;

import <iostream>;
import <format>;

import :token;

namespace foxlox
{
  export void format_error(Token token, std::string_view message)
  {
    std::cerr << std::format("[line {}] Error{}: {}\n",
      token.line,
      token.type == TokenType::TKEOF ? " at end" :
      token.type == TokenType::TKERROR ? "" :
      std::format(" at `{}'", token.lexeme),
      token.type == TokenType::TKERROR ? token.lexeme : message
    );
  }
}