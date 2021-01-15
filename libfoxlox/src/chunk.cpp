#include <cassert>
#include <string_view>
#include <algorithm>
#include <limits>
#include <bit>

#include <gsl/gsl>

#include <foxlox/chunk.h>

namespace foxlox
{
  Subroutine::Subroutine(std::string_view func_name, int num_of_params) :
    arity(num_of_params), name(func_name), gc_mark(false)
  {
  }
  std::string_view Subroutine::get_funcname() const noexcept
  {
    return name;
  }
  int Subroutine::get_arity() const noexcept
  {
    return arity;
  }
  std::span<const uint8_t> Subroutine::get_code() const noexcept
  {
    return code;
  }
  std::vector<Subroutine>& Chunk::get_subroutines() noexcept
  {
    return subroutines;
  }
  std::span<const Subroutine> Chunk::get_subroutines() const noexcept
  {
    return subroutines;
  }
  std::span<const Value> Chunk::get_constants() const
  {
    return constants;
  }
  std::span<const String* const> Chunk::get_const_strings() const
  {
    return const_strings;
  }
  std::span<String*> Chunk::get_const_strings()
  {
    return const_strings;
  }
  void Subroutine::add_code(bool c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c ? uint8_t{ 1 } : uint8_t{ 0 });
  }
  void Subroutine::add_code(uint8_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c);
  }
  void Subroutine::add_code(int16_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    struct Tmp { uint8_t a, b; };
    const auto tmp = std::bit_cast<Tmp>(c);
    code.push_back(tmp.a);
    code.push_back(tmp.b);
  }
  void Subroutine::add_code(uint16_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    struct Tmp { uint8_t a, b; };
    const auto tmp = std::bit_cast<Tmp>(c);
    code.push_back(tmp.a);
    code.push_back(tmp.b);
  }
  void Subroutine::edit_code(gsl::index idx, int16_t c)
  {
    struct Tmp { uint8_t a, b; };
    const auto tmp = std::bit_cast<Tmp>(c);
    code.at(idx) = tmp.a;
    code.at(idx + 1) = tmp.b;
  }
  gsl::index Subroutine::get_code_num() const noexcept
  {
    return code.size();
  }
  void Subroutine::add_referenced_static_value(uint16_t idx)
  {
    if (std::ranges::find(referenced_static_values, idx) == referenced_static_values.end())
    {
      referenced_static_values.push_back(idx);
    }
  }
  std::span<const uint16_t> Subroutine::get_referenced_static_values() const noexcept
  {
    return referenced_static_values;
  }
  uint16_t Chunk::add_constant(Value v)
  {
    assert(v.type == Value::F64 || v.type == Value::I64);

    constants.push_back(v);
    const auto index = constants.size() - 1;
    assert(index <= std::numeric_limits<uint16_t>::max());
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_subroutine(std::string_view func_name, int num_of_params)
  {
    subroutines.emplace_back(func_name, num_of_params);
    const auto index = subroutines.size() - 1;
    assert(index <= std::numeric_limits<uint16_t>::max());
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_string(std::string_view str)
  {
    String* p = String::alloc([](auto l) {GSL_SUPPRESS(r.11) return new char[l]; }, str.size());
    GSL_SUPPRESS(stl.1) GSL_SUPPRESS(bounds.3)
    std::copy(str.begin(), str.end(), p->data());
    const_strings.push_back(p);
    const auto index = const_strings.size() - 1;
    assert(index <= std::numeric_limits<uint16_t>::max());
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_static_value() noexcept
  {
    assert(uint32_t(static_value_num) + 1 <= std::numeric_limits<uint16_t>::max());
    return static_value_num++;
  }
  uint16_t Chunk::get_static_value_num() const noexcept
  {
    return static_value_num;
  }
  Chunk::Chunk() noexcept
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
    subroutines = std::move(r.subroutines);
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
    subroutines = std::move(r.subroutines);
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
      for (const String* p : const_strings)
      {
        String::free([](auto p, auto) { 
          GSL_SUPPRESS(r.11) GSL_SUPPRESS(i.11) 
            delete[] p; 
          }, p);
      }
    }
  }
  void Chunk::set_source(std::vector<std::string>&& src) noexcept
  {
    source = std::move(src);
  }
  std::string_view Chunk::get_source(gsl::index line_num) const
  {
    if (line_num < 0) { return "<EOF>"; }
    if (ssize(source) <= line_num) { return ""; }
    return source.at(line_num);
  }
  const LineInfo& Subroutine::get_lines() const noexcept
  {
    return lines;
  }
  void LineInfo::add_line(gsl::index code_index, int line_num)
  {
    if (!lines.empty() && line_num == lines.back().line_num) { return; }
    lines.emplace_back(code_index, line_num);
  }
  int LineInfo::get_line(gsl::index code_index) const noexcept
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