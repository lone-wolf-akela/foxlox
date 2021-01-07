#ifndef FOXLOX_VM_H
#define FOXLOX_VM_H

#include <array>
#include <vector>

#include "config.h"
#include "chunk.h"
#include "value.h"

namespace foxlox
{
  enum class InterpretResult
  {
    OK,
    RUNTIME_ERROR
  };
  
  class VM
  {
  public:
    VM();
    InterpretResult interpret(const Chunk& c);
  private:
    InterpretResult run();
    Inst read_inst();
    void reset_stack();

    Chunk::Code::const_iterator ip;
    const Chunk* chunk;

    using Stack = std::array<Value, STACK_MAX>;
    Stack stack;
    Stack::iterator stack_top;
    Stack::iterator top(int from_top);
    Stack::iterator top();
    void push();
    void pop();
  };
}
#endif 