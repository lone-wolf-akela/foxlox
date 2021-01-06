#ifndef FOXLOX_TOKEN_H
#define FOXLOX_TOKEN_H

#include <string>
#include <string_view>

#include "compiletime_value.h"

namespace foxlox
{
  enum class TokenType
  {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
	  SLASH_SLASH,

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

    TKERROR, TKEOF
  };

  struct Token
  {
    const TokenType type;
    const std::string lexeme;
    const CompiletimeValue literal;
    const int line;

    Token(TokenType type, std::string_view lexeme, CompiletimeValue literal, int line);

    std::string to_string() const;
  };
}

#endif // FOXLOX_TOKEN_H