module;
#include <fmt/format.h>
export module foxlox:vm;

import <array>;
import <vector>;
import <string_view>;
import <span>;
import <unordered_map>;
import <filesystem>;
import <iostream>;
import <deque>;

import "opcode.h";
import :runtimelib;
import :value;
import :chunk;

namespace foxlox
{
  class VM;
  class Debugger;

  class VM_Allocator
  {
  public:
      VM_Allocator(size_t* heap_sz) noexcept;
      VM_Allocator(const VM_Allocator&) noexcept = default;
      VM_Allocator(VM_Allocator&&) noexcept = default;
      VM_Allocator& operator=(const VM_Allocator&) noexcept = default;
      VM_Allocator& operator=(VM_Allocator&&) noexcept = default;
      char* operator()(size_t l) noexcept;
  private:
      size_t* heap_size;
  };

  class VM_Deallocator
  {
  public:
      VM_Deallocator(size_t* heap_sz) noexcept;
      VM_Deallocator(const VM_Deallocator&) noexcept = default;
      VM_Deallocator(VM_Deallocator&&) noexcept = default;
      VM_Deallocator& operator=(const VM_Deallocator&) noexcept = default;
      VM_Deallocator& operator=(VM_Deallocator&&) noexcept = default;
      void operator()(char* const p, size_t l) noexcept;
  private:
      size_t* heap_size;
  };

  class VM_GC_Index
  {
  public:
      // these pools serve as the index of sweep() in VM
      // and need special move func / dtor
      std::vector<Tuple*> tuple_pool;
      std::vector<Instance*> instance_pool;
      std::vector<Dict*> dict_pool;

      VM_GC_Index(VM* v) noexcept;
      ~VM_GC_Index();
      VM_GC_Index(const VM_GC_Index&) = delete;
      VM_GC_Index& operator=(const VM_GC_Index&) = delete;
      VM_GC_Index(VM_GC_Index&& o) noexcept;
      VM_GC_Index& operator=(VM_GC_Index&& o) noexcept;
  private:
      void clean();
      VM* vm;
  };

  export class VM
  {
  public:
      VM(bool load_default_lib = true) noexcept;
      VM(const VM&) = delete;
      VM& operator=(const VM&) = delete;
      VM(VM&& r) noexcept = default;
      VM& operator=(VM&& r) noexcept = default;

      void load_lib(std::string_view path, const RuntimeLib& lib);
      void load_binary(const std::vector<char>& binary);
      Value run();
      Value run(const std::vector<char>& binary);

      // stack ops
      using Stack = std::vector<Value>;
      size_t get_stack_size();
      size_t get_stack_capacity() noexcept;
      Stack::iterator top() noexcept;
      Stack::iterator top(int from_top) noexcept;
      void push() noexcept;
      void pop() noexcept;
      void pop(uint16_t n) noexcept;
  private:

      OP read_inst() noexcept;
      int16_t read_int16() noexcept;
      bool read_bool() noexcept;
      uint8_t read_uint8() noexcept;
      uint16_t read_uint16() noexcept;

      Subroutine* current_subroutine;
      Chunk* current_chunk;
      using IP = std::span<const uint8_t>::iterator;
      IP ip;
      std::vector<Chunk> chunks;

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

      // mem manage related
      size_t current_heap_size; // heap size should be initialized before allocator
      size_t next_gc_heap_size;
      VM_Allocator allocator;
      VM_Deallocator deallocator;
      void collect_garbage();
      void mark_roots();
      void mark_value(Value& v);
      void mark_class(Class& c);
      void mark_dict(Dict& d);
      void mark_subroutine(Subroutine& s);
      std::vector<Value*> gray_stack;
      void trace_references();
      void sweep();

      std::unordered_map<std::string, RuntimeLib> runtime_libs;
      std::filesystem::path findlib(std::span<const std::string_view> libpath);
      Dict* import_lib(std::span<const std::string_view> libpath);
      Dict* gen_export_dict();
      void jump_to_func(Subroutine* func) noexcept;
      void pop_calltrace() noexcept;
      void push_calltrace(uint16_t num_of_params) noexcept;

      // data pool
      VM_GC_Index gc_index;
      StringPool string_pool;

      // note: we don't need sweep static_value_pool during gc
      std::vector<Value> static_value_pool;
      // this pool is generated during chunk loading
      // use deque instead of vector here, as there're values the hold pointer to class
      // so it shouldn't be invalid after push_back
      std::deque<Class> class_pool;
      // this pool is generated during chunk loading; do not gc this
      // also need mark all of elem in it during gc marking
      std::vector<String*> const_string_pool;
      // special strings
      String* str__init__;


      friend class VM_GC_Index;
      friend class Debugger;
  };

  export class Debugger
  {
  public:
      Debugger(bool colored_output) noexcept :
          colored(colored_output)
      {
      }
      gsl::index disassemble_inst(const VM& vm, const Subroutine& subroutine, gsl::index index);
      void disassemble_chunk(const VM& vm, const Subroutine& subroutine, std::string_view name)
      {
          std::cout << fmt::format("== {} ==\n", name);
          gsl::index i = 0;
          while (i < ssize(subroutine.get_code()))
          {
              i += disassemble_inst(vm, subroutine, i);
          }
      }
      void print_vm_stack(VM& vm);
  private:
      [[maybe_unused]] bool colored;
  };
}
