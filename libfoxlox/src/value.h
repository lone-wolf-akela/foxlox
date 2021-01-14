#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <compare>
#include <string_view>
#include <version>
#include <span>
#include <concepts>

namespace foxlox
{
  template<typename F>
  concept Allocator = requires(F f) {
    { f(size_t(0)) } -> std::convertible_to<char*>;
  };
  template<typename F>
  concept Deallocator = requires(F f) {
    { f(reinterpret_cast<char*>(0), size_t(0)) };
  };

  struct String
  {
    size_t length;
    char str[sizeof(length)];

    template<Allocator F>
    static String* alloc(F allocator, size_t l)
    {
      char* data = l <= sizeof(String::str) ? allocator(sizeof(String)) :
        allocator(sizeof(String) + (l - sizeof(String::str)));
      assert(data != nullptr);
      String* str = reinterpret_cast<String*>(data);
      str->length = l;
      return str;
    }

    template<Deallocator F>
    static void free(F deallocator, String* p)
    {
      if (p->length <= sizeof(String::str))
      {
        deallocator(reinterpret_cast<char*>(p), sizeof(String));
      }
      else
      {
        deallocator(reinterpret_cast<char*>(p), sizeof(String) + (p->length - sizeof(String::str)));
      }
    }

    std::string_view get_view() const;

#if __cpp_lib_three_way_comparison >= 201907L
    using TStrViewComp = decltype(std::string_view{} <=> std::string_view{});
#else
    using TStrViewComp = std::weak_ordering;
#endif
    friend TStrViewComp operator<=>(const String& l, const String& r);
    friend bool operator==(const String& l, const String& r);
  };
  struct Tuple;

  struct Value
  {
    enum : uint8_t
    {
      NIL, BOOL, F64, I64, STR, TUPLE
    } type;
    union
    {
      bool b;
      double f64;
      int64_t i64;
      String* str;
      Tuple* tuple;
    } v;

    Value();
    Value(String* str);
    Value(Tuple* tuple);
    Value(bool b);
    Value(double f64);
    Value(int64_t i64);

    double get_double() const;
    int64_t get_int64() const;
    bool get_bool() const;
    std::string_view get_strview() const;
    std::span<const Value> get_tuplespan() const;

    friend double operator/(const Value& l, const Value& r);
    friend Value operator*(const Value& l, const Value& r);
    friend Value operator+(const Value& l, const Value& r);

    template<Allocator F>
    static String* strcat(F allocator, const Value& l, const Value& r);
    template<Allocator F>
    static Tuple* tuplecat(F allocator, const Value& l, const Value& r);

    friend Value operator-(const Value& l, const Value& r);
    friend Value operator-(const Value& val);
    friend bool operator!(const Value& val);
    friend int64_t intdiv(const Value& l, const Value& r);
    friend std::partial_ordering operator<=>(const Value& l, const Value& r);
    friend bool operator==(const Value& l, const Value& r);

    std::string to_string() const;
  };

  struct Tuple
  {
    size_t length;
    Value elems[1];
    const int a = sizeof(Value);

    template<Allocator F>
    static Tuple* alloc(F allocator, size_t l)
    {
      char* data = (l == 0) ? allocator(sizeof(Tuple)) :
        allocator(sizeof(Tuple) + (l - 1) * sizeof(Value));
      assert(data != nullptr);
      Tuple* tuple = reinterpret_cast<Tuple*>(data);
      tuple->length = l;
      return tuple;
    }

    template<Deallocator F>
    static void free(F deallocator, Tuple* p)
    {
      char* data{};
      if (p->length == 0)
      {
        deallocator(reinterpret_cast<char*>(p), sizeof(Tuple));
      }
      else
      {
        deallocator(reinterpret_cast<char*>(p), sizeof(Tuple) + (p->length - 1) * sizeof(Value));
      }
    }

    std::span<const Value> get_span() const;
  };

  template<Allocator F>
  static String* Value::strcat(F allocator, const Value& l, const Value& r)
  {
    assert(l.type == Value::STR && r.type == Value::STR);
    const auto s1 = l.v.str->get_view();
    const auto s2 = r.v.str->get_view();
    String* p = String::alloc(allocator, s1.size() + s2.size());
    const auto it = std::copy(s1.begin(), s1.end(), p->str);
    std::copy(s2.begin(), s2.end(), it);
    return p;
  }
  template<Allocator F>
  static Tuple* Value::tuplecat(F allocator, const Value& l, const Value& r)
  {
    if (l.type == Value::TUPLE && r.type == Value::TUPLE)
    {
      const auto s1 = l.v.tuple->get_span();
      const auto s2 = r.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, s1.size() + s2.size());
      const auto it = std::copy(s1.begin(), s1.end(), p->elems);
      std::copy(s2.begin(), s2.end(), it);
      return p;
    }
    if (l.type == Value::TUPLE)
    {
      const auto s1 = l.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, s1.size() + 1);
      const auto it = std::copy(s1.begin(), s1.end(), p->elems);
      *it = r;
      return p;
    }
    assert(r.type == Value::TUPLE);
    {
      const auto s2 = r.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, 1 + s2.size());
      p->elems[0] = l;
      std::copy(s2.begin(), s2.end(), p->elems + 1);
      return p;
    }
  }
}
