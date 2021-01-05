#include <fmt/format.h>
#include <gsl/gsl>
#include <range/v3/all.hpp>

#include "debug.h"

namespace foxlox
{
  void disassemble_inst(const Chunk& chunk, Inst inst, gsl::index index)
  {
    const int last_line_num = index == 0 ? -1 : chunk.get_lines().get_line(index - 1);
    const int this_line_num = chunk.get_lines().get_line(index);
    if (this_line_num == last_line_num)
    {
      fmt::print("{:04} {:>4} {}\n", index, '|', inst.to_string(chunk));
    }
    else
    {
      fmt::print("{:04} {:>4} {}\n", index, this_line_num, inst.to_string(chunk));
    }
  }
  void disassemble_chunk(const Chunk& chunk, std::string_view name)
  {
    fmt::print("== {} ==\n", name);
    for (auto [index, inst] : chunk.get_code() | ranges::views::enumerate)
    {
      disassemble_inst(chunk, inst, index);
    }
  }
}