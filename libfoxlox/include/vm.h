#ifndef FOXLOX_VM_H
#define FOXLOX_VM_H

#include <array>
#include <vector>

#include <chunk.h>
#include "../src/config.h"
#include "../src/value.h"

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
    ~VM();

    InterpretResult interpret(const Chunk& c);

    // stack ops
    using Stack = std::array<Value, STACK_MAX>;
    size_t get_stack_size();
    static size_t get_stack_capacity();
    Stack::iterator top(int from_top = 0);
    void push();
    void pop();
  private:
    InterpretResult run();
    Inst read_inst();
    void reset_stack();

    std::vector<Closure>::const_iterator current_closure;
    std::vector<Inst>::const_iterator ip;
    const Chunk* chunk;
    
    Stack stack;
    Stack::iterator stack_top;

    // data pool
    std::vector<String*> string_pool;
  };
}
#endif 