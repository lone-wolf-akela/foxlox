#include <span>
#include <functional>
#include <utility>
#include <iostream>

#include <fmt/format.h>
#include <magic_enum.hpp>

#include <foxlox/except.h>
#include <foxlox/debug.h>
#include "object.h"

#include <foxlox/vm.h>

namespace foxlox
{
  VM::VM() noexcept
  {
    chunk = nullptr;
    is_moved = false;
    current_heap_size = 0;
    next_gc_heap_size = FIRST_GC_HEAP_SIZE;
    reset_stack();
  }
  VM::~VM()
  {
    clean();
  }
  VM::VM(VM&& r) noexcept
  {
    is_moved = r.is_moved;
    current_subroutine = r.current_subroutine;
    ip = r.ip;
    chunk = r.chunk;
    stack = r.stack;
    stack_top = stack.begin() + std::distance(r.stack.begin(), r.stack_top);
    p_calltrace = calltrace.begin() + std::distance(r.calltrace.begin(), r.p_calltrace);
    string_pool = std::move(r.string_pool);
    tuple_pool = std::move(r.tuple_pool);
    static_value_pool = std::move(r.static_value_pool);
    current_heap_size = r.current_heap_size;
    next_gc_heap_size = r.next_gc_heap_size;
    r.is_moved = true;
  }
  VM& VM::operator=(VM&& r) noexcept
  {
    if (this == &r) { return *this; }
    clean();
    is_moved = r.is_moved;
    current_subroutine = r.current_subroutine;
    ip = r.ip;
    chunk = r.chunk;
    stack = r.stack;
    stack_top = stack.begin() + std::distance(r.stack.begin(), r.stack_top);
    p_calltrace = calltrace.begin() + std::distance(r.calltrace.begin(), r.p_calltrace);
    string_pool = std::move(r.string_pool);
    tuple_pool = std::move(r.tuple_pool);
    static_value_pool = std::move(r.static_value_pool);
    current_heap_size = r.current_heap_size;
    next_gc_heap_size = r.next_gc_heap_size;
    r.is_moved = true;
    return *this;
  }
  void VM::clean()
  {
    if (!is_moved)
    {
      for (const String* p : string_pool)
      {
        String::free(std::bind_front(&VM::deallocator, this), p);
      }
      for (const Tuple* p : tuple_pool)
      {
        Tuple::free(std::bind_front(&VM::deallocator, this), p);
      }
    }
  }
  Value VM::interpret(Chunk& c)
  {
    chunk = &c;
    auto& subroutines = chunk->get_subroutines();
    current_subroutine = &subroutines.at(0);
    ip = current_subroutine->get_code().begin();
    static_value_pool.resize(chunk->get_static_value_num());
    static_value_pool.shrink_to_fit();
    reset_stack();
    return run();
  }
  size_t VM::get_stack_size()
  {
    return std::distance(stack.begin(), stack_top);
  }
  size_t VM::get_stack_capacity() noexcept
  {
    return std::tuple_size_v<decltype(VM::stack)>;
  }
  Value VM::run()
  {
#ifdef DEBUG_TRACE_EXECUTION
    Debugger debugger(true);
#endif
    while (true)
    {
#ifdef DEBUG_TRACE_EXECUTION
      debugger.print_vm_stack(*this);
      debugger.disassemble_inst(*chunk, *current_subroutine, std::distance(current_subroutine->get_code().begin(), ip));
#endif
#ifdef DEBUG_STRESS_GC
      collect_garbage();
#endif
      try
      {
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
          if (p_calltrace == calltrace.begin()) { return Value(); }
          p_calltrace--;
          current_subroutine = p_calltrace->subroutine;
          ip = p_calltrace->ip;
          stack_top = p_calltrace->stack_top;
          // return a nil
          push();
          *top() = Value();

          collect_garbage();
          break;
        }
        case OP_RETURN_V:
        {
          const auto v = *top();

          if (p_calltrace == calltrace.begin()) { return v; }
          p_calltrace--;
          current_subroutine = p_calltrace->subroutine;
          ip = p_calltrace->ip;
          stack_top = p_calltrace->stack_top;
          // return a val
          push();
          *top() = v;

          collect_garbage();
          break;
        }
        case OP_POP:
        {
          pop();
          break;
        }
        case OP_POP_N:
        {
          pop(read_uint16());
          break;
        }
        case OP_NEGATE:
        {
          *top() = -*top();
          break;
        }
        case OP_NOT:
        {
          *top() = !*top();
          break;
        }
        case OP_ADD:
        {
          const auto l = top(1);
          const auto r = top(0);
          if (l->type == Value::STR && r->type == Value::STR)
          {
            *l = Value::strcat(std::bind_front(&VM::allocator, this), *l, *r);
            string_pool.push_back(l->v.str);
          }
          else if (l->type == Value::TUPLE || r->type == Value::TUPLE)
          {
            *l = Value::tuplecat(std::bind_front(&VM::allocator, this), *l, *r);
            tuple_pool.push_back(l->v.tuple);
          }
          else
          {
            *l = *l + *r;
          }
          pop();
          break;
        }
        case OP_SUBTRACT:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l - *r;
          pop();
          break;
        }
        case OP_MULTIPLY:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l * *r;
          pop();
          break;
        }
        case OP_DIVIDE:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l / *r;
          pop();
          break;
        }
        case OP_INTDIV:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = intdiv(*l, *r);
          pop();
          break;
        }
        case OP_EQ:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l == *r;
          pop();
          break;
        }
        case OP_NE:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l != *r;
          pop();
          break;
        }
        case OP_GT:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l > *r;
          pop();
          break;
        }
        case OP_GE:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l >= *r;
          pop();
          break;
        }
        case OP_LT:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l < *r;
          pop();
          break;
        }
        case OP_LE:
        {
          const auto l = top(1);
          const auto r = top(0);
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
          const std::span constants = chunk->get_constants();
          *top() = gsl::at(constants, read_uint16());
          break;
        }
        case OP_FUNC:
        {
          push();
          auto& subroutines = chunk->get_subroutines();
          *top() = &subroutines.at(read_uint16());
          break;
        }
        case OP_STRING:
        {
          push();
          std::span strings = chunk->get_const_strings();
          *top() = gsl::at(strings, read_uint16());
          break;
        }
        case OP_BOOL:
        {
          push();
          *top() = Value(read_bool());
          break;
        }
        case OP_TUPLE:
        {
          // note: n can be 0
          const auto n = read_uint16();
          const auto p = Tuple::alloc(std::bind_front(&VM::allocator, this), n);
          for (gsl::index i = 0; i < n; i++)
          {
            GSL_SUPPRESS(bounds.4) GSL_SUPPRESS(bounds.2)
              p->data()[i] = *top(gsl::narrow_cast<uint16_t>(n - i - 1));
          }
          tuple_pool.push_back(p);
          pop(n);
          push();
          *top() = p;
          break;
        }
        case OP_LOAD_STACK:
        {
          const auto idx = read_uint16();
          const auto v = *top(idx);
          push();
          *top() = v;
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
          *top() = static_value_pool.at(idx);
          break;
        }
        case OP_STORE_STATIC:
        {
          const auto idx = read_uint16();
          const auto r = top();
          static_value_pool.at(idx) = *r;
          break;
        }
        case OP_JUMP:
        {
          const int16_t offset = read_int16();
          ip += offset;
          if (offset < 0)
          {
            collect_garbage();
          }
          break;
        }
        case OP_JUMP_IF_TRUE:
        {
          const int16_t offset = read_int16();
          if (top()->get_bool() == true)
          {
            ip += offset;
          }
          pop();
          if (offset < 0)
          {
            collect_garbage();
          }
          break;
        }
        case OP_JUMP_IF_FALSE:
        {
          const int16_t offset = read_int16();
          if (top()->get_bool() == false)
          {
            ip += offset;
          }
          pop();
          if (offset < 0)
          {
            collect_garbage();
          }
          break;
        }
        case OP_JUMP_IF_TRUE_NO_POP:
        {
          const int16_t offset = read_int16();
          if (top()->get_bool() == true)
          {
            ip += offset;
          }
          break;
        }
        case OP_JUMP_IF_FALSE_NO_POP:
        {
          const int16_t offset = read_int16();
          if (top()->get_bool() == false)
          {
            ip += offset;
          }
          break;
        }
        case OP_CALL:
        {
          const auto v = *top();
          const uint16_t num_of_params = read_uint16();
          pop();
          if (v.type == Value::FUNC)
          {
            const auto func_to_call = v.v.func;
            p_calltrace->subroutine = current_subroutine;
            p_calltrace->ip = ip;
            p_calltrace->stack_top = stack_top - num_of_params;
            p_calltrace++;

            assert(func_to_call->get_arity() == num_of_params);
            current_subroutine = func_to_call;
            ip = current_subroutine->get_code().begin();
          }
          else if (v.type == Value::CPP_FUNC)
          {
            const auto func_to_call = v.v.cppfunc;
            const std::span<Value> params{ next(top(num_of_params)), next(top(0)) };
            const Value result = func_to_call(*this, params);
            pop(num_of_params);
            push();
            *top() = result;
          }
          else
          {
            throw ValueTypeError(fmt::format("Value of type {} is not callable.", 
              magic_enum::enum_name(v.type)).c_str());
          }    
          collect_garbage();
          break;
        }
        default:
          assert(false);
          break;
        }
      }
      catch(const ValueTypeError& e)
      {
        const auto code_idx = std::distance(current_subroutine->get_code().begin(), ip);
        const auto line_num = current_subroutine->get_lines().get_line(code_idx);
        const auto src = chunk->get_source(line_num - 1);
        throw RuntimeError(e.what(), line_num, src);
      }
    }
  }
  OpCode VM::read_inst() noexcept
  {
    return static_cast<OpCode>(read_uint8());
  }
  int16_t VM::read_int16() noexcept
  {
    const struct { uint8_t a, b; } tmp{ read_uint8(), read_uint8() };
    return std::bit_cast<int16_t>(tmp);
  }
  bool VM::read_bool() noexcept
  {
    return gsl::narrow_cast<bool>(read_uint8());
  }
  uint8_t VM::read_uint8() noexcept
  {
    const auto v = *(ip++);
    assert(ip <= current_subroutine->get_code().end());
    return v;
  }
  uint16_t VM::read_uint16() noexcept
  {
    const struct { uint8_t a, b; } tmp{ read_uint8(), read_uint8() };
    return std::bit_cast<uint16_t>(tmp);
  }
  void VM::reset_stack() noexcept
  {
    stack_top = stack.begin();
    p_calltrace = calltrace.begin();
  }
  VM::Stack::iterator VM::top(int from_top) noexcept
  {
    return stack_top - from_top - 1;
  }
  void VM::push() noexcept
  {
    stack_top++;
  }
  void VM::pop(uint16_t n) noexcept
  {
    stack_top -= n;
  }
  char* VM::allocator(size_t l) noexcept
  {
    current_heap_size += l;
    GSL_SUPPRESS(r.11)
#ifdef DEBUG_LOG_GC
      std::cout << fmt::format("alloc size={} ", l);
#endif
    char* const data = new char[l];
#ifdef DEBUG_LOG_GC
      std::cout << fmt::format("at {}; heap size: {} -> {}\n", static_cast<const void*>(data), current_heap_size - l, current_heap_size);
#endif
    return data;
  }
  void VM::deallocator(const char* p, size_t l) noexcept
  {
#ifdef DEBUG_LOG_GC
    std::cout << fmt::format("free size={} at {}; heap size: {} -> {}\n", l, static_cast<const void*>(p), current_heap_size, current_heap_size -l);
#endif
    assert(l <= current_heap_size);
    current_heap_size -= l;
    GSL_SUPPRESS(r.11) GSL_SUPPRESS(i.11)
    delete[] p;
  }
  void VM::collect_garbage()
  {
#ifdef DEBUG_STRESS_GC
    constexpr bool do_gc = true;
#else
    const bool do_gc = current_heap_size > next_gc_heap_size;
#endif
    if (do_gc)
    {
#ifdef DEBUG_LOG_GC
      std::cout << "-- gc begin --\n";
      const size_t heap_size_before = current_heap_size;
#endif
      mark_roots();
      trace_references();
      sweep();
      next_gc_heap_size = std::max<size_t>(current_heap_size * GC_HEAP_GROW_FACTOR, FIRST_GC_HEAP_SIZE);
#ifdef DEBUG_LOG_GC
      std::cout << "-- gc end --\n";
      std::cout << fmt::format("   collected {} bytes (from {} to {}). next at {}.\n",
        heap_size_before - current_heap_size,
        heap_size_before, 
        current_heap_size,
        next_gc_heap_size);
#endif
    }
  }
  void VM::mark_roots()
  {
    // stack
    for (auto& v : std::span(stack.begin(), stack_top))
    {
      mark_value(v);
    }
    // function wait for return
    for (auto& c : std::span(calltrace.begin(), p_calltrace))
    {
      mark_subroutine(*c.subroutine);
    }
    // current function
    mark_subroutine(*current_subroutine);
  }
  void VM::mark_subroutine(Subroutine& s)
  {
    if (s.gc_mark) { return; }
    s.gc_mark = true;
    for (auto idx : s.get_referenced_static_values())
    {
      mark_value(static_value_pool.at(idx));
    }
  }
  void VM::mark_value(Value& v)
  {
#ifdef DEBUG_LOG_GC
    if (v.type == Value::STR)
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.str), v.v.str->get_mark() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.type == Value::TUPLE)
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.tuple), v.v.tuple->get_mark() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.type == Value::FUNC)
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.func), v.v.func->gc_mark ? "is_marked" : "not_marked", v.to_string());
    }
