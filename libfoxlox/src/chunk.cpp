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
  void Closure::edit_code(gsl::index idx, int16_t c)
  {
    struct Tmp { uint8_t a, b; };
    const auto tmp = std::bit_cast<Tmp>(c);
    code.at(idx) = tmp.a;
    code.at(idx + 1) = tmp.b;
  }
  gsl::index Closure::get_code_num()
  {
    return code.size();
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
  uint16_t Chunk::add_static_value()
  {
    assert(uint32_t(static_value_num) + 1 <= std::numeric_limits<uint16_t>::max());
    return static_value_num++;
  }
  uint16_t Chunk::get_static_value_num()
  {
    return static_value_num;
  }
  Chunk::Chunk()
  {
    static_value_num = 0;
    is_moved = false;
  }
  Chunk::~Chunk()
  {
    clean();
  }
  Chunk::Chunk(Chunk&& r) noexcept
  {
    is_moved = r.is_moved;
    source = std::move(r.source);
    closures = std::move(r.closures);
    constants = std::move(r.constants);
    const_strings = std::move(r.const_strings);
    static_value_num = r.static_value_num;
    r.is_moved = true;
  }
  Chunk& Chunk::operator=(Chunk&& r) noexcept
  {
    if (this == &r) { return *this; }
    clean();
    is_moved = r.is_moved;
    source = std::move(r.source);
    closures = std::move(r.closures);
    constants = std::move(r.constants);
    const_strings = std::move(r.const_strings);
    static_value_num = r.static_value_num;
    r.is_moved = true;
    return *this;
  }
  void Chunk::clean()
  {
    if (!is_moved)
    {
      for (String* p : const_strings)
      {
        String::free(p);
      }
    }
  }
  void Chunk::set_source(std::vector<std::string>&& src)
  {
    source = std::move(src);
  }
  std::string_view Chunk::get_source(gsl::index line_num) const
  {
    if (line_num < 0 || ssize(source) <= line_num) { return ""; }
    return source[line_num];
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