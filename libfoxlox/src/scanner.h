#pragma once
#include <cstdint>
#include <charconv>
#include <string_view>
#include <vector>
#include <string_view>

#include <gsl/gsl>
#include <range/v3/all.hpp>
#include <fmt/format.h>
#include <tuple>

#include "token.h"
#include "util.h"

namespace foxlox
{
  class Scanner
  {
  public:
    Scanner(std::u32string&& s);
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