#endif
    switch (v.type)
    {
    case Value::STR:
      v.v.str->mark();
      break;
    case Value::TUPLE:
      if (!v.v.tuple->get_mark())
      {
        gray_stack.push(&v);
        v.v.tuple->mark();
      }
      break;
    case Value::FUNC:
      mark_subroutine(*v.v.func);
      break;
    default:
      /*do nothing*/
      break;
    }
  }
  void VM::trace_references()
  {
    while (!gray_stack.empty())
    {
      Value* const v = gray_stack.top();
      gray_stack.pop();
      if (v->type == Value::TUPLE)
      {
        for (auto& tuple_elem : v->v.tuple->get_span())
        {
          mark_value(tuple_elem);
        }
      }
    }
  }
  void VM::sweep()
  {
    // string_pool
    std::erase_if(string_pool, [this](String* str) {
#ifdef DEBUG_LOG_GC
      std::cout << fmt::format("sweeping {} [{}]: {}\n", static_cast<const void*>(str), str->get_mark() ? "is_marked" : "not_marked", str->get_view());
#endif
      if (!str->get_mark())
      {
        String::free(std::bind_front(&VM::deallocator, this), str);
        return true;
      }
      str->unmark();
      return false;
      });
    // tuple_pool
    std::erase_if(tuple_pool, [this](Tuple* tuple) {
#ifdef DEBUG_LOG_GC
      std::cout << fmt::format("sweeping {} [{}]: {}\n", static_cast<const void*>(tuple), tuple->get_mark() ? "is_marked" : "not_marked", tuple->get_mark() ? Value(tuple).to_string() : "<tuple elem may not avail>");
#endif
      if (!tuple->get_mark())
      {
        Tuple::free(std::bind_front(&VM::deallocator, this), tuple);
        return true;
      }
      tuple->unmark();
      return false;
      });
    // whiten all subroutines
    for (auto& s : chunk->get_subroutines())
    {
      s.gc_mark = false;
    }
  }
}