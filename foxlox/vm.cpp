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
      case OpCode::OP_RETURN:
        fmt::print("{}\n", pop().to_string());
        return InterpretResult::OK;
      case OpCode::OP_NEGATE:
        push(pop().neg());
        break;
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
        Value v = pop();
        push(pop().intdiv(v));
        break;
      }
      case OpCode::OP_CONSTANT:
        push(chunk->get_constants()[inst.A.a]);
        break;
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
  void VM::push(Value value)
  {
    *stack_top = value;
    stack_top++;
  }
  Value VM::pop()
  {
    stack_top--;
    return *stack_top;
  }
}