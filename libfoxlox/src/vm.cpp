#include <cassert>
#include <span>
#include <functional>
#include <utility>
#include <iostream>

#include <fmt/format.h>
#include <magic_enum.hpp>

#include <foxlox/except.h>
#include <foxlox/debug.h>
#include "object.h"
#include "config.h"

#include <foxlox/vm.h>

namespace foxlox
{
  VM_Allocator::VM_Allocator(size_t* heap_sz) noexcept :
    heap_size(heap_sz)
  {
  }
  char* VM_Allocator::operator()(size_t l)
  {
    *heap_size += l;
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
  VM_Deallocator::VM_Deallocator(size_t* heap_sz) noexcept :
    heap_size(heap_sz)
  {
  }
  void VM_Deallocator::operator()(const char* p, size_t l)
  {
#ifdef DEBUG_LOG_GC
    std::cout << fmt::format("free size={} at {}; heap size: {} -> {}\n", l, static_cast<const void*>(p), current_heap_size, current_heap_size - l);
#endif
    Ensures(l <= *heap_size);
    *heap_size -= l;
    GSL_SUPPRESS(r.11) GSL_SUPPRESS(i.11)
      delete[] p;
  }
  VM_GC_Index::VM_GC_Index(VM* v) noexcept :
    vm(v) 
  {
  }
  void VM_GC_Index::clean()
  {
    for (const Tuple* p : tuple_pool)
    {
      Tuple::free(vm->deallocator, p);
    }
    for (const Instance* p : instance_pool)
    {
      Instance::free(vm->deallocator, p);
    }
  }
  VM_GC_Index::~VM_GC_Index()
  {
    clean();
  }
  VM_GC_Index::VM_GC_Index(VM_GC_Index&& o) :
    tuple_pool(std::move(o.tuple_pool)),
    instance_pool(std::move(o.instance_pool))
  {
    // replace the moved vector to new empty ones
    // this prevents the moved VM_GC_Index's destructor do anything
    o.tuple_pool = std::vector<Tuple*>{};
    o.instance_pool = std::vector<Instance*>{};
  }
  VM_GC_Index& VM_GC_Index::operator=(VM_GC_Index&& o)
  {
    if (this == &o) { return *this; }
    clean();
    tuple_pool = std::move(o.tuple_pool);
    instance_pool = std::move(o.instance_pool);
    // replace the moved vector to new empty ones
    // this prevents the moved VM_GC_Index's destructor do anything
    o.tuple_pool = std::vector<Tuple*>{};
    o.instance_pool = std::vector<Instance*>{};
    return *this;
  }

  VM::VM() noexcept :
    stack(STACK_MAX),
    calltrace(CALLTRACE_MAX),
    allocator(&current_heap_size),
    deallocator(&current_heap_size),
    gc_index(this),
    string_pool(allocator, deallocator)
  {
    chunk = nullptr;
    current_heap_size = 0;
    next_gc_heap_size = FIRST_GC_HEAP_SIZE;
  }

  Value VM::interpret(Chunk& c)
  {
    chunk = &c;
    auto& subroutines = chunk->get_subroutines();
    current_subroutine = &subroutines.at(0);
    ip = current_subroutine->get_code().begin();
    static_value_pool.resize(chunk->get_static_value_num());
    static_value_pool.shrink_to_fit();

    const_string_pool.clear();
    for (auto& str : c.get_const_strings())
    {
      const_string_pool.push_back(string_pool.add_string(str));
    }
    str__init__ = string_pool.add_string("__init__");
    const_string_pool.push_back(str__init__);

    class_pool.clear();
    for (auto& compiletime_class : c.get_classes())
    {
      class_pool.emplace_back(compiletime_class.get_name());
      for (auto [name_idx, subroutine_idx] : compiletime_class.get_methods())
      {
        class_pool.back().add_method(
          const_string_pool.at(name_idx), &chunk->get_subroutines().at(subroutine_idx));
      }
    }

    stack_top = stack.begin();
    p_calltrace = calltrace.begin();
    return run();
  }
  size_t VM::get_stack_size()
  {
    return std::distance(stack.begin(), stack_top);
  }
  size_t VM::get_stack_capacity() noexcept
  {
    return stack.size();
  }
  Value VM::run()
  {
#if defined(DEBUG_TRACE_STACK) || defined(DEBUG_TRACE_INST) || defined(DEBUG_TRACE_SRC)
    Debugger debugger(true);
#endif
    while (true)
    {
#ifdef DEBUG_TRACE_STACK
      debugger.print_vm_stack(*this);
#endif
#ifdef DEBUG_STRESS_GC
      collect_garbage();
#endif
#if defined(DEBUG_TRACE_INST) || defined(DEBUG_TRACE_SRC)
      debugger.disassemble_inst(*this, *current_subroutine, std::distance(current_subroutine->get_code().begin(), ip));
#endif
      try
      {
        const OP inst = read_inst();
        switch (inst)
        {
          // N
        case OP::NOP:
        {
          /* do nothing */
          break;
        }
        case OP::RETURN:
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
        case OP::RETURN_V:
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
        case OP::POP:
        {
          pop();
          break;
        }
        case OP::POP_N:
        {
          pop(read_uint16());
          break;
        }
        case OP::NEGATE:
        {
          *top() = -*top();
          break;
        }
        case OP::NOT:
        {
          *top() = !*top();
          break;
        }
        case OP::ADD:
        {
          const auto l = top(1);
          const auto r = top(0);
          if (l->is_str() && r->is_str())
          {
            *l = string_pool.add_str_cat(l->get_strview(), r->get_strview());
          }
          else if (l->is_tuple() || r->is_tuple())
          {
            *l = Value::tuplecat(allocator, *l, *r);
            gc_index.tuple_pool.push_back(l->v.tuple);
          }
          else
          {
            *l = *l + *r;
          }
          pop();
          break;
        }
        case OP::SUBTRACT:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l - *r;
          pop();
          break;
        }
        case OP::MULTIPLY:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l * *r;
          pop();
          break;
        }
        case OP::DIVIDE:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l / *r;
          pop();
          break;
        }
        case OP::INTDIV:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = intdiv(*l, *r);
          pop();
          break;
        }
        case OP::EQ:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l == *r;
          pop();
          break;
        }
        case OP::NE:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l != *r;
          pop();
          break;
        }
        case OP::GT:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l > *r;
          pop();
          break;
        }
        case OP::GE:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l >= *r;
          pop();
          break;
        }
        case OP::LT:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l < *r;
          pop();
          break;
        }
        case OP::LE:
        {
          const auto l = top(1);
          const auto r = top(0);
          *l = *l <= *r;
          pop();
          break;
        }
        case OP::NIL:
        {
          push();
          *top() = Value();
          break;
        }
        case OP::CONSTANT:
        {
          push();
          const std::span constants = chunk->get_constants();
          *top() = gsl::at(constants, read_uint16());
          break;
        }
        case OP::FUNC:
        {
          push();
          auto& subroutines = chunk->get_subroutines();
          *top() = &subroutines.at(read_uint16());
          break;
        }
        case OP::CLASS:
        {
          push();
          *top() = &class_pool.at(read_uint16());
          break;
        }
        case OP::INHERIT:
        {
          const auto derived = top(1);
          const auto base = top(0);
          if (!derived->is_class() || !base->is_class())
          {
            throw ValueError("Value is not a class.");
          }
          derived->v.klass->set_super(base->v.klass);
          pop();
          break;
        }
        case OP::STRING:
        {
          push();
          *top() = const_string_pool.at(read_uint16());
          break;
        }
        case OP::BOOL:
        {
          push();
          *top() = Value(read_bool());
          break;
        }
        case OP::TUPLE:
        {
          // note: n can be 0
          const auto n = read_uint16();
          const auto p = Tuple::alloc(allocator, n);
          for (gsl::index i = 0; i < n; i++)
          {
            GSL_SUPPRESS(bounds.4) GSL_SUPPRESS(bounds.2)
              p->data()[i] = *top(gsl::narrow_cast<uint16_t>(n - i - 1));
          }
          gc_index.tuple_pool.push_back(p);
          pop(n);
          push();
          *top() = p;
          break;
        }
        case OP::LOAD_STACK:
        {
          const auto idx = read_uint16();
          const auto v = *top(idx);
          push();
          *top() = v;
          break;
        }
        case OP::STORE_STACK:
        {
          const auto idx = read_uint16();
          const auto r = top();
          *top(idx) = *r;
          break;
        }
        case OP::LOAD_STATIC:
        {
          const auto idx = read_uint16();
          push();
          *top() = static_value_pool.at(idx);
          break;
        }
        case OP::STORE_STATIC:
        {
          const auto idx = read_uint16();
          const auto r = top();
          static_value_pool.at(idx) = *r;
          break;
        }
        case OP::JUMP:
        {
          const int16_t offset = read_int16();
          ip += offset;
          if (offset < 0)
          {
            collect_garbage();
          }
          break;
        }
        case OP::JUMP_IF_TRUE:
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
        case OP::JUMP_IF_FALSE:
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
        case OP::JUMP_IF_TRUE_NO_POP:
        {
          const int16_t offset = read_int16();
          if (top()->get_bool() == true)
          {
            ip += offset;
          }
          break;
        }
        case OP::JUMP_IF_FALSE_NO_POP:
        {
          const int16_t offset = read_int16();
          if (top()->get_bool() == false)
          {
            ip += offset;
          }
          break;
        }
        case OP::CALL:
        {
          const auto v = *top();
          const uint16_t num_of_params = read_uint16();
          pop();
          switch (v.type)
          {
          case ValueType::FUNC:
          {
            const auto func_to_call = v.v.func;
            p_calltrace->subroutine = current_subroutine;
            p_calltrace->ip = ip;
            p_calltrace->stack_top = stack_top - num_of_params;
            p_calltrace++;

            if (func_to_call->get_arity() != num_of_params)
            {
              throw InternalRuntimeError(fmt::format("Wrong number of function parameters. Expect: {}, got: {}.", func_to_call->get_arity(), num_of_params));
            }
            current_subroutine = func_to_call;
            ip = current_subroutine->get_code().begin();
            break;
          }
          case ValueType::CPP_FUNC:
          {
            const auto func_to_call = v.v.cppfunc;
            const std::span<Value> params{ next(top(num_of_params)), next(top(0)) };
            const Value result = func_to_call(*this, params);
            pop(num_of_params);
            push();
            *top() = result;
            break;
          }
          case ValueType::METHOD:
          {
            const auto func_to_call = v.get_method_func();
            p_calltrace->subroutine = current_subroutine;
            p_calltrace->ip = ip;
            p_calltrace->stack_top = stack_top - num_of_params;
            p_calltrace++;
            
            push();
            *top() = v.v.instance; // `this'

            if (func_to_call->get_arity() != num_of_params)
            {
              throw InternalRuntimeError(fmt::format("Wrong number of function parameters. Expect: {}, got: {}.", func_to_call->get_arity(), num_of_params));
            }
            current_subroutine = func_to_call;
            ip = current_subroutine->get_code().begin();
            break;
          }
          case ValueType::OBJ:
          {
            if (v.is_nil())
            {
              throw ValueError("Value of type NIL is not callable.");
            }
            if (!v.is_class())
            {
              throw ValueError(fmt::format("Value of type {} is not callable.",
                magic_enum::enum_name(v.v.obj->type)));
            }
            const auto klass = v.v.klass;
            const auto instance = Instance::alloc(allocator, deallocator, klass);
            gc_index.instance_pool.push_back(instance);
            if (auto [got, func_to_call] = klass->try_get_method_idx(str__init__); got)
            {
              p_calltrace->subroutine = current_subroutine;
              p_calltrace->ip = ip;
              p_calltrace->stack_top = stack_top - num_of_params;
              p_calltrace++;

              push();
              *top() = instance; // `this'

              if (func_to_call->get_arity() != num_of_params)
              {
                throw InternalRuntimeError(fmt::format("Wrong number of function parameters. Expect: {}, got: {}.", func_to_call->get_arity(), num_of_params));
              }
              current_subroutine = func_to_call;
              ip = current_subroutine->get_code().begin();
            }
            else
            {
              if (num_of_params != 0)
              {
                throw InternalRuntimeError(fmt::format("Wrong number of function parameters. Expect: {}, got: {}.", 0, num_of_params));
              }
              push();
              *top() = instance;
            }
            break;
          }
          default:
          {
            throw ValueError(fmt::format("Value of type {} is not callable.",
              magic_enum::enum_name(v.type)));
          }
          }
          collect_garbage();
          break;
        }
        case OP::GET_SUPER_METHOD:
        {
          const auto name = const_string_pool.at(read_uint16());
          auto instance = top()->get_instance();
          *top() = instance->get_super_method(name);
          break;
        }
        case OP::GET_PROPERTY:
        {
          const auto name = const_string_pool.at(read_uint16());
          auto instance = top()->get_instance();
          *top() = instance->get_property(name);
          break;
        }
        case OP::SET_PROPERTY:
        {
          const auto name = const_string_pool.at(read_uint16());
          auto instance = top()->get_instance();
          pop();
          instance->set_property(name, *top());
          break;
        }
        default:
          throw InternalRuntimeError(fmt::format("Unknown instruction: {}.", magic_enum::enum_name(inst)));
        }
      }
      catch(const std::exception& e)
      {
        const auto code_idx = std::distance(current_subroutine->get_code().begin(), ip);
        const auto line_num = current_subroutine->get_lines().get_line(code_idx);
        const auto src = chunk->get_source(line_num);
        throw RuntimeError(e.what(), line_num, src);
      }
    }
  }
  OP VM::read_inst() noexcept
  {
    return static_cast<OP>(read_uint8());
  }
  int16_t VM::read_int16() noexcept
  {
    return static_cast<int16_t>(read_uint16());
  }
  bool VM::read_bool() noexcept
  {
    return gsl::narrow_cast<bool>(read_uint8());
  }
  uint8_t VM::read_uint8() noexcept
  {
    const auto v = *(ip++);
    // this check is too time consuming so we use a assert here
    // which means we will not do this check in a release build
    assert(ip <= current_subroutine->get_code().end());
    return v;
  }
  uint16_t VM::read_uint16() noexcept
  {
    uint16_t u = static_cast<uint16_t>(read_uint8()) << 8;
    u |= read_uint8();
    return u;
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
    // const strings
    for (auto str : const_string_pool)
    {
      str->mark();
    }
  }
  void VM::mark_subroutine(Subroutine& s)
  {
    if (s.is_marked()) { return; }
    s.mark();
    for (auto idx : s.get_referenced_static_values())
    {
      mark_value(static_value_pool.at(idx));
    }
  }
  void VM::mark_class(Class& c)
  {
    if (c.is_marked()) { return; }
    for (auto& [name, func] : c.get_all_methods())
    {
      std::ignore = name;
      mark_subroutine(*func);
    }
    if (c.get_super() != nullptr)
    {
      mark_class(*c.get_super());
    }
  }
  void VM::mark_value(Value& v)
  {
#ifdef DEBUG_LOG_GC
    if (v.is_str())
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.str), v.v.str->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.is_tuple())
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.tuple), v.v.tuple->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.is_class())
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.klass), v.v.klass->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.is_instance())
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.instance), v.v.instance->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.type == ValueType::FUNC)
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.func), v.v.func->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.type == ValueType::METHOD)
    {
      std::cout << fmt::format("marking {} [{}]: {}\n", static_cast<const void*>(v.get_method_func()), v.get_method_func()->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
#endif
    if (v.is_str())
    {
      v.v.str->mark();
    }
    else if (v.is_tuple())
    {
      if (!v.v.tuple->is_marked())
      {
        gray_stack.push(&v);
        v.v.tuple->mark();
      }
    }
    else if (v.is_instance())
    {
      if (!v.v.instance->is_marked())
      {
        gray_stack.push(&v);
        v.v.instance->mark();
      }
    }
    else if (v.is_class())
    {
      mark_class(*v.v.klass);
    }
    else if (v.type == ValueType::FUNC)
    {
      mark_subroutine(*v.v.func);
    }
    else if (v.type == ValueType::METHOD)
    {
      if (!v.v.instance->is_marked())
      {
        gray_stack.push(&v);
        v.v.instance->mark();
      }
      mark_subroutine(*v.get_method_func());
    }
  }
  void VM::trace_references()
  {
    while (!gray_stack.empty())
    {
      Value* const v = gray_stack.top();
      gray_stack.pop();
      // only tuple, instance, and method should be put into graystack
      if (v->is_tuple())
      {
        for (auto& tuple_elem : v->v.tuple->get_span())
        {
          mark_value(tuple_elem);
        }
      }
      else // if (v->is_instance() || v->type == ValueType::METHOD)
      {
        for (auto& [name, value] : v->v.instance->get_all_fields())
        {
          std::ignore = name;
          mark_value(value);
        }
        mark_class(*v->v.instance->get_class());
      }
    }
  }
  void VM::sweep()
  {
    // string_pool
    string_pool.sweep();
    // tuple_pool
    std::erase_if(gc_index.tuple_pool, [this](Tuple* tuple) {
#ifdef DEBUG_LOG_GC
      std::cout << fmt::format("sweeping {} [{}]: {}\n", static_cast<const void*>(tuple), tuple->is_marked() ? "is_marked" : "not_marked", tuple->is_marked() ? Value(tuple).to_string() : "<tuple elem may not avail>");
#endif
      if (!tuple->is_marked())
      {
        Tuple::free(deallocator, tuple);
        return true;
      }
      tuple->unmark();
      return false;
      });
    // instance_pool
    std::erase_if(gc_index.instance_pool, [this](Instance* instance) {
#ifdef DEBUG_LOG_GC
      std::cout << fmt::format("sweeping {} [{}]: {}\n", static_cast<const void*>(instance), instance->is_marked() ? "is_marked" : "not_marked", Value(instance).to_string());
#endif
      if (!instance->is_marked())
      {
        Instance::free(deallocator, instance);
        return true;
      }
      instance->unmark();
      return false;
      });
    // whiten all subroutines
    for (auto& s : chunk->get_subroutines())
    {
      s.unmark();
    }
    // whiten all classes
    for (auto& c : class_pool)
    {
      c.unmark();
    }
  }
}
