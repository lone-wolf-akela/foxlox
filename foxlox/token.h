#ifndef FOXLOX_TOKEN_H
#define FOXLOX_TOKEN_H

#include <string>
#include <string_view>
#include <fmt/format.h>

#include "value.h"

namespace foxlox
{
  enum class TokenType
  {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    COLON,

    // Literals.
    IDENTIFIER, STRING, INT, DOUBLE,

    // Keywords.
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    BREAK, CONTINUE,

    EOF
  };

  struct Token
  {
    const TokenType type;
    const std::string lexeme;
    const Value literal;
    const int line;

    Token(TokenType type, std::string_view lexeme, Value literal, int line) :
      type(type),
      lexeme(lexeme),
      literal(literal),
      line(line)
    {
    }

    std::string to_string()
    {
      return fmt::format("{} {} {}", type, lexeme, literal);
    }
  };
}

#endif // FOXLOX_TOKEN_H