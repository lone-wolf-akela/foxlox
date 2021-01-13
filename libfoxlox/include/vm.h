#pragma once

#include <array>
#include <vector>

#include <chunk.h>
#include "../src/config.h"
#include "../src/value.h"

namespace foxlox
{
  class VM
  {
  public:
    VM();
    ~VM();
    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
    VM(VM&& r) noexcept;
    VM& operator=(VM&& r) noexcept;

    Value interpret(Chunk& c);

    // stack ops
    using Stack = std::array<Value, STACK_MAX>;
    size_t get_stack_size();
    static size_t get_stack_capacity();
    Stack::iterator top(int from_top = 0);
    void push();
    void pop();
    void pop(uint16_t n);
  private:
    Value run();
    OpCode read_inst();
    int16_t read_int16();
    bool read_bool();
    uint8_t read_uint8();
    uint16_t read_uint16();
    void reset_stack();

    void clean();

    std::vector<Closure>::iterator current_closure;
    std::vector<uint8_t>::const_iterator ip;
    Chunk* chunk;
    
    bool is_moved;

    Stack stack;
    Stack::iterator stack_top;

    // data pool
    std::vector<String*> string_pool;
    std::vector<Tuple*> tuple_pool;
    std::vector<Value> static_value_pool;
  };
}
