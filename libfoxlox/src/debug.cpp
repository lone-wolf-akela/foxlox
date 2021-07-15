module foxlox:debug;

import <gsl/gsl>;

import :except;
import :vm;

namespace foxlox
{
  gsl::index Debugger::disassemble_inst([[maybe_unused]] const VM& vm, const Subroutine& subroutine, gsl::index index)
  {
    const int last_line_num = index == 0 ? -1 : subroutine.get_lines().get_line(index - 1);
    const int this_line_num = subroutine.get_lines().get_line(index);
    const auto formated_funcname = std::format("<{}>", subroutine.get_funcname());

    [[maybe_unused]]
    bool print_line_num_before_inst = this_line_num != last_line_num;

#ifdef FOXLOX_DEBUG_TRACE_SRC
    if (this_line_num != last_line_num)
    {
      auto src = vm.current_chunk->get_source(this_line_num);
      if (src != "")
      {
        const auto formatted = std::format("{:>5} {:25} {:>4} {}", "[src]", formated_funcname, this_line_num, src);
        std::cout << std::format(colored ? "\x1b[46m\x1b[30m{:100}\x1b[0m\n" : "{}\n", formatted);
        print_line_num_before_inst = false;
      }
    }
#endif
#ifdef FOXLOX_DEBUG_TRACE_INST
    if (print_line_num_before_inst)
    {
      std::cout << std::format("{:05} {:25} {:>4} ", index, formated_funcname, this_line_num);
    }
    else
    {
      std::cout << std::format("{:05} {:25} {:>4} ", index, formated_funcname, '|');
    }
#endif
    const auto codes = subroutine.get_code();

    [[maybe_unused]]
    const auto get_uint8 = [&]() -> uint8_t {
      return gsl::at(codes, index + 1);
    };
    [[maybe_unused]]
    const auto get_uint16 = [&]() -> uint16_t {
      return (static_cast<uint16_t>(gsl::at(codes, index + 1)) << 8) | gsl::at(codes, index + 2);
    };
    [[maybe_unused]]
    const auto get_int16 = [&]() -> int16_t {
      return std::bit_cast<int16_t>(get_uint16());
    };

    const OP op = static_cast<OP>(gsl::at(codes, index));
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
#ifdef FOXLOX_DEBUG_TRACE_INST
      std::cout << std::format("{}\n", magic_enum::enum_name(op));
#endif
      return 1;
    }
    case OP::SET_PROPERTY:
    case OP::GET_PROPERTY:
    case OP::GET_SUPER_METHOD:
    {
#ifdef FOXLOX_DEBUG_TRACE_INST
      const uint16_t str = get_uint16();
      std::cout << std::format("{:<16} {:>4}, {}\n", magic_enum::enum_name(op), str, vm.const_string_pool.at(vm.current_chunk->get_const_string_idx_base() + str)->get_view());
#endif
      return 3;
    }
    case OP::CONSTANT:
    {
#ifdef FOXLOX_DEBUG_TRACE_INST
      const uint16_t constant = get_uint16();
      std::cout << std::format("{:<16} {:>4}, {}\n", "CONSTANT", constant, vm.current_chunk->get_constant(constant).to_string());
#endif
      return 3;
    }
    case OP::FUNC:
    {
#ifdef FOXLOX_DEBUG_TRACE_INST
      const uint16_t subroutine_idx = get_uint16();
      std::cout << std::format("{:<16} {:>4}, {}\n", "FUNC", subroutine_idx, vm.current_chunk->get_subroutines().at(subroutine_idx).get_funcname());
#endif
      return 3;
    }
    case OP::CLASS:
    {
#ifdef FOXLOX_DEBUG_TRACE_INST
      const uint16_t constant = get_uint16();
      std::cout << std::format("{:<16} {:>4}, {}\n", "CLASS", constant, vm.class_pool.at(constant).get_name());
#endif
      return 3;
    }
    case OP::STRING:
    {
#ifdef FOXLOX_DEBUG_TRACE_INST
      const uint16_t str = get_uint16();
      std::cout << std::format("{:<16} {:>4}, {}\n", "STRING", str, vm.const_string_pool.at(vm.current_chunk->get_const_string_idx_base() + str)->get_view());
#endif
      return 3;
    }
    case OP::BOOL:
    {
#ifdef FOXLOX_DEBUG_TRACE_INST
      const bool b = gsl::narrow_cast<bool>(get_uint8());
      std::cout << std::format("{:<16} {:>4}, {}\n", "BOOL", "", b ? "true" : "false");
#endif
      return 2;
    }
    case OP::CALL:
    case OP::LOAD_STACK:
    case OP::STORE_STACK:
    case OP::LOAD_STATIC:
    case OP::STORE_STATIC:
    case OP::POP_N:
    case OP::TUPLE:
    case OP::IMPORT:
    case OP::UNPACK:
    {
#ifdef FOXLOX_DEBUG_TRACE_INST
      std::cout << std::format("{:<16} {:>4}\n", magic_enum::enum_name(op), get_uint16());
#endif
      return 3;
    }
    case OP::JUMP:
    case OP::JUMP_IF_TRUE:
    case OP::JUMP_IF_FALSE:
    case OP::JUMP_IF_TRUE_NO_POP:
    case OP::JUMP_IF_FALSE_NO_POP:
    {
#ifdef FOXLOX_DEBUG_TRACE_INST
      std::cout << std::format("{:<16} {:>4}\n", magic_enum::enum_name(op), get_int16());
#endif
      return 3;
    }
    default:
      throw FatalError("Unknown OpCode.");
    }
  }
  void Debugger::print_vm_stack(VM& vm)
  {
    std::cout << std::format("{:>36}", '|');
    for (auto v : std::span(vm.stack.begin(), vm.stack_top))
    {
      std::cout << std::format("[{}] ", v.to_string());
    }
    std::cout << "\n";
  }
}
