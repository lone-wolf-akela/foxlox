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
    COMMA, DOT, MINUS, SEMICOLON,  STAR,
	  
    // One or two character tokens.
    PLUS, PLUS_PLUS, 
    SLASH, SLASH_SLASH,
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
    TokenType type;
    std::string lexeme;
    CompiletimeValue literal;
    int line;

    Token(TokenType type, std::string_view lexeme, CompiletimeValue literal, int line);

    std::string to_string() const;
  };
}
