#pragma once

#include <array>
#include <vector>
#include <stack>

#include <foxlox/chunk.h>
#include <foxlox/debug.h>
#include "../../src/value.h"
#include "../../src/string_pool.h"

namespace foxlox
{
  class VM;
  class VM_GC_Index
  {
  public:
    // these pools serve as the index of sweep() in VM
    // and need special move func / dtor
    std::vector<Tuple*> tuple_pool;
    std::vector<Instance*> instance_pool;

    VM_GC_Index(VM* v);
    ~VM_GC_Index();
    VM_GC_Index(const VM_GC_Index&) = delete;
    VM_GC_Index& operator=(const VM_GC_Index&) = delete;
    VM_GC_Index(VM_GC_Index&& o);
    VM_GC_Index& operator=(VM_GC_Index&& o);
  private:
    void clean();
    VM* vm;
  };

  class VM
  {
  public:
    VM() noexcept;
    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
    VM(VM&& r) noexcept = default;
    VM& operator=(VM&& r) noexcept = default;

    Value interpret(Chunk& c);

    // stack ops
    using Stack = std::vector<Value>;
    size_t get_stack_size();
    size_t get_stack_capacity() noexcept;
    Stack::iterator top(int from_top = 0) noexcept;
    void push() noexcept;
    void pop(uint16_t n = 1) noexcept;
  private:
    Value run();
    OP read_inst() noexcept;
    int16_t read_int16() noexcept;
    bool read_bool() noexcept;
    uint8_t read_uint8() noexcept;
    uint16_t read_uint16() noexcept;

    Subroutine* current_subroutine;
    using IP = std::span<const uint8_t>::iterator;
    IP ip;
    Chunk* chunk;

    Stack stack;
    Stack::iterator stack_top;

    struct CallFrame
    {
      Subroutine* subroutine{};
      IP ip{};
      Stack::iterator stack_top{};
    };
    using CallTrace = std::vector<CallFrame>;
    CallTrace calltrace;
    CallTrace::iterator p_calltrace;

    // data pool
    VM_GC_Index gc_index;
    StringPool string_pool;
    
    // note: we don't need sweep static_value_pool during gc
    std::vector<Value> static_value_pool;
    // this pool is generated during chunk loading
    std::vector<Class> class_pool;
    // this pool is generated during chunk loading; do not gc this
    // also need mark all of elem in it during gc marking
    std::vector<String*> const_string_pool;
    // special strings
    String* str__init__;

    // mem manage related
    char* allocator(size_t l);
    void deallocator(const char* p, size_t l);
    size_t current_heap_size;
    size_t next_gc_heap_size;
    void collect_garbage();
    void mark_roots();
    void mark_value(Value& v);
    void mark_class(Class& c);
    void mark_subroutine(Subroutine& s);
    std::stack<Value*> gray_stack;
    void trace_references();
    void sweep();

    friend class VM_GC_Index;
    friend class Debugger;
  };
}
