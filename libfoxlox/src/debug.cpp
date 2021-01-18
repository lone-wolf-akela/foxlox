#include <bit>
#include <iostream>

#include <fmt/format.h>
#include <gsl/gsl>
#include <range/v3/all.hpp>
#include <magic_enum.hpp>

#include <foxlox/except.h>
#include <foxlox/vm.h>
#include "object.h"
#include "config.h"

#include <foxlox/debug.h>

namespace foxlox
{
  Debugger::Debugger(bool colored_output)
  {
    colored = colored_output;
  }
  gsl::index Debugger::disassemble_inst([[maybe_unused]] const VM& vm, const Subroutine& subroutine, gsl::index index)
  {
    const int last_line_num = index == 0 ? -1 : subroutine.get_lines().get_line(index - 1);
    const int this_line_num = subroutine.get_lines().get_line(index);
    const auto formated_funcname = fmt::format("<{}>", subroutine.get_funcname());

    bool print_line_num_before_inst = this_line_num != last_line_num;

#ifdef DEBUG_TRACE_SRC
    if (this_line_num != last_line_num)
    {
      auto src = vm.chunk->get_source(this_line_num);
      if (src != "")
      {
        const auto formatted = fmt::format("{:>5} {:15} {:>4} {}", "[src]", formated_funcname, this_line_num, src);
        std::cout << fmt::format(colored ? "\x1b[46m\x1b[30m{:80}\x1b[0m\n" : "{}\n", formatted);
        print_line_num_before_inst = false;
      }
    }
#endif
#ifdef DEBUG_TRACE_INST
    if (print_line_num_before_inst)
    {
      std::cout << fmt::format("{:05} {:15} {:>4} ", index, formated_funcname, this_line_num);
    }
    else
    {
      std::cout << fmt::format("{:05} {:15} {:>4} ", index, formated_funcname, '|');
    }
#endif
    const auto codes = subroutine.get_code();

    const auto get_uint8 = [&]() -> uint8_t {
      return codes[index + 1];
    };
    const auto get_uint16 = [&]() -> uint16_t {
      return (static_cast<uint16_t>(codes[index + 1]) << 8) | codes[index + 2];
    };
    const auto get_int16 = [&]() -> int16_t {
      const struct { uint8_t a, b; } tmp{ codes[index + 1], codes[index + 2] };
      return std::bit_cast<int16_t>(get_uint16());
    };

    OP op = static_cast<OP>(codes[index]);
    switch (op)
    {
    case OP::NOP:
    case OP::NIL:
    case OP::RETURN:
    case OP::RETURN_V:
    case OP::POP:
    case OP::NEGATE:
    case OP::NOT:
    case OP::ADD:
    case OP::SUBTRACT:
    case OP::MULTIPLY:
    case OP::DIVIDE:
    case OP::INTDIV:
    case OP::EQ:
    case OP::NE:
    case OP::GT:
    case OP::GE:
    case OP::LT:
    case OP::LE:
    case OP::INHERIT:
    {
#ifdef DEBUG_TRACE_INST
      std::cout << fmt::format("{}\n", magic_enum::enum_name(op));
#endif
      return 1;
    }
    case OP::SET_PROPERTY:
    case OP::GET_PROPERTY:
    case OP::GET_SUPER_METHOD:
    {
#ifdef DEBUG_TRACE_INST
      const uint16_t str = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", magic_enum::enum_name(op), str, vm.const_string_pool.at(str)->get_view());
#endif
      return 2;
    }
    case OP::CONSTANT:
    {
#ifdef DEBUG_TRACE_INST
      const uint16_t constant = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "CONSTANT", constant, vm.chunk->get_constants()[constant].to_string());
#endif
      return 3;
    }
    case OP::FUNC:
    {
#ifdef DEBUG_TRACE_INST
      const uint16_t constant = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "FUNC", constant, vm.chunk->get_subroutines()[constant].get_funcname());
#endif
      return 3;
    }
    case OP::CLASS:
    {
#ifdef DEBUG_TRACE_INST
      const uint16_t constant = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "CLASS", constant, vm.class_pool.at(constant).get_name());
#endif
      return 3;
    }
    case OP::STRING:
    {
#ifdef DEBUG_TRACE_INST
      const uint16_t str = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "STRING", str, vm.const_string_pool.at(str)->get_view());
#endif
      return 3;
    }
    case OP::BOOL:
    {
#ifdef DEBUG_TRACE_INST
      bool b = static_cast<bool>(get_uint8());
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "BOOL", "", b ? "true" : "false");
#endif
      return 2;
    }
    case OP::CALL:
    {
#ifdef DEBUG_TRACE_INST
      const uint16_t arity = get_uint16();
      std::cout << fmt::format("{:<16} {:>4}, {}\n", "CALL", "", arity);
#endif
      return 3;
    }
    case OP::LOAD_STACK:
    case OP::STORE_STACK:
    case OP::LOAD_STATIC:
    case OP::STORE_STATIC:
    case OP::POP_N:
    case OP::TUPLE:
    {
#ifdef DEBUG_TRACE_INST
      std::cout << fmt::format("{:<16} {:>4}\n", magic_enum::enum_name(op), get_uint16());
#endif
      return 3;
    }
    case OP::JUMP:
    case OP::JUMP_IF_TRUE:
    case OP::JUMP_IF_FALSE:
    case OP::JUMP_IF_TRUE_NO_POP:
    case OP::JUMP_IF_FALSE_NO_POP:
    {
#ifdef DEBUG_TRACE_INST
      std::cout << fmt::format("{:<16} {:>4}\n", magic_enum::enum_name(op), get_int16());
#endif
      return 3;
    }
    default:
      throw FatalError("Unknown OpCode.");
    }
  }
  void Debugger::disassemble_chunk(const VM& vm, const Subroutine& subroutine, std::string_view name)
  {
    std::cout << fmt::format("== {} ==\n", name);
    gsl::index i = 0;
    while (i < ssize(subroutine.get_code()))
    {
      i += disassemble_inst(vm, subroutine, i);
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