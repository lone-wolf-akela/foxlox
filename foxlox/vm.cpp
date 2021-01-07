#include <span>

#include "debug.h"
#include "vm.h"

namespace foxlox
{
  VM::VM()
  {
    chunk = nullptr;
    reset_stack();
  }
  InterpretResult VM::interpret(const Chunk& c)
  {
    chunk = &c;
    ip = c.get_code().begin();
    return run();
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
      disassemble_inst(*chunk, *ip, std::distance(chunk->get_code().begin(), ip));
#endif
      Inst inst = read_inst();
      switch (inst.N.op)
      {
        // N
      case OpCode::OP_RETURN:
      {
        fmt::print("{}\n", pop().to_string());
        return InterpretResult::OK;
      }
      case OpCode::OP_NEGATE:
      {
        push(pop().neg());
        break;
      }
      case OpCode::OP_ADD:
      {
        Value v = pop();
        push(pop().add(v));
        break;
      }
      case OpCode::OP_SUBTRACT:
      {
        Value v = pop();
        push(pop().sub(v));
        break;
      }
      case OpCode::OP_MULTIPLY:
      {
        Value v = pop();
        push(pop().mul(v));
        break;
      }
      case OpCode::OP_DIVIDE:
      {
        Value v = pop();
        push(pop().div(v));
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
        *top() = chunk->get_strings()[inst.uA.ua];
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
    assert(ip <= chunk->get_code().end());
    return inst;
  }
  void VM::reset_stack()
  {
    stack_top = stack.begin();
  }
  VM::Stack::iterator VM::top(int from_top)
  {
    return stack_top - from_top;
  }
  VM::Stack::iterator VM::top()
  {
    return stack_top;
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