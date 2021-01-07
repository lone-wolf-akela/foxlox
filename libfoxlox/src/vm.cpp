#include <span>

#include <fmt/format.h>

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
  InterpretResult VM::interpret(const Chunk& c)
  {
    chunk = &c;
    current_closure = chunk->get_closures().begin();
    ip = current_closure->get_code().begin();
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
  InterpretResult VM::run()
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
      disassemble_inst(*chunk, *current_closure, *ip, std::distance(current_closure->get_code().begin(), ip));
#endif
      Inst inst = read_inst();
      switch (inst.N.op)
      {
      // N
      case OpCode::OP_NOP:
      {
        /* do nothing */
        break;
      }
      case OpCode::OP_RETURN:
      {
        return InterpretResult::OK;
      }
      case OpCode::OP_NEGATE:
      {
        *top() = -*top();
        break;
      }
      case OpCode::OP_ADD:
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
      case OpCode::OP_SUBTRACT:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l - *r;
        pop();
        break;
      }
      case OpCode::OP_MULTIPLY:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l * *r;
        pop();
        break;
      }
      case OpCode::OP_DIVIDE:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l / *r;
        pop();
        break;
      }
      case OpCode::OP_INTDIV:
      {
        auto l = top(1);
        auto r = top(0);
        *l = intdiv(*l, *r);
        pop();
        break;
      }
      case OpCode::OP_EQ:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l == *r;
        pop();
        break;
      }
      case OpCode::OP_NE:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l != *r;
        pop();
        break;
      }
      case OpCode::OP_GT:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l > *r;
        pop();
        break;
      }
      case OpCode::OP_GE:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l >= *r;
        pop();
        break;
      }
      case OpCode::OP_LT:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l < *r;
        pop();
        break;
      }
      case OpCode::OP_LE:
      {
        auto l = top(1);
        auto r = top(0);
        *l = *l <= *r;
        pop();
        break;
      }
      case OpCode::OP_NIL:
      {
        push();
        *top() = Value();
        break;
      }
      // uA
      case OpCode::OP_CONSTANT:
      {
        push();
        *top() = chunk->get_constants()[inst.uA.ua];
        break;
      }
      case OpCode::OP_STRING:
      {
        push();
        *top() = chunk->get_const_strings()[inst.uA.ua];
        break;
      }
      case OpCode::OP_BOOL:
      {
        push();
        *top() = Value(bool(inst.uA.ua));
        break;
      }
      // iA
      case OpCode::OP_INT:
      {
        push();
        *top() = Value(int64_t(inst.iA.ia));
        break;
      }
      default:
        assert(false);
        break;
      }
    }
  }
  Inst VM::read_inst()
  {
    const auto inst = *(ip++);
    assert(ip <= current_closure->get_code().end());
    return inst;
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