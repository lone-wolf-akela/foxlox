#pragma once
#include <climits>
#include <version>

#include <gsl/gsl>

#include "hash_table.h"
#include "value.h"

namespace foxlox
{
  template<typename T>
  class SimpleObj : public ObjBase
  {
  private:
    bool gc_mark;
    uint32_t m_size;
  public:
    SimpleObj(ObjType t, size_t l) noexcept :
      ObjBase(t), gc_mark(false), m_size(gsl::narrow_cast<uint32_t>(l))
    {
      Expects(l <= std::numeric_limits<decltype(m_size)>::max());
    }
    SimpleObj(const SimpleObj&) = delete;
    SimpleObj(SimpleObj&&) = delete;
    SimpleObj& operator=(const SimpleObj&) = delete;
    SimpleObj& operator=(SimpleObj&&) = delete;

    auto data() noexcept
    {
      GSL_SUPPRESS(type.2) GSL_SUPPRESS(bounds.3)
      return static_cast<T*>(this)->m_data;
    }
    auto data() const noexcept
    {
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(type.2)
      return static_cast<const T*>(this)->m_data;
    }
    bool is_marked() const noexcept
    {
      return gc_mark;
    }
    void mark() noexcept
    {
      gc_mark = true;
    }
    void unmark() noexcept
    {
      gc_mark = false;
    }
    size_t size() const noexcept
    {
      return m_size;
    }
  };

  class String : public SimpleObj<String>
  {
    friend class SimpleObj<String>;
  private:
#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union
    char m_data[0];
#pragma warning(default:4200)
  public:
    String(size_t l) noexcept;
    ~String() = default;

    template<Allocator F>
    static gsl::not_null<String*> alloc(F allocator, size_t l)
    {
      const gsl::not_null<char*> data = allocator(sizeof(String) + l * sizeof(char));
      return new(data) String(l);
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<String*> p)
    {
      const auto size = p->size();
      p->~String();
      GSL_SUPPRESS(type.1)
      deallocator(reinterpret_cast<char*>(p.get()), sizeof(String) + size * sizeof(char));
    }

    std::string_view get_view() const noexcept;

#if __cpp_lib_three_way_comparison >= 201907L
    using TStrViewComp = decltype(std::string_view{} <=> std::string_view{});
#else
    using TStrViewComp = std::weak_ordering;
#endif
    friend TStrViewComp operator<=>(const String& l, const String& r) noexcept;
    friend bool operator==(const String& l, const String& r) noexcept;
  };


  class Tuple : public SimpleObj<Tuple>
  {
    friend class SimpleObj<Tuple>;
  private:
#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union
    Value m_data[0];
#pragma warning(default:4200)
  public:
    Tuple(size_t l) noexcept;
    ~Tuple() = default;

    template<Allocator F>
    static gsl::not_null<Tuple*> alloc(F allocator, size_t l)
    {
      const gsl::not_null<char*> data = allocator(sizeof(Tuple) + l * sizeof(Value));
      return new(data) Tuple(l);
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<Tuple*> p)
    {
      const auto size = p->size();
      p->~Tuple();
      GSL_SUPPRESS(type.1)
      deallocator(reinterpret_cast<char*>(p.get()), sizeof(Tuple) + size * sizeof(Value));
    }

    std::span<const Value> get_span() const noexcept;
    std::span<Value> get_span() noexcept;
  };

  class Class : public ObjBase
  {
  public:
    Class(std::string_view name);
    std::string_view get_name() const noexcept { return class_name; }
    void add_method(String* name, Subroutine* func);
    void set_super(gsl::not_null<Class*> super);
    Class* get_super() noexcept;
    bool has_method(String* name);
    std::optional<Subroutine*> get_method(String* name);
    HashTable<Subroutine*>& get_hash_table() noexcept;

    bool is_marked() const noexcept;
    void mark() noexcept;
    void unmark() noexcept;
  private:
    bool gc_mark;
    Class* superclass;
    std::string class_name;
    HashTable<Subroutine*> methods;
  };

