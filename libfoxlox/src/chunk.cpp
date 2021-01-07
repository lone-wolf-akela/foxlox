#include <cassert>
#include <string_view>

#include <gsl/gsl>
#include <algorithm>

#include "chunk.h"

namespace foxlox
{
  const std::vector<Inst>& Closure::get_code() const
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
  void Closure::add_code(Inst c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c);
  }
  uint32_t Chunk::add_constant(Value v)
  {
    constants.push_back(v);
    const auto index = constants.size() - 1;
    assert(index <= INST_UA_MAX);
    return gsl::narrow_cast<uint32_t>(index);
  }
  uint32_t Chunk::add_closure()
  {
    closures.emplace_back();
    const auto index = closures.size() - 1;
    assert(index <= INST_UA_MAX);
    return gsl::narrow_cast<uint32_t>(index);
  }
  uint32_t Chunk::add_string(std::string_view str)
  {
    String* p = String::alloc(str.size());
    std::copy(str.begin(), str.end(), p->str);
    const_strings.push_back(p);
    const auto index = const_strings.size() - 1;
    assert(index <= INST_UA_MAX);
    return gsl::narrow_cast<uint32_t>(index);
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