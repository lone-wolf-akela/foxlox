#include <fmt/format.h>
#include <gsl/gsl>
#include <range/v3/all.hpp>

#include "debug.h"

namespace foxlox
{
  void disassemble_chunk(const Chunk& chunk, std::string_view name)
  {
    fmt::print("== {} ==\n", name);
    int last_line_num = -1;
    for (auto [index, inst] : chunk.get_code() | ranges::views::enumerate)
    {
      int this_line_num = chunk.get_lines().get_line(index);
      if (this_line_num == last_line_num)
      {
        fmt::print("{:04} {:>4} {}\n", index, '|', inst.to_string(chunk));
      }
      else
      {
        fmt::print("{:04} {:>4} {}\n", index, this_line_num, inst.to_string(chunk));
      }
      last_line_num = this_line_num;
    }
  }
}