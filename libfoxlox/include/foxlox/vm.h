#pragma once

#include <array>
#include <vector>
#include <stack>

#include <foxlox/chunk.h>
#include <foxlox/debug.h>
#include "../../src/config.h"
#include "../../src/value.h"

namespace foxlox
{
  class VM
  {
  public:
    VM() noexcept;
    ~VM();
    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
    VM(VM&& r) noexcept;
    VM& operator=(VM&& r) noexcept;

    Value interpret(Chunk& c);

    // stack ops
    using Stack = std::array<Value, STACK_MAX>;
    size_t get_stack_size();
    static size_t get_stack_capacity() noexcept;
    Stack::iterator top(int from_top = 0) noexcept;
    void push() noexcept;
    void pop(uint16_t n = 1) noexcept;
  private:
    Value run();
    OpCode read_inst() noexcept;
    int16_t read_int16() noexcept;
    bool read_bool() noexcept;
    uint8_t read_uint8() noexcept;
    uint16_t read_uint16() noexcept;
    void reset_stack() noexcept;

    void clean();

    Subroutine* current_subroutine;
    using IP = std::span<const uint8_t>::iterator;
    IP ip;
    Chunk* chunk;
    
    bool is_moved;

    Stack stack;
    Stack::iterator stack_top;

    struct CallFrame
    {
      Subroutine* subroutine{};
      IP ip{};
      Stack::iterator stack_top{};
    };
    using CallTrace = std::array<CallFrame, CALLTRACE_MAX>;
    CallTrace calltrace;
    CallTrace::iterator p_calltrace;

    // data pool
    std::vector<String*> string_pool;
    std::vector<Tuple*> tuple_pool;
    std::vector<Value> static_value_pool;

    // mem manage related
    char* allocator(size_t l) noexcept;
    void deallocator(const char* p, size_t l) noexcept;
    size_t current_heap_size;
    size_t next_gc_heap_size;
    void collect_garbage();
    void mark_roots();
    void mark_value(Value& v);
    void mark_subroutine(Subroutine& s);
    std::stack<Value*> gray_stack;
    void trace_references();
    void sweep();

    friend class Debugger;
  };
}
