#pragma once

#include <climits>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <compare>
#include <string_view>
#include <version>
#include <span>
#include <concepts>
#include <type_traits>

namespace foxlox
{
  class ValueTypeError : public std::exception 
  { 
  public:
    using std::exception::exception; 
  };

  template<typename T1, typename T2>
  concept remove_cv_same_as = std::same_as<std::remove_cv_t<T1>, std::remove_cv_t<T2>>;

  template<typename T>
  concept IntegralExcludeBool = std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;

  template<typename F>
  concept Allocator = requires(F f) {
    { f(std::ptrdiff_t{}) } -> std::convertible_to<char*>;
  };
  template<typename F>
  concept Deallocator = requires(F f) {
    { f(reinterpret_cast<char*>(0), std::ptrdiff_t{}) };
  };

  template<typename T>
  class SimpleObj
  {
  public:
    bool is_marked() const
    {
      return staic_cast<T*>(this)->length < 0;
    }
    void mark()
    {
      staic_cast<T*>(this)->length = staic_cast<T*>(this)->length | signbit_mask;
    }
    void unmark()
    {
      staic_cast<T*>(this)->length = staic_cast<T*>(this)->length & ~signbit_mask;
    }
    std::ptrdiff_t get_length() const
    {
      return staic_cast<T*>(this)->length & ~signbit_mask;
    }
  private:
    static constexpr auto signbit_mask = 1ull << (sizeof std::ptrdiff_t * CHAR_BIT - 1);
  };

  class String : public SimpleObj<String>
  {
  public:
    template<Allocator F>
    static gsl::not_null<String*> alloc(F allocator, std::ptrdiff_t l)
    {
      const gsl::not_null<char*> data = l <= sizeof(String::str) ? allocator(sizeof(String)) :
        allocator(sizeof(String) + (l - sizeof(String::str)));

      GSL_SUPPRESS(type.1)
      gsl::not_null<String*> str = reinterpret_cast<String*>(data.get());
      str->length = l;
      return str;
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<const String*> p)
    {
      if (p->get_length() <= sizeof(String::str))
      {
        GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<const char*>(p.get()), sizeof(String));
      }
      else
      {
        GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<const char*>(p.get()), sizeof(String) + (p->get_length() - sizeof(String::str)));
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

  private:
    std::ptrdiff_t length;
    char str[sizeof(length)];
  };

  class Tuple;
  class Subroutine;

  struct Value
  {
    enum Type : uint8_t
    {
      NIL, BOOL, F64, I64, STR, TUPLE, FUNC
    } type;

    union
    {
      bool b;
      double f64;
      int64_t i64;
      const String* str;
      const Tuple* tuple;
      const Subroutine* func;
    } v;


    Value() noexcept : type(NIL), v{} {}

    template<std::convertible_to<const String*> T>
    Value(T str) noexcept : type(STR), v{ .str = str } {}

    template<std::convertible_to<const Tuple*> T>
    Value(T tuple) noexcept : type(TUPLE), v{ .tuple = tuple } {}

    template<std::convertible_to<const Subroutine*> T>
    Value(T tuple) noexcept : type(FUNC), v{ .func = tuple } {}

    template<remove_cv_same_as<bool> T>
    Value(T b) noexcept : type(BOOL), v{ .b = b } {}

    template<std::floating_point T>
    Value(T f64) noexcept : type(F64), v{ .f64 = f64 } {}

    template<IntegralExcludeBool T>
    Value(T i64) noexcept : type(I64), v{ .i64 = i64 } {}

    double get_double() const;
    int64_t get_int64() const;
    bool get_bool() const;
    std::string_view get_strview() const;
    std::span<const Value> get_tuplespan() const;
    const Subroutine* get_func() const;

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

  class Tuple : public SimpleObj<Tuple>
  {
  public:
    template<Allocator F>
    static gsl::not_null<Tuple*> alloc(F allocator, size_t l)
    {
      const gsl::not_null<char*> data = (l == 0) ? allocator(sizeof(Tuple)) :
        allocator(sizeof(Tuple) + (l - 1) * sizeof(Value));

      GSL_SUPPRESS(type.1)
      gsl::not_null<Tuple*> tuple = reinterpret_cast<Tuple*>(data.get());
      tuple->length = l;
      return tuple;
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<const Tuple*> p)
    {
      if (p->get_length() == 0)
      {
        GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<const char*>(p.get()), sizeof(Tuple));
      }
      else
      {
        GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<const char*>(p.get()), sizeof(Tuple) + (p->get_length() - 1) * sizeof(Value));
      }
    }

    std::span<const Value> get_span() const noexcept;
  private:
    std::ptrdiff_t length;
    Value elems[1];
  };

  template<Allocator F>
  static String* Value::strcat(F allocator, const Value& l, const Value& r)
  {
    assert(l.type == Value::STR && r.type == Value::STR);
    const auto s1 = l.v.str->get_view();
    const auto s2 = r.v.str->get_view();
    String* p = String::alloc(allocator, s1.size() + s2.size());
    GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
    const auto it = std::copy(s1.begin(), s1.end(), p->str);
    GSL_SUPPRESS(stl.1)
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
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
      const auto it = std::copy(s1.begin(), s1.end(), p->elems);
      GSL_SUPPRESS(stl.1)
      std::copy(s2.begin(), s2.end(), it);
      return p;
    }
    if (l.type == Value::TUPLE)
    {
      const auto s1 = l.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, s1.size() + 1);
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
      const auto it = std::copy(s1.begin(), s1.end(), p->elems);
      *it = r;
      return p;
    }
    assert(r.type == Value::TUPLE);
    {
      const auto s2 = r.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, 1 + s2.size());
      p->elems[0] = l;
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1) GSL_SUPPRESS(bounds.1)
      std::copy(s2.begin(), s2.end(), p->elems + 1);
      return p;
    }
  }
}
