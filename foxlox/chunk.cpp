#include <cassert>
#include <string_view>

#include <gsl/gsl>

#include "chunk.h"

namespace foxlox
{
  const Chunk::Code& Chunk::get_code() const
  {
    return code;
  }
  const ValueArray& Chunk::get_constants() const
  {
    return constants;
  }
  void Chunk::add_code(Inst c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c);
  }
  uint32_t Chunk::add_constant(Value v)
  {
    constants.push_back(v);
    const auto index = constants.size() - 1;
    assert(index <= INST_UA_MAX);
    return gsl::narrow<uint32_t>(index);
  }
  const LineInfo& Chunk::get_lines() const
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