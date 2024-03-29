module;
module foxlox:chunk;

import <cassert>;
import <gsl/gsl>;

import :chunk;

namespace foxlox
{
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
  Chunk* Subroutine::get_chunk() const noexcept
  {
    return chunk;
  }
  void Subroutine::set_chunk(Chunk* c) noexcept
  {
    chunk = c;
  }
  uint16_t Chunk::add_constant(int64_t v)
  {
    constants.push_back(v);
    const auto index = constants.size() - 1;
    if (index > std::numeric_limits<uint16_t>::max())
    {
      throw ChunkOperationError("Too many constants. Chunk constant table is full.");
    }
    return gsl::narrow_cast<uint16_t>(index);
  }
  uint16_t Chunk::add_constant(double v)
  {
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
  LineInfo::LineNum::LineNum(int64_t code_idx, int line_n) noexcept :
    code_index(code_idx),
    line_num(line_n)
  {
  }
  void LineInfo::LineNum::dump(std::ostream& strm) const
  {
    dump_int64(strm, code_index);
    dump_int32(strm, line_num);
  }
  LineInfo::LineNum LineInfo::LineNum::load(std::istream& strm)
  {
    const int64_t code_index = load_int64(strm);
    const int32_t line_num = load_int32(strm);
    return LineNum(code_index, line_num);
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
  void LineInfo::dump(std::ostream& strm) const
  {
    dump_int64(strm, ssize(lines));
    for (const auto& line : lines)
    {
      line.dump(strm);
    }
  }
  LineInfo LineInfo::load(std::istream& strm)
  {
    LineInfo info;
    const int64_t n = load_int64(strm);
    info.lines.reserve(n);
    for (int64_t i = 0; i < n; i++)
    {
      info.lines.emplace_back(LineNum::load(strm));
    }
    return info;
  }
  void Subroutine::dump(std::ostream& strm) const
  {
    dump_int32(strm, arity);
    dump_int64(strm, ssize(code));
    for (const auto c : code)
    {
      dump_uint8(strm, c);
    }
    dump_str(strm, name);
    lines.dump(strm);
    dump_int64(strm, ssize(referenced_static_values));
    for (const auto v : referenced_static_values)
    {
      dump_uint16(strm, v);
    }
  }
  Subroutine Subroutine::load(std::istream& strm)
  {
    const int32_t arity = load_int32(strm);
    const int64_t code_len = load_int64(strm);
    std::vector<uint8_t> code;
    code.reserve(code_len);
    for (int64_t i = 0; i < code_len; i++)
    {
      code.emplace_back(load_uint8(strm));
    }
    const std::string name = load_str(strm);
    Subroutine routine(name, arity);
    routine.code = std::move(code);
    routine.lines = LineInfo::load(strm);
    const int64_t referenced_len = load_int64(strm);
    routine.referenced_static_values.reserve(referenced_len);
    for (int64_t i = 0; i < referenced_len; i++)
    {
      routine.referenced_static_values.emplace_back(load_uint16(strm));
    }
    return routine;
  }
  void Chunk::dump(std::ostream& strm) const
  {
    dump_str(strm, source_path);
    dump_int64(strm, ssize(source));
    for (const auto& str : source)
    {
      dump_str(strm, str);
    }
    dump_int64(strm, ssize(subroutines));
    for (const auto& routine : subroutines)
    {
      routine.dump(strm);
    }
    dump_int64(strm, ssize(classes));
    for (const auto& klass : classes)
    {
      klass.dump(strm);
    }
    dump_int64(strm, ssize(export_list));
    for (const auto& exp : export_list)
    {
      dump_uint16(strm, exp.name_idx);
      dump_uint16(strm, exp.value_idx);
    }
    dump_int64(strm, ssize(constants));
    for (const auto& v : constants)
    {
      if (std::holds_alternative<int64_t>(v))
      {
        dump_uint8(strm, 0);
        dump_int64(strm, std::get<int64_t>(v));
      }
      else
      {
        assert(std::holds_alternative<double>(v));
        dump_uint8(strm, 1);
        dump_double(strm, std::get<double>(v));
      }
    }
    dump_int64(strm, ssize(const_strings));
    for (const auto& str : const_strings)
    {
      dump_str(strm, str);
    }
    dump_uint16(strm, static_value_num);
  }
  Chunk Chunk::load(std::istream& strm)
  {
    Chunk chunk;
    chunk.source_path = load_str(strm);
    const int64_t src_len = load_int64(strm);
    chunk.source.reserve(src_len);
    for (int64_t i = 0; i < src_len; i++)
    {
      chunk.source.emplace_back(load_str(strm));
    }
    const int64_t subr_len = load_int64(strm);
    chunk.subroutines.reserve(subr_len);
    for (int64_t i = 0; i < subr_len; i++)
    {
      chunk.subroutines.emplace_back(Subroutine::load(strm));
      chunk.subroutines.back().set_chunk(&chunk);
    }
    const int64_t classes_len = load_int64(strm);
    chunk.classes.reserve(classes_len);
    for (int64_t i = 0; i < classes_len; i++)
    {
      chunk.classes.emplace_back(CompiletimeClass::load(strm));
    }
    const int64_t export_len = load_int64(strm);
    chunk.export_list.reserve(export_len);
    for (int64_t i = 0; i < export_len; i++)
    {
      auto name_idx = load_uint16(strm);
      auto value_idx = load_uint16(strm);
      chunk.export_list.emplace_back(name_idx, value_idx);
    }
    const int64_t const_len = load_int64(strm);
    chunk.constants.reserve(const_len);
    for (int64_t i = 0; i < const_len; i++)
    {
      const uint8_t type = load_uint8(strm);
      if (type == 0)
      {
        chunk.constants.emplace_back(load_int64(strm));
      }
      else
      {
        assert(type == 1);
        chunk.constants.emplace_back(load_double(strm));
      }
    }
    const int64_t const_str_len = load_int64(strm);
    chunk.const_strings.reserve(const_str_len);
    for (int64_t i = 0; i < const_str_len; i++)
    {
      chunk.const_strings.emplace_back(load_str(strm));
    }
    chunk.static_value_num = load_uint16(strm);

    return chunk;
  }
  Chunk::Chunk(Chunk&& o) noexcept :
    source_path(std::move(o.source_path)),
    source(std::move(o.source)),
    subroutines(std::move(o.subroutines)),
    classes(std::move(o.classes)),
    export_list(std::move(o.export_list)),
    constants(std::move(o.constants)),
    const_strings(std::move(o.const_strings)),
    static_value_num(o.static_value_num),
    static_value_idx_base(o.static_value_idx_base),
    class_idx_base(o.class_idx_base),
    const_string_idx_base(o.const_string_idx_base)
  {
    for (auto& subr : subroutines)
    {
      subr.set_chunk(this);
    }
  }
  Chunk& Chunk::operator=(Chunk&& o) noexcept
  {
    source_path = std::move(o.source_path);
    source = std::move(o.source);
    subroutines = std::move(o.subroutines);
    classes = std::move(o.classes);
    export_list = std::move(o.export_list);
    constants = std::move(o.constants);
    const_strings = std::move(o.const_strings);
    static_value_num = o.static_value_num;
    static_value_idx_base = o.static_value_idx_base;
    class_idx_base = o.class_idx_base;
    const_string_idx_base = o.const_string_idx_base;
    for (auto& subr : subroutines)
    {
      subr.set_chunk(this);
    }
    return *this;
  }

  std::string_view Chunk::get_src_path() const noexcept
  {
    return source_path;
  }
  void Chunk::set_src_path(std::string_view path)
  {
    source_path = path;
  }

  std::vector<Subroutine>& Chunk::get_subroutines() noexcept
  {
    return subroutines;
  }
  std::span<const Subroutine> Chunk::get_subroutines() const noexcept
  {
    return subroutines;
  }
  Value Chunk::get_constant(uint16_t idx) const
  {
    const auto v = constants.at(idx);
    if (std::holds_alternative<int64_t>(v))
    {
      return std::get<int64_t>(v);
    }
    else // double
    {
      return std::get<double>(v);
    }
  }
  std::span<const std::string> Chunk::get_const_strings() const
  {
    return const_strings;
  }

  void Chunk::add_export(std::string_view name, uint16_t idx)
  {
    export_list.emplace_back(add_string(name), idx);
  }
  std::span<const CompiletimeExport> Chunk::get_export_list() const noexcept
  {
    return export_list;
  }

  void Chunk::set_static_value_idx_base(size_t n) noexcept
  {
    static_value_idx_base = n;
  }
  size_t Chunk::get_static_value_idx_base() const noexcept
  {
    return static_value_idx_base;
  }
  void Chunk::set_class_idx_base(size_t n) noexcept
  {
    class_idx_base = n;
  }
  size_t Chunk::get_class_idx_base() const noexcept
  {
    return class_idx_base;
  }
  void Chunk::set_const_string_idx_base(size_t n) noexcept
  {
    const_string_idx_base = n;
  }
  size_t Chunk::get_const_string_idx_base() const noexcept
  {
    return const_string_idx_base;
  }
}