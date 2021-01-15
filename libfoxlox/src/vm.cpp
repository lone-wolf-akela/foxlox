#include <span>
#include <functional>
#include <utility>

#include <fmt/format.h>

#include <foxlox/except.h>
#include <foxlox/debug.h>
#include <foxlox/vm.h>

namespace foxlox
{
  VM::VM() noexcept
  {
    chunk = nullptr;
    is_moved = false;
    current_heap_size = 0;
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
  Value VM::interpret(const Chunk& c)
  {
    chunk = &c;
    const std::span subroutines = chunk->get_subroutines();
    current_subroutine = &gsl::at(subroutines, 0);
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
          return v;
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
          const std::span subroutines = chunk->get_subroutines();
          *top() = &gsl::at(subroutines, read_uint16());
          break;
        }
        case OP_STRING:
        {
          push();
          const std::span strings = chunk->get_const_strings();
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
              p->elems[i] = *top(gsl::narrow_cast<uint16_t>(n - i - 1));
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
          const auto func_to_call = top()->get_func();
          pop();
          const uint16_t num_of_params = read_uint16();
          p_calltrace->subroutine = current_subroutine;
          p_calltrace->ip = ip;
          p_calltrace->stack_top = stack_top - num_of_params;
          p_calltrace++;

          assert(func_to_call->get_arity() == num_of_params);
          current_subroutine = func_to_call;
          ip = current_subroutine->get_code().begin();
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
  char* VM::allocator(size_t l)
  {
    current_heap_size += l;
    GSL_SUPPRESS(r.11)
    return new char[l];
  }
  void VM::deallocator(const char* p, size_t l)
  {
    assert(l <= current_heap_size);
    current_heap_size -= l;
    GSL_SUPPRESS(r.11) GSL_SUPPRESS(i.11)
    delete[] p;
  }
}