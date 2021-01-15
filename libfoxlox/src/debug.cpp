#include <bit>
#include <iostream>

#include <fmt/format.h>
#include <gsl/gsl>
#include <range/v3/all.hpp>
#include <magic_enum.hpp>

#include <foxlox/vm.h>
#include <foxlox/debug.h>

namespace foxlox
{
  Debugger::Debugger(bool colored_output)
  {
    colored = colored_output;
  }
  gsl::index Debugger::disassemble_inst(const Chunk& chunk, const Subroutine& subroutine, gsl::index index)
  {
    const int last_line_num = index == 0 ? -1 : subroutine.get_lines().get_line(index - 1);
    const int this_line_num = subroutine.get_lines().get_line(index);
    const auto formated_funcname = fmt::format("<{}>", subroutine.get_funcname());
    if (this_line_num == last_line_num)
    {
      std::cout << fmt::format("{:05} {:15} {:>4} ", index, formated_funcname, '|');
    }
    else
    {
      auto src = chunk.get_source(this_line_num - 1);
      if (src != "")
      {
        const auto formatted = fmt::format("{:>5} {:15} {:>4} {}", "[src]", formated_funcname, this_line_num, src);
        std::cout << fmt::format(colored ? "\x1b[46m\x1b[30m{:80}\x1b[0m\n" : "{}\n", formatted);
        std::cout << fmt::format("{:05} {:15} {:>4} ", index, formated_funcname, '|');
      }
      else
      {
        std::cout << fmt::format("{:05} {:15} {:>4} ", index, formated_funcname, this_line_num);
      }
    }
    const auto codes = subroutine.get_code();

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
      std::cout << fmt::format("{}\n", magic_enum::enum_name(op));
      return 1;
    }
    case OP_CONSTANT:
    {
      const uint16_t constant = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "OP_CONSTANT", constant, chunk.get_constants()[constant].to_string());
      return 3;
    }
    case OP_FUNC:
    {
      const uint16_t constant = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "OP_FUNC", constant, chunk.get_subroutines()[constant].get_funcname());
      return 3;
    }
    case OP_STRING:
    {
      const uint16_t str = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "OP_STRING", str, chunk.get_const_strings()[str]->get_view());
      return 3;
    }
    case OP_BOOL:
    {
      bool b = static_cast<bool>(get_uint8());
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "OP_BOOL", "", b ? "true" : "false");
      return 2;
    }
    case OP_CALL:
    {
      const uint16_t arity = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "OP_CALL", "", arity);
      return 3;
    }
    case OP_LOAD_STACK:
    case OP_STORE_STACK:
    case OP_LOAD_STATIC:
    case OP_STORE_STATIC:
    case OP_POP_N:
    case OP_TUPLE:
    {
      std::cout << fmt::format("{:<16} {:>4}\n", magic_enum::enum_name(op), get_uint16());
      return 3;
    }
    case OP_JUMP:
    case OP_JUMP_IF_TRUE:
    case OP_JUMP_IF_FALSE:
    case OP_JUMP_IF_TRUE_NO_POP:
    case OP_JUMP_IF_FALSE_NO_POP:
    {
      std::cout << fmt::format("{:<16} {:>4}\n", magic_enum::enum_name(op), get_int16());
      return 3;
    }
    default:
      assert(false);
    }
    assert(false);
    return 0;
  }
  void Debugger::disassemble_chunk(const Chunk& chunk, const Subroutine& subroutine, std::string_view name)
  {
    std::cout << fmt::format("== {} ==\n", name);
    gsl::index i = 0;
    while (i < ssize(subroutine.get_code()))
    {
      i += disassemble_inst(chunk, subroutine, i);
    }
  }
  void Debugger::print_vm_stack(VM& vm)
  {
    fmt::print("{:>26}", '|');
    for (auto v : std::span(vm.stack.begin(), vm.stack_top))
    {
      fmt::print("[{}] ", v.to_string());
    }
    fmt::print("\n");
  }
}