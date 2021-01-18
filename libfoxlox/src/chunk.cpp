#include <string_view>
#include <algorithm>
#include <limits>
#include <bit>

#include <gsl/gsl>

#include <foxlox/except.h>
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
  std::span<const std::string> Chunk::get_const_strings() const
  {
    return const_strings;
  }
  void Subroutine::add_code(bool c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c ? uint8_t{ 1 } : uint8_t{ 0 });
  }
  void Subroutine::add_code(OP c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(static_cast<uint8_t>(c));
  }
  void Subroutine::add_code(uint8_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c);
  }
  void Subroutine::add_code(int16_t c, int line_num)
  {
    add_code(static_cast<uint16_t>(c), line_num);
  }
  void Subroutine::add_code(uint16_t c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(gsl::narrow_cast<uint8_t>(c >> 8));
    code.push_back(gsl::narrow_cast<uint8_t>(c & 0xff));
  }
  void Subroutine::edit_code(gsl::index idx, int16_t c)
  {
    edit_code(idx, static_cast<uint16_t>(c));
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
  bool Subroutine::is_marked() const noexcept
  {
    return gc_mark;
  }
  void Subroutine::mark() noexcept
  {
    gc_mark = true;
  }
  void Subroutine::unmark() noexcept
  {
    gc_mark = false;
  }
  uint16_t Chunk::add_constant(Value v)
  {
    if (v.type != ValueType::F64 && v.type != ValueType::I64 && v.type != ValueType::CPP_FUNC)
    {
      throw FatalError("Wrong constant type.");
    }

    constants.push_back(v);
    const auto index = constants.size() - 1;
    if (index > std::numeric_limits<uint16_t>::max())
    {
      throw ChunkOperationError("Too many constants. Chunk constant table is full.");
    }
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_subroutine(std::string_view func_name, int num_of_params)
  {
    subroutines.emplace_back(func_name, num_of_params);
    const auto index = subroutines.size() - 1;
    if (index > std::numeric_limits<uint16_t>::max())
    {
      throw ChunkOperationError("Too many subroutines. Chunk subroutine table is full.");
    }
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_string(std::string_view str)
  {
    const auto it = std::ranges::find(const_strings, str);
    if (it != const_strings.end())
    {
      const auto index = std::distance(const_strings.begin(), it);
      return gsl::narrow_cast<uint16_t>(index);
    }

    const_strings.emplace_back(str);
    const auto index = const_strings.size() - 1;
    if (index > std::numeric_limits<uint16_t>::max())
    {
      throw ChunkOperationError("Too many strings. Chunk string table is full.");
    }
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_static_value()
  {
    if (uint32_t(static_value_num) + 1 > std::numeric_limits<uint16_t>::max())
    {
      throw ChunkOperationError("Too many static values. Chunk static table is full.");
    }
    return static_value_num++;
  }
  uint16_t Chunk::get_static_value_num() const noexcept
  {
    return static_value_num;
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
  uint16_t Chunk::add_class(CompiletimeClass&& klass)
  {
    classes.emplace_back(std::move(klass));
    const auto index = classes.size() - 1;
    if (index > std::numeric_limits<uint16_t>::max())
    {
      throw ChunkOperationError("Too many classes. Chunk class table is full.");
    }
    return gsl::narrow_cast<uint16_t>(index);
  }
  std::span<const CompiletimeClass> Chunk::get_classes() const noexcept
  {
    return classes;
  }
}