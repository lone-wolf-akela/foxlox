#pragma once
#include <climits>
#include <version>
#include <unordered_map>

#include <gsl/gsl>

#include "config.h"
#include "container_helper.h"
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
      return static_cast<T*>(this)->m_data;
    }
    auto data() const noexcept
    {
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
    static void free(F deallocator, gsl::not_null<const String*> p)
    {
      const auto size = p->size();
      p->~String();
      deallocator(reinterpret_cast<const char*>(p.get()), sizeof(String) + size * sizeof(char));
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
    static void free(F deallocator, gsl::not_null<const Tuple*> p)
    {
      const auto size = p->size();
      p->~Tuple();
      deallocator(reinterpret_cast<const char*>(p.get()), sizeof(Tuple) + size * sizeof(Value));
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
    void set_super(Class* super);
    Class* get_super();
    bool has_method(String* name);
    std::pair<bool, Subroutine*> try_get_method_idx(String* name);
    std::unordered_map<String*, Subroutine*>& get_all_methods();

    bool is_marked();
    void mark();
    void unmark();
  private:
    Class* superclass;
    std::string class_name;
    std::unordered_map<String*, Subroutine*> methods;
    bool gc_mark;
  };

  class Instance : public ObjBase
  {
  public:
    using Fields = std::unordered_map<
      String*, Value,
      std::hash<String*>,
      std::equal_to<String*>,
      AllocatorWrapper<std::pair<String* const, Value>>
    >;

    template<Allocator A, Deallocator D>
    Instance(A allocator, D deallocator, Class* from_class) :
      ObjBase(ObjType::INSTANCE),
      klass(from_class),
      fields(
        HASH_TABLE_START_BUCKET, //bucket_count
        std::hash<String*>{},
        std::equal_to<String*>{},
        AllocatorWrapper<std::pair<String* const, Value>>(allocator, deallocator)
      ),
      gc_mark(false)
    {
    }
    ~Instance() = default;
    Class* get_class() const noexcept;
    Value get_property(String* name);
    Value get_super_method(String* name);
    Fields& get_all_fields();
    void set_property(String* name, Value value);
    bool is_marked();
    void mark();
    void unmark();

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
    bool gc_mark;
  };

  template<Allocator F>
  Tuple* Value::tuplecat(F allocator, const Value& l, const Value& r)
  {
    Expects(l.is_tuple() || r.is_tuple());
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
    else if (l.is_tuple())
    {
      const auto s1 = l.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, s1.size() + 1);
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
        const auto it = std::copy(s1.begin(), s1.end(), p->data());
      *it = r;
      return p;
    }
    else
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