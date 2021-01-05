#ifndef FOXLOX_SCANNER_H
#define FOXLOX_SCANNER_H

#include <string_view>
#include <vector>
#include <map>

#include <gsl/gsl>

#include "token.h"

namespace foxlox
{
  class Scanner
  {
  private:
    const std::string_view source;
    const std::vector<Token> tokens;

    gsl::index start = 0;
    gsl::index current = 0;
    int line = 1;

    const std::map<std::string_view, TokenType> keywords
    {
      { "and", AND },
      { "class", CLASS },
      { "else", ELSE },
      { "false", FALSE },
      { "for", FOR },
      { "fun", FUN },
      { "if", IF },
      { "nil", NIL },
      { "or", OR },
      { "return", RETURN },
      { "super", SUPER },
      { "this", THIS },
      { "true", TRUE },
      { "var", VAR },
      { "while", WHILE },
      { "break", BREAK},
      { "continue", CONTINUE},
    };

    public Scanner(string source)
    {
      this.source = source;
    }

    public List<Token> scanTokens()
    {
      while (!isAtEnd())
      {
        // We are at the beginning of the next lexeme.
        start = current;
        scanToken();
      }

      tokens.Add(new Token(EOF, "", null, line));
      return tokens;
    }

    private bool isAtEnd()
    {
      return current >= source.Length;
    }

    private void scanToken()
    {
      char c = advance();
      switch (c)
      {
      case '(': addToken(LEFT_PAREN); break;
      case ')': addToken(RIGHT_PAREN); break;
      case '{': addToken(LEFT_BRACE); break;
      case '}': addToken(RIGHT_BRACE); break;
      case ',': addToken(COMMA); break;
      case '.': addToken(DOT); break;
      case '-': addToken(MINUS); break;
      case '+': addToken(PLUS); break;
      case ';': addToken(SEMICOLON); break;
      case '*': addToken(STAR); break;
      case '/': addToken(SLASH); break;
      case ':': addToken(COLON); break;
      case '!' when match('='): addToken(BANG_EQUAL); break;
      case '!': addToken(BANG); break;
      case '=' when match('='): addToken(EQUAL_EQUAL); break;
      case '=': addToken(EQUAL); break;
      case '<' when match('='): addToken(LESS_EQUAL); break;
      case '<': addToken(LESS); break;
      case '>' when match('='): addToken(GREATER_EQUAL); break;
      case '>': addToken(GREATER); break;
      case '#': skipline(); break;
      case '\n': line++; break;
      case '"': scanstring(); break;
      case var _ when Char.IsDigit(c): number(); break;
      case var _ when Char.IsWhiteSpace(c): /*Ignore whitespace*/ break;
      case var _ when IsLetter(c): identifier(); break;
      default: Lox.error(line, $"Unexpected character {c}."); break;
      }
    }

    private char advance()
    {
      return source[current++];
    }

    private void addToken(TokenType type)
    {
      addToken(type, null);
    }

    private void addToken(TokenType type, Object literal)
    {
      string text = source[start..current];
      tokens.Add(new Token(type, text, literal, line));
    }

    private bool match(char expected)
    {
      if (isAtEnd()) return false;
      if (source[current] != expected)
      {
        return false;
      }
      current++;
      return true;
    }

    private char peek()
    {
      if (isAtEnd())
      {
        return '\0';
      }
      return source[current];
    }

    private void scanstring()
    {
      while (peek() != '"' && !isAtEnd())
      {
        if (peek() == '\n') line++;
        advance();
      }

      if (isAtEnd())
      {
        Lox.error(line, "Unterminated string.");
        return;
      }

      // The closing ".
      advance();

      // Trim the surrounding quotes.
      string value = source[(start + 1)..(current - 1)];
      addToken(STRING, value);
    }

    private void skipline()
    {
      while (peek() != '\n' && !isAtEnd())
      {
        advance();
      }
    }

    private void number()
    {
      while (Char.IsDigit(peek()))
      {
        advance();
      }
      // Look for a fractional part.
      if (peek() == '.' && Char.IsDigit(peekNext()))
      {
        // Consume the "."
        advance();

        while (Char.IsDigit(peek()))
        {
          advance();
        }
      }
      if (source[start..current].Contains('.'))
      {
        addToken(DOUBLE, double.Parse(source[start..current], CultureInfo.InvariantCulture));
      }
      else
      {
        addToken(INT, int.Parse(source[start..current], CultureInfo.InvariantCulture));
      }
    }

    private char peekNext()
    {
      if (current + 1 >= source.Length)
      {
        return '\0';
      }
      return source[current + 1];
    }
    private bool IsLetter(char c)
    {
      return Char.IsLetter(c) || c == '_';
    }
    private bool IsLetterOrDigit(char c)
    {
      return Char.IsLetterOrDigit(c) || c == '_';
    }
    private void identifier()
    {
      while (IsLetterOrDigit(peek()))
      {
        advance();
      }
      var text = source[start..current];
      if (!keywords.TryGetValue(text, out TokenType type))
      {
        type = IDENTIFIER;
      }
      addToken(type);
    }
  }
}

#endif // FOXLOX_SCANNER_H