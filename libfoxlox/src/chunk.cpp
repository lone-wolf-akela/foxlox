#include <cassert>
#include <string_view>
#include <algorithm>
#include <limits>
#include <bit>

#include <gsl/gsl>

#include "object.h"

#include <foxlox/chunk.h>

namespace foxlox
{

  // need this for Value to work correctly
  static_assert(std::alignment_of_v<Subroutine> >= (1u << Value::method_func_shift));

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
    add_code(std::bit_cast<uint16_t>(c), line_num);
  }
  void Subroutine::add_code(uint16_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(gsl::narrow_cast<uint8_t>(c >> 8));
    code.push_back(gsl::narrow_cast<uint8_t>(c & 0xff));
  }
  void Subroutine::edit_code(gsl::index idx, int16_t c)
  {
    edit_code(idx, std::bit_cast<uint16_t>(c));
  }
  void Subroutine::edit_code(gsl::index idx, uint16_t c)
  {
    code.at(idx) = gsl::narrow_cast<uint8_t>(c >> 8);
    code.at(idx + 1) = gsl::narrow_cast<uint8_t>(c & 0xff);
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
    assert(v.type == ValueType::F64 || v.type == ValueType::I64 || v.type == ValueType::CPP_FUNC);

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
    const auto it = std::ranges::find(const_strings, str, [](String* s) {return s->get_view(); });
    if (it != const_strings.end())
    {
      const auto index = std::distance(const_strings.begin(), it);
      return gsl::narrow_cast<uint16_t>(index);
    }

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
    classes = std::move(r.classes);
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
    classes = std::move(r.classes);
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
    if (line_num <= -1) { return "<EOF>"; }
    if (line_num == 0) { return "<RUNTIME>"; }
    if (ssize(source) < line_num) { return ""; }
    return source.at(line_num - 1);
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
  uint16_t Chunk::add_class(Class&& klass)
  {
    classes.emplace_back(std::move(klass));
    const auto index = classes.size() - 1;
    assert(index <= std::numeric_limits<uint16_t>::max());
    return gsl::narrow_cast<uint16_t>(index);
  }
  std::span<const Class> Chunk::get_classes() const noexcept
  {
    return classes;
  }
  std::vector<Class>& Chunk::get_classes() noexcept
  {
    return classes;
  }
}