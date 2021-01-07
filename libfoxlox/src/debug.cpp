#include <fmt/format.h>
#include <gsl/gsl>
#include <range/v3/all.hpp>

#include "debug.h"

namespace foxlox
{
  void disassemble_inst(const Chunk& chunk, const Closure& closure, Inst inst, gsl::index index)
  {
    const int last_line_num = index == 0 ? -1 : closure.get_lines().get_line(index - 1);
    const int this_line_num = closure.get_lines().get_line(index);
    if (this_line_num == last_line_num)
    {
      fmt::print("{:04} {:>4} {}\n", index, '|', inst.to_string(chunk));
    }
    else
    {
      fmt::print("{:04} {:>4} {}\n", index, this_line_num, inst.to_string(chunk));
    }
  }
  void disassemble_chunk(const Chunk& chunk, const Closure& closure, std::string_view name)
  {
    fmt::print("== {} ==\n", name);
    for (auto [index, inst] : closure.get_code() | ranges::views::enumerate)
    {
      disassemble_inst(chunk, closure, inst, index);
    }
  }
}