  class Instance : public ObjBase
  {
  public:
    template<Allocator A, Deallocator D>
    Instance(A allocator, D deallocator, Class* from_class) :
      ObjBase(ObjType::INSTANCE),
      gc_mark(false),
      klass(from_class),
      fields(allocator, deallocator)
    {
    }
    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&&) = delete;
    ~Instance() = default;
    Class* get_class() const noexcept;
    Value get_property(gsl::not_null<String*> name);
    Value get_super_method(gsl::not_null<String*> name);
    HashTable<Value>& get_hash_table() noexcept;
    void set_property(gsl::not_null<String*> name, Value value);
    bool is_marked() const noexcept;
    void mark() noexcept;
    void unmark() noexcept;

    template<Allocator A, Deallocator D>
    static gsl::not_null<Instance*> alloc(A allocator, D deallocator, Class* klass)
    {
      const gsl::not_null<char*> data = allocator(sizeof(Instance));
      return new(data) Instance(allocator, deallocator, klass);
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<Instance*> p)
    {
      // call dtor to delete the map inside
      p->~Instance();
      GSL_SUPPRESS(type.1)
      deallocator(reinterpret_cast<char*>(p.get()), sizeof(Instance));
    }
  private:
    bool gc_mark;
    Class* klass;
    HashTable<Value> fields;
  };

  template<Allocator F>
  Tuple* Value::tuplecat(F allocator, const Value& l, const Value& r)
  {
    Expects(l.is_tuple() || r.is_tuple());
    if (l.is_tuple() && r.is_tuple())
    {
      const auto s1 = l.v.tuple->get_span();
      const auto s2 = r.v.tuple->get_span();
      const gsl::not_null p = Tuple::alloc(allocator, s1.size() + s2.size());
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
        const auto it = std::copy(s1.begin(), s1.end(), p->data());
      GSL_SUPPRESS(stl.1)
        std::copy(s2.begin(), s2.end(), it);
      return p;
    }
    else if (l.is_tuple())
    {
      const auto s1 = l.v.tuple->get_span();
      const gsl::not_null p = Tuple::alloc(allocator, s1.size() + 1);
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
        const auto it = std::copy(s1.begin(), s1.end(), p->data());
      *it = r;
      return p;
    }
    else
    {
      const auto s2 = r.v.tuple->get_span();
      const gsl::not_null p = Tuple::alloc(allocator, 1 + s2.size());
      GSL_SUPPRESS(bounds.1)
      p->data()[0] = l;
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1) GSL_SUPPRESS(bounds.1)
        std::copy(s2.begin(), s2.end(), p->data() + 1);
      return p;
    }
  }

  class Dict : public ObjBase
  {
  public:
    template<Allocator A, Deallocator D>
    Dict(A allocator, D deallocator) :
      ObjBase(ObjType::DICT),
      gc_mark(false),
      fields(allocator, deallocator)
    {
    }
    Dict(const Instance&) = delete;
    Dict(Instance&&) = delete;
    Dict& operator=(const Instance&) = delete;
    Dict& operator=(Instance&&) = delete;
    ~Dict() = default;
    Value get(gsl::not_null<String*> name);
    HashTable<Value>& get_hash_table() noexcept;
    void set(gsl::not_null<String*> name, Value value);
    bool is_marked() const noexcept;
    void mark() noexcept;
    void unmark() noexcept;

    template<Allocator A, Deallocator D>
    static gsl::not_null<Dict*> alloc(A allocator, D deallocator)
    {
      const gsl::not_null<char*> data = allocator(sizeof(Dict));
      return new(data) Dict(allocator, deallocator);
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<Dict*> p)
    {
      // call dtor to delete the map inside
      p->~Dict();
      GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<char*>(p.get()), sizeof(Dict));
    }
  private:
    bool gc_mark;
    HashTable<Value> fields;
  };
}