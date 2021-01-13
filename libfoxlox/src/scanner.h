#pragma once

#include <cassert>
#include <cstdint>
#include <charconv>
#include <string_view>
#include <vector>
#include <string_view>

#include <gsl/gsl>
#include <range/v3/all.hpp>
#include <fmt/format.h>

#include "token.h"
#include "util.h"

namespace foxlox
{
  class Scanner
  {
  public:
    Scanner(std::u32string&& s);
    std::vector<Token> scan_tokens();

  private:
    const std::u32string source;
    std::vector<Token> tokens;

    gsl::index start;
    gsl::index current;
    int line;

    bool is_at_end();
    void scan_token();
    char32_t advance();
    void add_token(TokenType type);
    void add_token(TokenType type, CompiletimeValue literal);
    void add_error(std::string_view msg);
    bool match(char32_t expected);
    char32_t peek();
    void scanstring();
    void skipline();
    void number();
    char32_t peek_next();
    void identifier();
  };
}
