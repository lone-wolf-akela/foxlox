#include <span>

#include <fmt/format.h>

#include <foxexcept.h>
#include "debug.h"
#include "vm.h"

namespace foxlox
{
  VM::VM()
  {
    chunk = nullptr;
    reset_stack();
  }
  VM::~VM()
  {
    for (String* p : string_pool)
    {
      String::free(p);
    }
  }
  Value VM::interpret(Chunk& c)
  {
    chunk = &c;
    current_closure = chunk->get_closures().begin();
    ip = current_closure->get_code().begin();
    static_value_pool.resize(chunk->get_static_value_num());
    static_value_pool.shrink_to_fit();
    return run();
  }
  size_t VM::get_stack_size()
  {
    return std::distance(stack.begin(), stack_top);
  }
  size_t VM::get_stack_capacity()
  {
    return std::tuple_size_v<decltype(VM::stack)>;
  }
  Value VM::run()
  {
    while (true)
    {
#ifdef DEBUG_TRACE_EXECUTION
      fmt::print("{:10}", ' ');
      for (auto v : std::span(stack.begin(), stack_top))
      {
        fmt::print("[ {} ]", v.to_string());
      }
      fmt::print("\n");
      disassemble_inst(*chunk, *current_closure, std::distance(current_closure->get_code().begin(), ip));
#endif
      const OpCode inst = read_inst();
      switch (inst)
      {
      // N
      case OP_NOP:
      {
        /* do nothing */
        break;
      }
      case OP_RETURN:
      {
        return Value();
      }
      case OP_RETURN_V:
      {
        const auto v = *top();
        pop();
        return v;
      }
      case OP_NEGATE:
      {
        *top() = -*top();
        break;
      }
      case OP_ADD:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l + *r;
        if (l->type == Value::STR)
        {
          string_pool.push_back(l->v.str);
        }
        pop();
        break;
      }
      case OP_SUBTRACT:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l - *r;
        pop();
        break;
      }
      case OP_MULTIPLY:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l * *r;
        pop();
        break;
      }
      case OP_DIVIDE:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l / *r;
        pop();
        break;
      }
      case OP_INTDIV:
      {
        auto l = top(1);
        auto r = top(0);
        *l = intdiv(*l, *r);
        pop();
        break;
      }
      case OP_EQ:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l == *r;
        pop();
        break;
      }
      case OP_NE:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l != *r;
        pop();
        break;
      }
      case OP_GT:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l > *r;
        pop();
        break;
      }
      case OP_GE:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l >= *r;
        pop();
        break;
      }
      case OP_LT:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l < *r;
        pop();
        break;
      }
      case OP_LE:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l <= *r;
        pop();
        break;
      }
      case OP_NIL:
      {
        push();
        *top() = Value();
        break;
      }
      case OP_CONSTANT:
      {
        push();
        *top() = chunk->get_constants()[read_uint16()];
        break;
      }
      case OP_STRING:
      {
        push();
        *top() = chunk->get_const_strings()[read_uint16()];
        break;
      }
      case OP_BOOL:
      {
        push();
        *top() = Value(read_bool());
        break;
      }
      case OP_LOAD_STACK:
      {
        const auto idx = read_uint16();
        push();
        *top() = *top(idx);
        break;
      }
      case OP_STORE_STACK:
      {
        const auto idx = read_uint16();
        const auto r = top();
        *top(idx) = *r;
        break;
      }
      case OP_LOAD_STATIC:
      {
        const auto idx = read_uint16();
        push();
        *top() = static_value_pool[idx];
        break;
      }
      case OP_STORE_STATIC:
      {
        const auto idx = read_uint16();
        const auto r = top();
        static_value_pool[idx] = *r;
        break;
      }
      default:
        assert(false);
        break;
      }
    }
  }
  OpCode VM::read_inst()
  {
    return static_cast<OpCode>(read_uint8());
  }
  int16_t VM::read_int16()
  {
    const struct { uint8_t a, b; } tmp{ read_uint8(), read_uint8() };
    return std::bit_cast<int16_t>(tmp);
  }
  bool VM::read_bool()
  {
    return static_cast<bool>(read_uint8());
  }
  uint8_t VM::read_uint8()
  {
    const auto v = *(ip++);
    assert(ip <= current_closure->get_code().end());
    return v;
  }
  uint16_t VM::read_uint16()
  {
    const struct { uint8_t a, b; } tmp{ read_uint8(), read_uint8() };
    return std::bit_cast<uint16_t>(tmp);
  }
  void VM::reset_stack()
  {
    stack_top = stack.begin();
  }
  VM::Stack::iterator VM::top(int from_top)
  {
    return stack_top - from_top - 1;
  }
  void VM::push()
  {
    stack_top++;
  }
  void VM::pop()
  {
    stack_top--;
  }
}