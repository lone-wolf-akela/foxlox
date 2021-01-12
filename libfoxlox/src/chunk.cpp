#include <cassert>
#include <string_view>
#include <algorithm>
#include <limits>
#include <bit>

#include <gsl/gsl>

#include "chunk.h"

namespace foxlox
{
  const std::vector<uint8_t>& Closure::get_code() const
  {
    return code;
  }
  std::vector<Closure>& Chunk::get_closures()
  {
    return closures;
  }
  const std::vector<Closure>& Chunk::get_closures() const
  {
    return closures;
  }
  const std::vector<Value>& Chunk::get_constants() const
  {
    return constants;
  }
  const std::vector<String*>& Chunk::get_const_strings() const
  {
    return const_strings;
  }
  void Closure::add_code(bool c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c ? uint8_t(1) : uint8_t(0));
  }
  void Closure::add_code(uint8_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c);
  }
  void Closure::add_code(int16_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    struct Tmp { uint8_t a, b; };
    const auto tmp = std::bit_cast<Tmp>(c);
    code.push_back(tmp.a);
    code.push_back(tmp.b);
  }
  void Closure::add_code(uint16_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    struct Tmp { uint8_t a, b; };
    const auto tmp = std::bit_cast<Tmp>(c);
    code.push_back(tmp.a);
    code.push_back(tmp.b);
  }
  uint16_t Chunk::add_constant(Value v)
  {
    constants.push_back(v);
    const auto index = constants.size() - 1;
    assert(index <= std::numeric_limits<uint16_t>::max());
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_closure()
  {
    closures.emplace_back();
    const auto index = closures.size() - 1;
    assert(index <= std::numeric_limits<uint16_t>::max());
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_string(std::string_view str)
  {
    String* p = String::alloc(str.size());
    std::copy(str.begin(), str.end(), p->str);
    const_strings.push_back(p);
    const auto index = const_strings.size() - 1;
    assert(index <= std::numeric_limits<uint16_t>::max());
    return gsl::narrow_cast<uint16_t>(index);
  }
  int16_t Chunk::add_static_value(Value v)
  {
    static_value_pool.push_back(v);
    return gsl::narrow_cast<int16_t>(-ssize(static_value_pool));
  }
  Chunk::~Chunk()
  {
    for (String* p : const_strings)
    {
      String::free(p);
    }
  }
  const LineInfo& Closure::get_lines() const
  {
    return lines;
  }
  void LineInfo::add_line(gsl::index code_index, int line_num)
  {
    if (!lines.empty() && line_num == lines.back().line_num) { return; }
    lines.emplace_back(code_index, line_num);
  }
  int LineInfo::get_line(gsl::index code_index) const
  {
    auto last_line_num = lines.front().line_num;
    for (auto& line : lines)
    {
      if (line.code_index > code_index) { return last_line_num; }
      last_line_num = line.line_num;
    }
    return last_line_num;
  }
}