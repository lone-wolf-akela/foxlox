#pragma once

#include <string>
#include <string_view>

#include "compiletime_value.h"

namespace foxlox
{
  enum class TokenType
  {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, SEMICOLON,
	  
    // One or two character tokens.
    PLUS, PLUS_PLUS, PLUS_EQUAL,
    MINUS, MINUS_MINUS, MINUS_EQUAL,
    STAR, STAR_EQUAL,
    SLASH, SLASH_SLASH, SLASH_EQUAL, SLASH_SLASH_EQUAL,
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
    FROM, IMPORT, AS,

    TKERROR, TKEOF
  };

  struct Token
  {
    TokenType type;
    std::string lexeme;
    CompiletimeValue literal;
    int line;

    Token(TokenType type, std::string_view lexeme, CompiletimeValue literal, int line);

    std::string to_string() const;
  };
}
