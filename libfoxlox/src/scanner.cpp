#include <utility>
#include <map>

#ifdef _WIN32
#include <icu.h>
#pragma comment(lib, "icu.lib") 
#undef FALSE
#undef TRUE
#endif

#include "scanner.h"

namespace
{
  using namespace foxlox;

  bool is_digit(char32_t c)
  {
    return u_isdigit(c);
  }
  bool is_whitespace(char32_t c)
  {
    return u_isWhitespace(c);
  }
  bool is_letter(char32_t c)
  {
    return u_isalpha(c) || c == '_';
  }
  bool is_letter_or_digit(char32_t c)
  {
    return u_isalnum(c) || c == U'_';
  }

  const std::map<std::u32string_view, TokenType> keywords
  {
    { U"and", TokenType::AND },
    { U"class", TokenType::CLASS },
    { U"else", TokenType::ELSE },
    { U"false", TokenType::FALSE },
    { U"for", TokenType::FOR },
    { U"fun", TokenType::FUN },
    { U"if", TokenType::IF },
    { U"nil", TokenType::NIL },
    { U"or", TokenType::OR },
    { U"return", TokenType::RETURN },
    { U"super", TokenType::SUPER },
    { U"this", TokenType::THIS },
    { U"true", TokenType::TRUE },
    { U"var", TokenType::VAR },
    { U"while", TokenType::WHILE },
    { U"break", TokenType::BREAK},
    { U"continue", TokenType::CONTINUE},
  };
}

namespace foxlox
{
  Scanner::Scanner(std::u32string&& s) : source(std::move(s))
  {
    start = 0;
    current = 0;
    line = 1;
  }
  std::vector<Token> Scanner::scan_tokens()
  {
    tokens = std::vector<Token>();
    while (!is_at_end())
    {
      // We are at the beginning of the next lexeme.
      start = current;
      scan_token();
    }

    tokens.emplace_back(Token(TokenType::TKEOF, "", {}, line));
    return std::move(tokens);
  }
  bool Scanner::is_at_end()
  {
    return current >= ssize(source);
  }
  void Scanner::scan_token()
  {
    const auto c = advance();
    switch (c)
    {
    case '(': add_token(TokenType::LEFT_PAREN); break;
    case ')': add_token(TokenType::RIGHT_PAREN); break;
    case '{': add_token(TokenType::LEFT_BRACE); break;
    case '}': add_token(TokenType::RIGHT_BRACE); break;
    case ',': add_token(TokenType::COMMA); break;
    case '.': add_token(TokenType::DOT); break;
    case '-': add_token(TokenType::MINUS); break;
    case '+': add_token(TokenType::PLUS); break;
    case ';': add_token(TokenType::SEMICOLON); break;
    case '*': add_token(TokenType::STAR); break;
    case '/':  
    {
      if (match('/')) { add_token(TokenType::SLASH_SLASH); }
      else { add_token(TokenType::SLASH); }
      break;
    }
    case ':': add_token(TokenType::COLON); break;
    case '!':
    {
      if (match('=')) { add_token(TokenType::BANG_EQUAL); }
      else { add_token(TokenType::BANG); }
      break;
    }
    case '=':
    {
      if (match('=')) { add_token(TokenType::EQUAL_EQUAL); }
      else { add_token(TokenType::EQUAL); }
      break;
    }
    case '<':
    {
      if (match('=')) { add_token(TokenType::LESS_EQUAL); }
      else { add_token(TokenType::LESS); }
      break;
    }
    case '>':
    {
      if (match('=')) { add_token(TokenType::GREATER_EQUAL); }
      else { add_token(TokenType::GREATER); }
      break;
    }
    case '#': skipline(); break;
    case '\n': line++; break;
    case '"': scanstring(); break;
    default:
      if (is_digit(c)) { number(); }
      else if (is_whitespace(c)) { /*Ignore whitespace*/ }
      else if (is_letter(c)) { identifier(); }
      else { add_error(fmt::format("Unexpected character `{}'.", u32_to_u8(c))); }
      break;
    }
  }
  char32_t Scanner::advance()
  {
    return source[current++];
  }
  void Scanner::add_token(TokenType type)
  {
    add_token(type, {});
  }
  void Scanner::add_token(TokenType type, CompiletimeValue literal)
  {
    auto u32text = source.substr(start, current - start);
    const auto u8text = u32_to_u8(u32text);
    tokens.emplace_back(type, u8text, literal, line);
  }
  void Scanner::add_error(std::string_view msg)
  {
    tokens.emplace_back(TokenType::TKERROR, msg, CompiletimeValue(), line);
  }
  bool Scanner::match(char32_t expected)
  {
    if (is_at_end()) { return false; }
    if (source[current] != expected) { return false; }
    current++;
    return true;
  }

  char32_t Scanner::peek()
  {
    if (is_at_end()) { return '\0'; }
    return source[current];
  }

  void Scanner::scanstring()
  {
    while (peek() != '"' && !is_at_end())
    {
      if (peek() == '\n') { line++; }
      advance();
    }

    if (is_at_end())
    {
      add_error("Unterminated string.");
      return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    auto u32value = source.substr(start + 1, current - start - 2);
    const auto u8value = u32_to_u8(u32value);
    add_token(TokenType::STRING, CompiletimeValue(u8value));
  }

  void Scanner::skipline()
  {
    while (peek() != '\n' && !is_at_end())
    {
      advance();
    }
  }

  void Scanner::number()
  {
    while (is_digit(peek()))
    {
      advance();
    }
    // Look for a fractional part.
    if (peek() == '.' && is_digit(peek_next()))
    {
      // Consume the "."
      advance();

      while (is_digit(peek()))
      {
        advance();
      }
    }
    auto u32substr = source.substr(start, current - start);
    const auto u8substr = u32_to_u8(u32substr);
    if (u8substr.find(U'.') != decltype(u8substr)::npos)
    {
      double f64;
      const auto r = std::from_chars(u8substr.data(), u8substr.data() + u8substr.size(), f64);
      assert(r.ptr == u8substr.data() + u8substr.size());
      add_token(TokenType::DOUBLE, f64);
    }
    else
    {
      int64_t i64;
      const auto r = std::from_chars(u8substr.data(), u8substr.data() + u8substr.size(), i64);
      assert(r.ptr == u8substr.data() + u8substr.size());
      add_token(TokenType::INT, i64);
    }
  }

  char32_t Scanner::peek_next()
  {
    if (current + 1 >= ssize(source))
    {
      return '\0';
    }
    return source[current + 1];
  }

  void Scanner::identifier()
  {
    while (is_letter_or_digit(peek()))
    {
      advance();
    }
    auto text = source.substr(start, current - start);
    TokenType type;
    if (const auto found = keywords.find(text); found == keywords.end())
    {
      type = TokenType::IDENTIFIER;
    }
    else
    {
      type = found->second;
    }
    add_token(type);
  }
}