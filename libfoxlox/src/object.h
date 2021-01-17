#pragma once
#include <climits>
#include <cassert>
#include <version>
#include <unordered_map>

#include <gsl/gsl>

#include "container_helper.h"
#include "value.h"

namespace foxlox
{
  template<typename T>
  class SimpleObj : public ObjBase
  {
  public:
    SimpleObj(ObjType t) : ObjBase(t) {}

    auto data() noexcept
    {
      return static_cast<T*>(this)->data_array;
    }
    auto data() const noexcept
    {
      return static_cast<const T*>(this)->data_array;
    }
    bool is_marked() const noexcept
    {
      return bool(real_length() & signbit_mask);
    }
    void mark() noexcept
    {
      real_length() |= signbit_mask;
    }
    void unmark() noexcept
    {
      real_length() &= ~signbit_mask;
    }
    std::size_t size() const noexcept
    {
      return real_length() & ~signbit_mask;
    }
  protected:
    static constexpr auto signbit_mask = 1ull << (sizeof std::size_t * CHAR_BIT - 1);
    static constexpr auto max_length = ~signbit_mask;
  private:
    std::size_t& real_length()
    {
      return static_cast<T*>(this)->length;
    }
    std::size_t real_length() const
    {
      return static_cast<const T*>(this)->length;
    }
  };
  
  class String : public SimpleObj<String>
  {
    friend class SimpleObj<String>;
  private:
    std::size_t length;
    char data_array[sizeof(length)];
  public:
    String(size_t l) : SimpleObj(ObjType::STR), length(l) {}
    ~String() = default;

    template<Allocator F>
    static gsl::not_null<String*> alloc(F allocator, std::size_t l)
    {
      assert(l <= max_length);
      const gsl::not_null<char*> data = l <= sizeof(String::data_array) ? allocator(sizeof(String)) :
        allocator(sizeof(String) + (l - sizeof(String::data_array)));
      return new(data) String(l);
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<const String*> p)
    {
      const auto size = p->size();
      if (size <= sizeof(String::data_array))
      {
        GSL_SUPPRESS(type.1)
          deallocator(reinterpret_cast<const char*>(p.get()), sizeof(String));
      }
      else
      {
        GSL_SUPPRESS(type.1)
          deallocator(reinterpret_cast<const char*>(p.get()), sizeof(String) + (size - sizeof(String::data_array)));
      }
    }

    std::string_view get_view() const noexcept;

#if __cpp_lib_three_way_comparison >= 201907L
    using TStrViewComp = decltype(std::string_view{} <=> std::string_view{});
#else
    using TStrViewComp = std::weak_ordering;
#endif
    friend TStrViewComp operator<=>(const String& l, const String& r);
    friend bool operator==(const String& l, const String& r);
  };

  class Tuple : public SimpleObj<Tuple>
  {
    friend class SimpleObj<Tuple>;
  private:
    std::size_t length;
    Value data_array[1];
  public:
    Tuple(size_t l) : SimpleObj(ObjType::TUPLE), length(l) {}
    ~Tuple() = default;

    template<Allocator F>
    static gsl::not_null<Tuple*> alloc(F allocator, size_t l)
    {
      assert(l <= max_length);
      const gsl::not_null<char*> data = (l == 0) ? allocator(sizeof(Tuple)) :
        allocator(sizeof(Tuple) + (l - 1) * sizeof(Value));
      return new(data) Tuple(l);
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<const Tuple*> p)
    {
      const auto size = p->size();
      p->~Tuple();
      if (size == 0)
      {
        GSL_SUPPRESS(type.1)
          deallocator(reinterpret_cast<const char*>(p.get()), sizeof(Tuple));
      }
      else
      {
        GSL_SUPPRESS(type.1)
          deallocator(reinterpret_cast<const char*>(p.get()), sizeof(Tuple) + (size - 1) * sizeof(Value));
      }
    }

    std::span<const Value> get_span() const noexcept;
    std::span<Value> get_span() noexcept;
  };

  class Class : public ObjBase
  {
  public:
    Class(std::string_view name);
    std::string_view get_name() const noexcept { return class_name; }
    void add_method(std::string_view name, uint16_t func_idx);
    void set_super(Class* super);
    Class* get_super();
    bool has_method(std::string_view name);
    std::pair<bool, uint16_t> try_get_method_idx(std::string_view name);
  private:
    Class* superclass;
    std::string class_name;
    std::unordered_map<std::string_view, uint16_t> methods;
  };

  class Instance : public ObjBase
  {
  public:
    using Fields = std::unordered_map<
      std::string_view, Value,
      std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      AllocatorWrapper<std::pair<const std::string_view, Value>>
    >;

    template<Allocator A, Deallocator D>
    Instance(A allocator, D deallocator, Class* from_class) : 
      ObjBase(ObjType::INSTANCE),
      klass(from_class), 
      fields(
        0, //bucket_count
        std::hash<std::string_view>{},
        std::equal_to<std::string_view>{},
        AllocatorWrapper<std::pair<const std::string_view, Value>>(allocator, deallocator)
      )
    {
    }
    ~Instance() = default;
    Class* get_class() const noexcept;
    Value get_property(std::string_view name, Chunk& chunk);
    Value get_super_method(std::string_view name, Chunk& chunk);
    void set_property(std::string_view name, Value value);

    template<Allocator A, Deallocator D>
    static gsl::not_null<Instance*> alloc(A allocator, D deallocator, Class* klass)
    {
      const gsl::not_null<char*> data = allocator(sizeof(Instance));
      return new(data) Instance(allocator, deallocator, klass);
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<const Instance*> p)
    {
      // call dtor to delete the map inside
      p->~Instance();
      GSL_SUPPRESS(type.1)
      deallocator(reinterpret_cast<const char*>(p.get()), sizeof(Instance));
    }
  private:
    Class* klass;
    Fields fields;
  };

  template<Allocator F>
  static String* Value::strcat(F allocator, const Value& l, const Value& r)
  {
    assert(l.is_str() && r.is_str());
    const auto s1 = l.v.str->get_view();
    const auto s2 = r.v.str->get_view();
    String* p = String::alloc(allocator, s1.size() + s2.size());
    GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
      const auto it = std::copy(s1.begin(), s1.end(), p->data());
    GSL_SUPPRESS(stl.1)
      std::copy(s2.begin(), s2.end(), it);
    return p;
  }
  template<Allocator F>
  static Tuple* Value::tuplecat(F allocator, const Value& l, const Value& r)
  {
    if (l.is_tuple() && r.is_tuple())
    {
      const auto s1 = l.v.tuple->get_span();
      const auto s2 = r.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, s1.size() + s2.size());
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
        const auto it = std::copy(s1.begin(), s1.end(), p->data());
      GSL_SUPPRESS(stl.1)
        std::copy(s2.begin(), s2.end(), it);
      return p;
    }
    if (l.is_tuple())
    {
      const auto s1 = l.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, s1.size() + 1);
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
        const auto it = std::copy(s1.begin(), s1.end(), p->data());
      *it = r;
      return p;
    }
    assert(r.is_tuple());
    {
      const auto s2 = r.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, 1 + s2.size());
      p->data()[0] = l;
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1) GSL_SUPPRESS(bounds.1)
        std::copy(s2.begin(), s2.end(), p->data() + 1);
      return p;
    }
  }
}