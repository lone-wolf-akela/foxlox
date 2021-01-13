#include <bit>

#include <fmt/format.h>
#include <gsl/gsl>
#include <range/v3/all.hpp>
#include <magic_enum.hpp>

#include "debug.h"

namespace foxlox
{
  gsl::index disassemble_inst(const Chunk& chunk, const Closure& closure, gsl::index index)
  {
    const int last_line_num = index == 0 ? -1 : closure.get_lines().get_line(index - 1);
    const int this_line_num = closure.get_lines().get_line(index);
    if (this_line_num == last_line_num)
    {
      fmt::print("{:05} {:>4} ", index, '|');
    }
    else
    {
      auto src = chunk.get_source(this_line_num - 1);
      if (src != "")
      {
        fmt::print("{:>5} {:>4} {}\n", "[src]", this_line_num, src);
        fmt::print("{:05} {:>4} ", index, '|');
      }
      else
      {
        fmt::print("{:05} {:>4} ", index, this_line_num);
      }
    }
    const auto codes = closure.get_code();

    const auto get_uint8 = [&] {
      return codes[index + 1];
    };
    const auto get_int16 = [&] {
      const struct { uint8_t a, b; } tmp{ codes[index + 1], codes[index + 2] };
      return std::bit_cast<int16_t>(tmp);
    };
    const auto get_uint16 = [&] {
      const struct { uint8_t a, b; } tmp{ codes[index + 1], codes[index + 2] };
      return std::bit_cast<uint16_t>(tmp);
    };

    OpCode op = static_cast<OpCode>(codes[index]);
    switch (op)
    {
    case OP_NOP:
    case OP_NIL:
    case OP_RETURN:
    case OP_RETURN_V:
    case OP_POP:
    case OP_NEGATE:
    case OP_NOT:
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_INTDIV:
    case OP_EQ:
    case OP_NE:
    case OP_GT:
    case OP_GE:
    case OP_LT:
    case OP_LE:
    {
      fmt::print("{}\n", magic_enum::enum_name(op));
      return 1;
    }
    case OP_CONSTANT:
    {
      const uint16_t constant = get_uint16();
      fmt::print("{:<16} {:>4}, {}\n", "OP_CONSTANT", constant, chunk.get_constants()[constant].to_string());
      return 3;
    }
    case OP_STRING:
    {
      const uint16_t str = get_uint16();
      fmt::print("{:<16} {:>4}, {}\n", "OP_STRING", str, chunk.get_const_strings()[str]->get_view());
      return 3;
    }
    case OP_BOOL:
    {
      bool b = static_cast<bool>(get_uint8());
      fmt::print("{:<16} {:>4}, {}\n", "OP_BOOL", "", b ? "true" : "false");
      return 2;
    }
    case OP_LOAD_STACK:
    case OP_STORE_STACK:
    case OP_LOAD_STATIC:
    case OP_STORE_STATIC:
    case OP_POP_N:
    case OP_TUPLE:
    {
      fmt::print("{:<16} {:>4}\n", magic_enum::enum_name(op), get_uint16());
      return 3;
    }
    case OP_JUMP:
    case OP_JUMP_IF_TRUE:
    case OP_JUMP_IF_FALSE:
    case OP_JUMP_IF_TRUE_NO_POP:
    case OP_JUMP_IF_FALSE_NO_POP:
    {
      fmt::print("{:<16} {:>4}\n", magic_enum::enum_name(op), get_int16());
      return 3;
    }
    default:
      assert(false);
    }
    assert(false);
    return 0;
  }
  void disassemble_chunk(const Chunk& chunk, const Closure& closure, std::string_view name)
  {
    fmt::print("== {} ==\n", name);
    gsl::index i = 0;
    while (i < ssize(closure.get_code()))
    {
      i += disassemble_inst(chunk, closure, i);
    }
  }
}