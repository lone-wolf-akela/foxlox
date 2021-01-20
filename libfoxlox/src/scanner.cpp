#include <utility>
#include <map>
#include <version>

#include <unicode/uchar.h>
#undef FALSE
#undef TRUE

#include "scanner.h"

namespace
{
  using namespace foxlox;

  bool is_digit(char32_t c) noexcept
  {
    return u_isdigit(c);
  }
  bool is_whitespace(char32_t c) noexcept
  {
    return u_isWhitespace(c);
  }
  bool is_letter(char32_t c) noexcept
  {
    return u_isalpha(c) || c == '_';
  }
  bool is_letter_or_digit(char32_t c) noexcept 
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
  Scanner::Scanner(std::u32string&& s) noexcept :
    source(std::move(s)),
    last_line_end(0),
    start(0),
    current(0),
    line(1)
  {
  }
  std::tuple<std::vector<Token>, std::vector<std::string>> Scanner::scan_tokens()
  {
    tokens = std::vector<Token>();
    while (!is_at_end())
    {
      // We are at the beginning of the next lexeme.
      start = current;
      scan_token();
    }

    tokens.emplace_back(Token(TokenType::TKEOF, "", {}, line));
    return std::make_tuple(std::move(tokens), std::move(source_per_line));
  }
  bool Scanner::is_at_end()
  {
    return current >= ssize(source);
  }
  void Scanner::scan_token()
  {
    const char32_t c = advance();
    switch (c)
    {
    case U'(': add_token(TokenType::LEFT_PAREN); break;
    case U')': add_token(TokenType::RIGHT_PAREN); break;
    case U'{': add_token(TokenType::LEFT_BRACE); break;
    case U'}': add_token(TokenType::RIGHT_BRACE); break;
    case U',': add_token(TokenType::COMMA); break;
    case U'.': add_token(TokenType::DOT); break;
    case U'-': 
    {
      if (match(U'-')) { add_token(TokenType::MINUS_MINUS); }
      else if (match(U'=')) { add_token(TokenType::MINUS_EQUAL); }
      else { add_token(TokenType::MINUS); }
      break;
    }
    case U'+':
    {
      if (match(U'+')) { add_token(TokenType::PLUS_PLUS); }
      else if (match(U'=')) { add_token(TokenType::PLUS_EQUAL); }
      else { add_token(TokenType::PLUS); }
      break;
    }
    case U';': add_token(TokenType::SEMICOLON); break;
    case U'*': 
    {
      if (match(U'=')) { add_token(TokenType::STAR_EQUAL); }
      else { add_token(TokenType::STAR); }
      break;
    }
    case U'/':
    {
      if (match(U'/')) 
      { 
        if (match(U'=')) { add_token(TokenType::SLASH_SLASH_EQUAL); }
        else { add_token(TokenType::SLASH_SLASH); }
      }
      else 
      { 
        if (match(U'=')) { add_token(TokenType::SLASH_EQUAL); }
        else { add_token(TokenType::SLASH); }
      }
      break;
    }
    case U':': add_token(TokenType::COLON); break;
    case U'!':
    {
      if (match(U'=')) { add_token(TokenType::BANG_EQUAL); }
      else { add_token(TokenType::BANG); }
      break;
    }
    case U'=':
    {
      if (match(U'=')) { add_token(TokenType::EQUAL_EQUAL); }
      else { add_token(TokenType::EQUAL); }
      break;
    }
    case U'<':
    {
      if (match(U'=')) { add_token(TokenType::LESS_EQUAL); }
      else { add_token(TokenType::LESS); }
      break;
    }
    case U'>':
    {
      if (match(U'=')) { add_token(TokenType::GREATER_EQUAL); }
      else { add_token(TokenType::GREATER); }
      break;
    }
    case U'#': skipline(); break;
    case U'\n':
    {
      const auto a_line_u32 = std::u32string_view(source).substr(last_line_end, current - last_line_end - 1);
      source_per_line.emplace_back(u32_to_u8(a_line_u32));
      last_line_end = current;
      line++;
      break;
    }
    case U'"': scanstring(); break;
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
    return source.at(current++);
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
    if (source.at(current) != expected) { return false; }
    current++;
    return true;
  }

  char32_t Scanner::peek()
  {
    if (is_at_end()) { return '\0'; }
    return source.at(current);
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

  GSL_SUPPRESS(bounds.1)
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
      double f64{};
#if __cpp_lib_to_chars >= 201611L
      const auto r = std::from_chars(u8substr.data(), u8substr.data() + u8substr.size(), f64);
      if(r.ptr == u8substr.data() + u8substr.size())
      {
        add_token(TokenType::DOUBLE, f64);
      }
      else
      {
        add_error("Wrong number format.");
      }
#else
#pragma message("no floating point std::from_chars support!")
      f64 = std::stod(u8substr);
      add_token(TokenType::DOUBLE, f64);
#endif
    }
    else
    {
      int64_t i64{};
      const auto r = std::from_chars(u8substr.data(), u8substr.data() + u8substr.size(), i64);
      if(r.ptr == u8substr.data() + u8substr.size())
      {
        add_token(TokenType::INT, i64);
      }
      else
      {
        add_error("Wrong number format.");
      }
    }
  }

  char32_t Scanner::peek_next()
  {
    if (current + 1 >= ssize(source))
    {
      return '\0';
    }
    return source.at(current + 1);
  }

  void Scanner::identifier()
  {
    while (is_letter_or_digit(peek()))
    {
      advance();
    }
    auto text = source.substr(start, current - start);
    TokenType type{};
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