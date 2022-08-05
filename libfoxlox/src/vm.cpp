module;
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/join.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
module foxlox:vm;
import :vm;

import <cassert>;
import <span>;
import <functional>;
import <utility>;
import <iostream>;
import <sstream>;
import <algorithm>;
import <format>;

import <magic_enum.hpp>;

import "opcode.h";
import :config;
import :mem_alloc;
import :except;
import :value;
import :compiler;

namespace foxlox
{
  VM_Allocator::VM_Allocator(size_t* heap_sz) noexcept :
    heap_size(heap_sz)
  {
  }
  GSL_SUPPRESS(f.6)
    char* VM_Allocator::operator()(size_t l) noexcept
  {
    *heap_size += l;
#ifdef FOXLOX_DEBUG_LOG_GC
    std::cout << std::format("alloc size={} ", l);
#endif
    char* const data = static_cast<char*>(MALLOC(l));
#ifdef FOXLOX_DEBUG_LOG_GC
    std::cout << std::format("at {}; heap size: {} -> {}\n", static_cast<const void*>(data), *heap_size - l, *heap_size);
#endif
    Ensures(data != nullptr);
    return data;
  }
  VM_Deallocator::VM_Deallocator(size_t* heap_sz) noexcept :
    heap_size(heap_sz)
  {
  }
  GSL_SUPPRESS(f.6)
    void VM_Deallocator::operator()(char* const p, size_t l) noexcept
  {
#ifdef FOXLOX_DEBUG_LOG_GC
    std::cout << std::format("free size={} at {}; heap size: {} -> {}\n", l, static_cast<const void*>(p), *heap_size, *heap_size - l);
#endif
    Expects(l <= *heap_size);
    *heap_size -= l;
    FREE(p);
  }
  VM_GC_Index::VM_GC_Index(VM* v) noexcept :
    vm(v)
  {
  }
  void VM_GC_Index::clean()
  {
    for (auto p : tuple_pool)
    {
      Tuple::free(vm->deallocator, p);
    }
    for (auto p : instance_pool)
    {
      Instance::free(vm->deallocator, p);
    }
    for (auto p : dict_pool)
    {
      Dict::free(vm->deallocator, p);
    }
  }
  VM_GC_Index::~VM_GC_Index()
  {
    try
    {
      clean();
    }
    catch (...)
    {
      std::terminate();
    }
  }
  VM_GC_Index::VM_GC_Index(VM_GC_Index&& o) noexcept :
    tuple_pool(std::move(o.tuple_pool)),
    instance_pool(std::move(o.instance_pool)),
    dict_pool(std::move(o.dict_pool)),
    vm(o.vm)
  {
    // replace the moved vector to new empty ones
    // this prevents the moved VM_GC_Index's destructor do anything
    o.tuple_pool = std::vector<Tuple*>{};
    o.instance_pool = std::vector<Instance*>{};
    o.dict_pool = std::vector<Dict*>{};
  }
  VM_GC_Index& VM_GC_Index::operator=(VM_GC_Index&& o) noexcept
  {
    try
    {
      if (this == &o) { return *this; }
      clean();
      tuple_pool = std::move(o.tuple_pool);
      instance_pool = std::move(o.instance_pool);
      dict_pool = std::move(o.dict_pool);
      vm = o.vm;
      // replace the moved vector to new empty ones
      // this prevents the moved VM_GC_Index's destructor do anything
      o.tuple_pool = std::vector<Tuple*>{};
      o.instance_pool = std::vector<Instance*>{};
      o.dict_pool = std::vector<Dict*>{};
      return *this;
    }
    catch (...)
    {
      std::terminate();
    }
  }
  GSL_SUPPRESS(f.6)
    VM::VM(bool load_default_lib) noexcept :
    current_subroutine(nullptr),
    current_super_level(0),
    current_chunk(nullptr),
    stack(STACK_MAX),
    calltrace(CALLTRACE_MAX),
    current_heap_size(0),
    next_gc_heap_size(FIRST_GC_HEAP_SIZE),
    allocator(&current_heap_size),
    deallocator(&current_heap_size),
    gc_index(this),
    string_pool(allocator, deallocator)
  {
    try
    {
      str__init__ = string_pool.add_string("__init__");
      const_string_pool.push_back(str__init__);
      if (load_default_lib)
      {
        for (const auto& [path, lib] : default_libs())
        {
          load_lib(path, lib);
        }
      }
    }
    catch (...)
    {
      std::terminate();
    }
  }
  void VM::load_lib(std::string_view path, const RuntimeLib& lib)
  {
    runtime_libs.emplace(path, lib);
  }
  void VM::load_binary(const std::vector<char>& binary)
  {
    if (
      ssize(binary) < ssize(BINARY_HEADER) ||
      !std::equal(begin(BINARY_HEADER), end(BINARY_HEADER), begin(binary))
      )
    {
      throw VMError("Wrong binary format.");
    }
    std::string tmp_str(begin(binary) + ssize(BINARY_HEADER), end(binary));
    std::istringstream strm;
    strm.str(std::move(tmp_str));
    chunks.push_back(Chunk::load(strm));

    chunks.back().set_static_value_idx_base(static_value_pool.size());
    static_value_pool.resize(static_value_pool.size() + chunks.back().get_static_value_num());

    chunks.back().set_const_string_idx_base(const_string_pool.size());
    for (auto& str : chunks.back().get_const_strings())
    {
      const_string_pool.push_back(string_pool.add_string(str));
    }

    chunks.back().set_class_idx_base(class_pool.size());
    for (auto& compiletime_class : chunks.back().get_classes())
    {
      class_pool.emplace_back(compiletime_class.get_name());
      for (const auto& [name_idx, subroutine_idx] : compiletime_class.get_methods())
      {
        class_pool.back().add_method(
          const_string_pool.at(chunks.back().get_const_string_idx_base() + name_idx),
          &chunks.back().get_subroutines().at(subroutine_idx)
        );
      }
    }
  }
  Value VM::run(const std::vector<char>& binary)
  {
    if (!chunks.empty())
    {
      throw VMError("The VM has already been loaded with some other binary.");
    }
    load_binary(binary);
    stack_top = stack.begin();
    p_calltrace = calltrace.begin();
    jump_to_func(&chunks.front().get_subroutines().front());
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

  GSL_SUPPRESS(es.76) GSL_SUPPRESS(gsl.util)
    Value VM::run()
  {
#if defined(FOXLOX_DEBUG_TRACE_STACK) || defined(FOXLOX_DEBUG_TRACE_INST) || defined(FOXLOX_DEBUG_TRACE_SRC)
    Debugger debugger(true);
#endif
#ifdef FOXLOX_DEBUG_TRACE_STACK
#define DBG_PRINT_STACK debugger.print_vm_stack(*this)
#else
#define DBG_PRINT_STACK
#endif
#ifdef FOXLOX_DEBUG_STRESS_GC
#define DBG_GC collect_garbage()
#else
#define DBG_GC
#endif
#if defined(FOXLOX_DEBUG_TRACE_INST) || defined(FOXLOX_DEBUG_TRACE_SRC)
#define DBG_PRINT_INST debugger.disassemble_inst(*this, *current_subroutine, std::distance(current_subroutine->get_code().begin(), ip))
#else
#define DBG_PRINT_INST
#endif

    try
    {
      // switched goto from https://bullno1.com/blog/switched-goto
#define DISPATCH() \
      DBG_PRINT_STACK; \
      DBG_GC; \
      DBG_PRINT_INST; \
      switch(read_inst()) \
      { \
        OPCODE(DISPATCH_CASE) \
        default: UNREACHABLE(); \
      }
#define LBL(op) lbl_##op
#define DISPATCH_CASE(op) case OP::op: goto LBL(op);

      DISPATCH();
      // N
      LBL(NOP) :
      {
        /* do nothing */
        DISPATCH();
      }
      LBL(RETURN) :
      {
        if (current_subroutine == &current_chunk->get_subroutines().front())
        {
          collect_garbage();
          return Value();
        }
        pop_calltrace();
        // return a nil
        push();
        *top() = Value();
        collect_garbage();
        DISPATCH();
      }
      LBL(RETURN_V) :
      {
        const auto v = *top();
        if (current_subroutine == &current_chunk->get_subroutines().front())
        {
          collect_garbage();
          return v;
        }

        pop_calltrace();
        // return a val
        push();
        *top() = v;

        collect_garbage();
        DISPATCH();
      }
      LBL(POP) :
      {
        pop();
        DISPATCH();
      }
      LBL(POP_N) :
      {
        pop(read_uint16());
        DISPATCH();
      }
      LBL(NEGATE) :
      {
        *top() = -*top();
        DISPATCH();
      }
      LBL(NOT) :
      {
        *top() = !*top();
        DISPATCH();
      }
      LBL(ADD) :
      {
        const auto l = top(1);
        const auto r = top(0);
        if (l->is_str() && r->is_str())
        {
          *l = string_pool.add_str_cat(l->get_strview(), r->get_strview());
        }
        else if (l->is_tuple() || r->is_tuple())
        {
          *l = Tuple::tuplecat(allocator, *l, *r);
          gc_index.tuple_pool.push_back(l->v.tuple);
        }
        else
        {
          *l = *l + *r;
        }
        pop();
        DISPATCH();
      }
      LBL(SUBTRACT) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l - *r;
        pop();
        DISPATCH();
      }
      LBL(MULTIPLY) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l * *r;
        pop();
        DISPATCH();
      }
      LBL(DIVIDE) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l / *r;
        pop();
        DISPATCH();
      }
      LBL(INTDIV) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = intdiv(*l, *r);
        pop();
        DISPATCH();
      }
      LBL(EQ) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l == *r;
        pop();
        DISPATCH();
      }
      LBL(NE) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l != *r;
        pop();
        DISPATCH();
      }
      LBL(GT) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l > *r;
        pop();
        DISPATCH();
      }
      LBL(GE) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l >= *r;
        pop();
        DISPATCH();
      }
      LBL(LT) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l < *r;
        pop();
        DISPATCH();
      }
      LBL(LE) :
      {
        const auto l = top(1);
        const auto r = top(0);
        *l = *l <= *r;
        pop();
        DISPATCH();
      }
      LBL(NIL) :
      {
        push();
        *top() = Value();
        DISPATCH();
      }
      LBL(CONSTANT) :
      {
        push();
        *top() = current_chunk->get_constant(read_uint16());
        DISPATCH();
      }
      LBL(FUNC) :
      {
        push();
        auto& subroutines = current_chunk->get_subroutines();
        *top() = &subroutines.at(read_uint16());
        DISPATCH();
      }
      LBL(CLASS) :
      {
        push();
        *top() = &class_pool.at(current_chunk->get_class_idx_base() + read_uint16());
        DISPATCH();
      }
      LBL(INHERIT) :
      {
        const auto derived = top(1);
        const auto base = top(0);
        if (!derived->is_class() || !base->is_class())
        {
          throw ValueError("Value is not a class.");
        }
        derived->v.klass->set_super(base->v.klass);
        pop();
        DISPATCH();
      }
      LBL(STRING) :
      {
        push();
        *top() = const_string_pool.at(current_chunk->get_const_string_idx_base() + read_uint16());
        DISPATCH();
      }
      LBL(BOOL) :
      {
        push();
        *top() = Value(read_bool());
        DISPATCH();
      }
      LBL(TUPLE) :
      {
        // note: n can be 0
        const auto n = read_uint16();
        const auto p = Tuple::alloc(allocator, n);
        for (gsl::index i = 0; i < n; i++)
        {
          //TODO: deduce this
          GSL_SUPPRESS(bounds.4) GSL_SUPPRESS(bounds.2) GSL_SUPPRESS(bounds.1)
            p->data<Tuple>()[i] = *top(gsl::narrow_cast<uint16_t>(n - i - 1));
        }
        gc_index.tuple_pool.push_back(p);
        pop(n);
        push();
        *top() = p;
        DISPATCH();
      }
      LBL(LOAD_STACK) :
      {
        const auto idx = read_uint16();
        const auto v = *top(idx);
        push();
        *top() = v;
        DISPATCH();
      }
      LBL(STORE_STACK) :
      {
        const auto idx = read_uint16();
        const auto r = top();
        *top(idx) = *r;
        DISPATCH();
      }
      LBL(LOAD_STATIC) :
      {
        const auto idx = read_uint16();
        push();
        *top() = static_value_pool.at(current_chunk->get_static_value_idx_base() + idx);
        DISPATCH();
      }
      LBL(STORE_STATIC) :
      {
        const auto idx = read_uint16();
        const auto r = top();
        static_value_pool.at(current_chunk->get_static_value_idx_base() + idx) = *r;
        DISPATCH();
      }
      LBL(JUMP) :
      {
        const int16_t offset = read_int16();
        ip += offset;
        if (offset < 0)
        {
          collect_garbage();
        }
        DISPATCH();
      }
      LBL(JUMP_IF_TRUE) :
      {
        const int16_t offset = read_int16();
        if (top()->is_truthy())
        {
          ip += offset;
        }
        pop();
        if (offset < 0)
        {
          collect_garbage();
        }
        DISPATCH();
      }
      LBL(JUMP_IF_FALSE) :
      {
        const int16_t offset = read_int16();
        if (!top()->is_truthy())
        {
          ip += offset;
        }
        pop();
        if (offset < 0)
        {
          collect_garbage();
        }
        DISPATCH();
      }
      LBL(JUMP_IF_TRUE_NO_POP) :
      {
        const int16_t offset = read_int16();
        if (top()->is_truthy())
        {
          ip += offset;
        }
        DISPATCH();
      }
      LBL(JUMP_IF_FALSE_NO_POP) :
      {
        const int16_t offset = read_int16();
        if (!top()->is_truthy())
        {
          ip += offset;
        }
        DISPATCH();
      }
      LBL(CALL) :
      {
        const auto v = *top();
        const uint16_t num_of_params = read_uint16();
        pop();
        switch (v.type)
        {
        case ValueType::FUNC:
        {
          const auto func_to_call = v.v.func;
          push_calltrace(num_of_params);

          if (func_to_call->get_arity() != num_of_params)
          {
            throw InternalRuntimeError(std::format("Wrong number of function parameters. Expect: {}, got: {}.", func_to_call->get_arity(), num_of_params));
          }
          jump_to_func(func_to_call);
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
          push_calltrace(num_of_params);
          current_super_level = v.method_super_level();
          const auto func_to_call = v.method_func();

          push();
          *top() = v.method_instance(); // `this'

          if (func_to_call->get_arity() != num_of_params)
          {
            throw InternalRuntimeError(std::format("Wrong number of function parameters. Expect: {}, got: {}.", func_to_call->get_arity(), num_of_params));
          }
          jump_to_func(func_to_call);
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
            throw ValueError(std::format("Value of type {} is not callable.",
              magic_enum::enum_name(v.v.obj->type)));
          }
          const auto klass = v.v.klass;
          const auto instance = Instance::alloc(allocator, deallocator, klass);
          gc_index.instance_pool.push_back(instance);
          if (auto method = klass->get_method(str__init__); method.has_value())
          {
            push_calltrace(num_of_params);

            push();
            *top() = instance; // `this'

            if (method->func->get_arity() != num_of_params)
            {
              throw InternalRuntimeError(std::format("Wrong number of function parameters. Expect: {}, got: {}.", method->func->get_arity(), num_of_params));
            }
            jump_to_func(method->func);
          }
          else
          {
            if (num_of_params != 0)
            {
              throw InternalRuntimeError(std::format("Wrong number of function parameters. Expect: {}, got: {}.", 0, num_of_params));
            }
            push();
            *top() = instance;
          }
          break;
        }
        default:
        {
          throw ValueError(std::format("Value of type {} is not callable.",
            magic_enum::enum_name(v.type)));
        }
        }
        DISPATCH();
      }
      LBL(GET_SUPER_METHOD) :
      {
        const auto name = const_string_pool.at(current_chunk->get_const_string_idx_base() + read_uint16());
        auto instance = top()->get_instance();
        *top() = instance->get_super_method(current_super_level, name);
        DISPATCH();
      }
      LBL(GET_PROPERTY) :
      {
        const auto name = const_string_pool.at(current_chunk->get_const_string_idx_base() + read_uint16());
        *top() = top()->get_property(name);
        DISPATCH();
      }
      LBL(SET_PROPERTY) :
      {
        const auto name = const_string_pool.at(current_chunk->get_const_string_idx_base() + read_uint16());
        auto instance = top()->get_instance();
        pop();
        instance->set_property(name, *top());
        DISPATCH();
      }
      LBL(IMPORT) :
      {
        const uint16_t path_len = read_uint16();
        Ensures(path_len >= 1);
        const auto libpath = std::span(top(path_len - 1), next(top()))
          | ranges::views::transform([](auto v) { return v.get_strview(); })
          | ranges::to<std::vector<std::string_view>>;
        pop(path_len - 1);
        *top() = import_lib(libpath);
        DISPATCH();
      }
      LBL(UNPACK) :
      {
        const uint16_t tuple_size = read_uint16();
        std::span<Value> values = top()->get_tuplespan();
        if (values.size() != tuple_size)
        {
          throw InternalRuntimeError(
            std::format("Tuple size mismatch. Expect: {}, got: {}.", tuple_size, values.size())
          );
        }
        for (auto& e : values)
        {
          push();
          *top() = e;
        }
        DISPATCH();
      }
    }
    catch (const std::exception& e)
    {
      const auto code_idx = std::distance(current_subroutine->get_code().begin(), ip);
      const auto line_num = current_subroutine->get_lines().get_line(code_idx);
      const auto src = current_chunk->get_source(line_num);
      throw RuntimeError(e.what(), line_num, src);
    }
  }
  OP VM::read_inst() noexcept
  {
    return static_cast<OP>(read_uint8());
  }
  int16_t VM::read_int16() noexcept
  {
    return std::bit_cast<int16_t>(read_uint16());
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
  VM::Stack::iterator VM::top() noexcept
  {
    return stack_top - 1;
  }
  VM::Stack::iterator VM::top(int from_top) noexcept
  {
    return stack_top - from_top - 1;
  }
  void VM::push() noexcept
  {
    stack_top++;
  }
  void VM::pop() noexcept
  {
    stack_top -= 1;
  }
  void VM::pop(uint16_t n) noexcept
  {
    stack_top -= n;
  }
  void VM::collect_garbage()
  {
#ifdef FOXLOX_DEBUG_STRESS_GC
    constexpr bool do_gc = true;
#else
    const bool do_gc = current_heap_size > next_gc_heap_size;
#endif
    if (do_gc)
    {
#ifdef FOXLOX_DEBUG_LOG_GC
      std::cout << "-- gc begin --\n";
      const size_t heap_size_before = current_heap_size;
#endif
      mark_roots();
      trace_references();
      sweep();
      next_gc_heap_size = std::max<size_t>(current_heap_size * GC_HEAP_GROW_FACTOR, FIRST_GC_HEAP_SIZE);
#ifdef FOXLOX_DEBUG_LOG_GC
      std::cout << "-- gc end --\n";
      std::cout << std::format("   collected {} bytes (from {} to {}). next at {}.\n",
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
    for (const gsl::not_null str : const_string_pool)
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
      mark_value(static_value_pool.at(s.get_chunk()->get_static_value_idx_base() + idx));
    }
  }
  void VM::mark_class(Class& c)
  {
    if (c.is_marked()) { return; }
    for (auto& entry : c.get_hash_table())
    {
      mark_subroutine(*entry.value.func);
    }
    if (c.get_super() != nullptr)
    {
      mark_class(*c.get_super());
    }
  }
  void VM::mark_value(Value& v)
  {
#ifdef FOXLOX_DEBUG_LOG_GC
    if (v.is_str())
    {
      std::cout << std::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.str), v.v.str->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.is_tuple())
    {
      std::cout << std::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.tuple), v.v.tuple->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.is_class())
    {
      std::cout << std::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.klass), v.v.klass->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.is_instance())
    {
      std::cout << std::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.instance), v.v.instance->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.is_dict())
    {
      std::cout << std::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.dict), v.v.dict->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.type == ValueType::FUNC)
    {
      std::cout << std::format("marking {} [{}]: {}\n", static_cast<const void*>(v.v.func), v.v.func->is_marked() ? "is_marked" : "not_marked", v.to_string());
    }
    if (v.type == ValueType::METHOD)
    {
      std::cout << std::format("marking {} [{}]: {}\n", static_cast<const void*>(v.method_func()), v.method_func()->is_marked() ? "is_marked" : "not_marked", v.to_string());
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
        gray_stack.push_back(&v);
        v.v.tuple->mark();
      }
    }
    else if (v.is_instance())
    {
      if (!v.v.instance->is_marked())
      {
        gray_stack.push_back(&v);
        v.v.instance->mark();
      }
    }
    else if (v.is_class())
    {
      mark_class(*v.v.klass);
    }
    else if (v.is_dict())
    {
      if (!v.v.dict->is_marked())
      {
        gray_stack.push_back(&v);
        v.v.dict->mark();
      }
    }
    else if (v.is_array())
    {
      throw UnimplementedError("");
    }
    else if (v.type == ValueType::FUNC)
    {
      mark_subroutine(*v.v.func);
    }
    else if (v.type == ValueType::METHOD)
    {
      if (!v.method_instance()->is_marked())
      {
        gray_stack.push_back(&v);
        v.method_instance()->mark();
      }
      mark_subroutine(*v.method_func());
    }
  }
  void VM::trace_references()
  {
    while (!gray_stack.empty())
    {
      const gsl::not_null v = gray_stack.back();
      gray_stack.pop_back();
      // only tuple, instance, and method should be put into graystack
      if (v->is_tuple())
      {
        for (auto& tuple_elem : v->v.tuple->get_span())
        {
          mark_value(tuple_elem);
        }
      }
      else if (v->is_dict())
      {
        for (auto& entry : v->v.dict->get_hash_table())
        {
          mark_value(entry.key);
          mark_value(entry.value);
        }
      }
      else if (v->is_instance())
      {
        for (auto& entry : v->v.instance->get_hash_table())
        {
          mark_value(entry.value);
        }
        mark_class(*v->v.instance->get_class());
      }
      else // if (v->type == ValueType::METHOD)
      {
        Instance* instance = v->method_instance();
        for (auto& entry : instance->get_hash_table())
        {
          mark_value(entry.value);
        }
        mark_class(*instance->get_class());
      }
    }
  }
  void VM::sweep()
  {
    // string_pool
    string_pool.sweep();
    // tuple_pool
    std::erase_if(gc_index.tuple_pool, [this](gsl::not_null<Tuple*> tuple) {
#ifdef FOXLOX_DEBUG_LOG_GC
      std::cout << std::format("sweeping {} [{}]: {}\n", static_cast<const void*>(tuple), tuple->is_marked() ? "is_marked" : "not_marked", tuple->is_marked() ? Value(tuple).to_string() : "<tuple elem may not avail>");
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
    std::erase_if(gc_index.instance_pool, [this](gsl::not_null<Instance*> instance) {
#ifdef FOXLOX_DEBUG_LOG_GC
      std::cout << std::format("sweeping {} [{}]: {}\n", static_cast<const void*>(instance), instance->is_marked() ? "is_marked" : "not_marked", Value(instance).to_string());
#endif
      if (!instance->is_marked())
      {
        Instance::free(deallocator, instance);
        return true;
      }
      instance->unmark();
      return false;
      });
    // dict_pool
    std::erase_if(gc_index.dict_pool, [this](gsl::not_null<Dict*> dict) {
#ifdef FOXLOX_DEBUG_LOG_GC
      std::cout << std::format("sweeping {} [{}]: {}\n", static_cast<const void*>(dict), dict->is_marked() ? "is_marked" : "not_marked", Value(dict).to_string());
#endif
      if (!dict->is_marked())
      {
        Dict::free(deallocator, dict);
        return true;
      }
      dict->unmark();
      return false;
      });
    // whiten all subroutines
    for (auto& c : chunks)
    {
      for (auto& s : c.get_subroutines())
      {
        s.unmark();
      }
    }
    // whiten all classes
    for (auto& c : class_pool)
    {
      c.unmark();
    }
  }
  Dict* VM::import_lib(std::span<const std::string_view> libpath)
  {
    auto combined_path = libpath | ranges::views::join('.') | ranges::to<std::string>;
    if (const auto found = runtime_libs.find(combined_path); found != runtime_libs.end())
    {
      // an internal lib
      const gsl::not_null<Dict*> p = Dict::alloc(allocator, deallocator);
      gc_index.dict_pool.push_back(p);
      for (auto& val : found->second)
      {
        p->set(string_pool.add_string(val.name), val.val);
      }
      return p;
    }
    else
    {
      // external lib
      const auto filepath = findlib(libpath);
      auto [res, chunkdata] = compile_file(filepath);
      if (res != CompilerResult::OK)
      {
        throw InternalRuntimeError(std::format("Failed to load file: {}.", filepath.string()));
      }
      load_binary(chunkdata);
      Chunk& loaded_chunk = chunks.back();
      push_calltrace(0);
      jump_to_func(&loaded_chunk.get_subroutines().front());
      run();
      const gsl::not_null<Dict*> p = gen_export_dict();
      pop_calltrace();
      return p;
    }
  }
  void VM::jump_to_func(Subroutine* func) noexcept
  {
    current_subroutine = func;
    current_chunk = current_subroutine->get_chunk();
    ip = current_subroutine->get_code().begin();
  }
  void VM::pop_calltrace() noexcept
  {
    p_calltrace--;
    current_subroutine = p_calltrace->subroutine;
    current_super_level = p_calltrace->super_level;
    current_chunk = current_subroutine->get_chunk();
    ip = p_calltrace->ip;
    stack_top = p_calltrace->stack_top;
  }
  void VM::push_calltrace(uint16_t num_of_params) noexcept
  {
    p_calltrace->subroutine = current_subroutine;
    p_calltrace->super_level = current_super_level;
    p_calltrace->ip = ip;
    p_calltrace->stack_top = stack_top - num_of_params;
    p_calltrace++;
  }
  Dict* VM::gen_export_dict()
  {
    const gsl::not_null<Dict*> dict = Dict::alloc(allocator, deallocator);
    gc_index.dict_pool.push_back(dict);
    for (const auto& exp : current_chunk->get_export_list())
    {
      auto name = const_string_pool.at(current_chunk->get_const_string_idx_base() + exp.name_idx);
      auto val = static_value_pool.at(current_chunk->get_static_value_idx_base() + exp.value_idx);
      dict->set(name, val);
    }
    return dict;
  }
  std::filesystem::path VM::findlib(std::span<const std::string_view> libpath)
  {
    namespace fs = std::filesystem;
    auto pathstr = libpath
      | ranges::views::join('/')
      | ranges::to<std::string>;
    fs::path pathobj(pathstr + ".fox");
    // relative to current chunk
    if (auto p = fs::path(current_chunk->get_src_path()).parent_path() / pathobj;
      fs::is_regular_file(p))
    {
      return p;
    }
    // relative to current path
    if (auto p = fs::current_path() / pathobj;
      fs::is_regular_file(p))
    {
      return p;
    }
    // relative to exe  
    if (auto p = boost::dll::program_location().string() / pathobj;
      fs::is_regular_file(p))
    {
      return p;
    }
    throw InternalRuntimeError(std::format("Failed to find file: {}.", pathobj.string()));
  }
}
