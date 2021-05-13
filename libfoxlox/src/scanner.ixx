export module foxlox:scanner;

import <cstdint>;
import <charconv>;
import <string_view>;
import <vector>;
import <string_view>;
import <tuple>;
import <utility>;
import <map>;
import <sstream>;
import <version>;
import <format>;

import <gsl/gsl>;
import "libicu.h";

import :util;
import :token;

//TODO: wait for a vs update fix this
#undef TRUE
#undef FALSE

namespace foxlox
{
  export class Scanner
  {
  public:
    explicit Scanner(std::u32string&& s) noexcept;
    // return tokens with each line of the source file
    std::tuple<std::vector<Token>, std::vector<std::string>> scan_tokens();

  private:
    const std::u32string source;
    std::vector<std::string> source_per_line;
    std::vector<Token> tokens;

    gsl::index last_line_end;
    gsl::index start;
    gsl::index current;
    int line;

    bool is_at_end();
    void scan_token();
    char32_t advance();
    void add_src_line();
    void add_token(TokenType type);
    void add_token(TokenType type, CompiletimeValue literal);
    void add_error(std::string_view msg);
    bool match(char32_t expected);
    char32_t peek();
    void scanstring();
    std::tuple<char32_t, bool> hexstr_to_u32char(std::u32string_view hexstr);
    void skipline();
    void number();
    char32_t peek_next();
    void identifier();
  };
}

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
    { U"break", TokenType::BREAK },
    { U"continue", TokenType::CONTINUE },
    { U"from", TokenType::FROM },
    { U"import", TokenType::IMPORT },
    { U"as", TokenType::AS },
    { U"export", TokenType::EXPORT }
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
      add_src_line();
      break;
    }
    case U'"': scanstring(); break;
    default:
      if (is_digit(c)) { number(); }
      else if (is_whitespace(c)) { /*Ignore whitespace*/ }
      else if (is_letter(c)) { identifier(); }
      else { add_error(std::format("Unexpected character `{}'.", u32_to_u8(c))); }
      break;
    }
  }
  char32_t Scanner::advance()
  {
    return source.at(current++);
  }
  void Scanner::add_src_line()
  {
    const auto a_line_u32 = std::u32string_view(source).substr(last_line_end, current - last_line_end - 1);
    source_per_line.emplace_back(u32_to_u8(a_line_u32));
    last_line_end = current;
    line++;
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
      if (peek() == '\\')
      {
        // escape the next char
        advance();
      }
      if (peek() == '\n')
      {
        add_src_line();
      }
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
    auto u32str = source.substr(start + 1, current - start - 2);

    // handle the escape sequences
    std::ostringstream unescaped_strm;
    enum
    {
      NON, SLASH, OCT, HEX, U16, U32
    } state = NON;
    gsl::index idx_seq_begin = 0;
    // we parse one elem post to the end of u32str
    // which is valid because that is the U'\0'
    // this is to allow we parse escape sequence like '\0' at the end of the string
    for (gsl::index i = 0; i < ssize(u32str) + 1; i++)
    {
      const char32_t c = u32str.c_str()[i];
      switch (state)
      {
      case NON:
      {
        if (c == U'\\')
        {
          state = SLASH;
        }
        else
        {
          unescaped_strm << u32_to_u8(c);
        }
        break;
      }
      case SLASH:
      {
        if (c == U'\'' || c == U'\"' || c == U'\?' || c == U'\\')
        {
          unescaped_strm << gsl::narrow_cast<char>(c);
          state = NON;
        }
        else if (c == U'a')
        {
          unescaped_strm << '\a';
          state = NON;
        }
        else if (c == U'b')
        {
          unescaped_strm << '\b';
          state = NON;
        }
        else if (c == U'f')
        {
          unescaped_strm << '\f';
          state = NON;
        }
        else if (c == U'n')
        {
          unescaped_strm << '\n';
          state = NON;
        }
        else if (c == U'r')
        {
          unescaped_strm << '\r';
          state = NON;
        }
        else if (c == U't')
        {
          unescaped_strm << '\t';
          state = NON;
        }
        else if (c == U'v')
        {
          unescaped_strm << '\v';
          state = NON;
        }
        else if (U'0' <= c && c <= U'7')
        {
          state = OCT;
          idx_seq_begin = i;
        }
        else if (c == U'x')
        {
          state = HEX;
          idx_seq_begin = i + 1;
        }
        else if (c == U'u')
        {
          state = U16;
          idx_seq_begin = i + 1;
        }
        else if (c == U'U')
        {
          state = U32;
          idx_seq_begin = i + 1;
        }
        else
        {
          add_error("Invalid escaped sequence in string.");
          return;
        }
        break;
      }
      case OCT:
      {
        const auto seq_len = i - idx_seq_begin;
        if (c < U'0' || c > U'7' || seq_len >= 3)
        {
          char32_t sum = 0;
          for (char32_t oct : u32str.substr(idx_seq_begin, seq_len))
          {
            sum = sum * 8 + (oct - U'0');
          }
          unescaped_strm << gsl::narrow_cast<char>(sum);
          // rewind i as u32str[i] is not in this oct seq
          i--;
          state = NON;
        }
        break;
      }
      case HEX:
      {
        const auto seq_len = i - idx_seq_begin;
        if (!(U'0' <= c && c <= U'9')
          && !(U'a' <= c && c <= U'f')
          && !(U'A' <= c && c <= U'F'))
        {
          auto [val, success] = hexstr_to_u32char(u32str.substr(idx_seq_begin, seq_len));
          if (!success) { return; }
          unescaped_strm << gsl::narrow_cast<char>(val);
          // rewind i as u32str[i] is not in this hex seq
          i--;
          state = NON;
        }
        break;
      }
      case U16:
      {
        const auto seq_len = i - idx_seq_begin + 1;
        if (seq_len >= 4)
        {
          auto [val, success] = hexstr_to_u32char(u32str.substr(idx_seq_begin, seq_len));
          if (!success) { return; }
          unescaped_strm << u32_to_u8(val);
          state = NON;
        }
        break;
      }
      case U32:
      {
        const auto seq_len = i - idx_seq_begin + 1;
        if (seq_len >= 8)
        {
          auto [val, success] = hexstr_to_u32char(u32str.substr(idx_seq_begin, seq_len));
          if (!success) { return; }
          unescaped_strm << u32_to_u8(val);
          state = NON;
        }
        break;
      }
      default: // ???
        break;
      }
    }

    auto parsed_str = unescaped_strm.view();

    // remove the last '\0' in parsed_str
    parsed_str = parsed_str.substr(0, parsed_str.size() - 1);
    add_token(TokenType::STRING, CompiletimeValue(parsed_str));
  }

  std::tuple<char32_t, bool> Scanner::hexstr_to_u32char(std::u32string_view hexstr)
  {
    char32_t sum = 0;
    for (const char32_t hex : hexstr)
    {
      if (!(U'0' <= hex && hex <= U'9')
        && !(U'a' <= hex && hex <= U'f')
        && !(U'A' <= hex && hex <= U'F'))
      {
        add_error("Invalid hexadecimal number in Unicode value.");
        return std::make_tuple(sum, false);
      }
      const char32_t v = (U'0' <= hex && hex <= U'9') ? hex - U'0' :
        (U'a' <= hex && hex <= U'f') ? hex - U'a' + 10 :
        hex - U'A' + 10;
      sum = sum * 16 + v;
    }
    return std::make_tuple(sum, true);
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
      const auto r = std::from_chars(u8substr.data(), u8substr.data() + u8substr.size(), f64);
      if (r.ptr == u8substr.data() + u8substr.size())
      {
        add_token(TokenType::DOUBLE, f64);
      }
      else
      {
        add_error("Wrong number format.");
      }
    }
    else
    {
      int64_t i64{};
      const auto r = std::from_chars(u8substr.data(), u8substr.data() + u8substr.size(), i64);
      if (r.ptr == u8substr.data() + u8substr.size())
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