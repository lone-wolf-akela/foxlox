module;
#include <fmt/format.h>
#include <magic_enum.hpp>
export module foxlox.token;

import <string>;
import <string_view>;

import foxlox.compiletime_value;

namespace foxlox
{
    export enum class TokenType
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
        FROM, IMPORT, AS, EXPORT,

        TKERROR, TKEOF
    };

    export struct Token
    {
        TokenType type;
        std::string lexeme;
        CompiletimeValue literal;
        int line;

        Token(TokenType type, std::string_view lexeme, CompiletimeValue literal, int line) :
            type(type),
            lexeme(lexeme),
            literal(literal),
            line(line)
        {
        }

        std::string to_string() const
        {
            return fmt::format("{} {} {}", magic_enum::enum_name(type), lexeme, literal.to_string());
        }
    };
}
