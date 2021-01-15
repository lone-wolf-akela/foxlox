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

#include <gsl/gsl>

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
    { f(std::size_t{}) } -> std::convertible_to<char*>;
  };
  template<typename F>
  concept Deallocator = requires(F f) {
    { f(reinterpret_cast<char*>(0), std::size_t{}) };
  };

  template<typename T>
  class ObjBase
  {
  public:
    auto data() noexcept
    {
      return static_cast<T*>(this)->data_array;
    }
    auto data() const noexcept
    {
      return static_cast<const T*>(this)->data_array;
    }
    bool get_mark() const noexcept
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

  class String : public ObjBase<String>
  {
    friend class ObjBase<String>;
  private:
    std::size_t length;
    char data_array[sizeof(length)];
  public:
    template<Allocator F>
    static gsl::not_null<String*> alloc(F allocator, std::size_t l)
    {
      assert(l <= max_length);
      const gsl::not_null<char*> data = l <= sizeof(String::data_array) ? allocator(sizeof(String)) :
        allocator(sizeof(String) + (l - sizeof(String::data_array)));

      GSL_SUPPRESS(type.1)
      gsl::not_null<String*> str = reinterpret_cast<String*>(data.get());
      str->length = l;
      return str;
    }

    template<Deallocator F>
    static void free(F deallocator, gsl::not_null<const String*> p)
    {
      if (p->size() <= sizeof(String::data_array))
      {
        GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<const char*>(p.get()), sizeof(String));
      }
      else
      {
        GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<const char*>(p.get()), sizeof(String) + (p->size() - sizeof(String::data_array)));
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

  class Tuple;
  class Subroutine;
  struct Value;
  class VM;

  using CppFunc = Value(VM&, std::span<Value>);

  struct Value
  {
    enum Type : uint8_t
    {
      NIL, BOOL, F64, I64, STR, TUPLE, FUNC, CPP_FUNC,
    } type;

    union
    {
      bool b;
      double f64;
      int64_t i64;
      String* str;
      Tuple* tuple;
      Subroutine* func;
      CppFunc* cppfunc;
    } v;


    Value() noexcept : type(NIL), v{} {}

    template<std::convertible_to<String*> T>
    Value(T str) noexcept : type(STR), v{ .str = str } {}

    template<std::convertible_to<Tuple*> T>
    Value(T tuple) noexcept : type(TUPLE), v{ .tuple = tuple } {}

    template<std::convertible_to<Subroutine*> T>
    Value(T func) noexcept : type(FUNC), v{ .func = func } {}

    template<std::convertible_to<CppFunc*> T>
    Value(T cppfunc) noexcept : type(CPP_FUNC), v{ .cppfunc = cppfunc } {}

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

  class Tuple : public ObjBase<Tuple>
  {
    friend class ObjBase<Tuple>;
  private:
    std::size_t length;
    Value data_array[1];
  public:
    template<Allocator F>
    static gsl::not_null<Tuple*> alloc(F allocator, size_t l)
    {
      assert(l <= max_length);
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
      if (p->size() == 0)
      {
        GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<const char*>(p.get()), sizeof(Tuple));
      }
      else
      {
        GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<const char*>(p.get()), sizeof(Tuple) + (p->size() - 1) * sizeof(Value));
      }
    }

    std::span<const Value> get_span() const noexcept;
    std::span<Value> get_span() noexcept;
  };

  template<Allocator F>
  static String* Value::strcat(F allocator, const Value& l, const Value& r)
  {
    assert(l.type == Value::STR && r.type == Value::STR);
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
    if (l.type == Value::TUPLE && r.type == Value::TUPLE)
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
    if (l.type == Value::TUPLE)
    {
      const auto s1 = l.v.tuple->get_span();
      Tuple* p = Tuple::alloc(allocator, s1.size() + 1);
      GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
      const auto it = std::copy(s1.begin(), s1.end(), p->data());
      *it = r;
      return p;
    }
    assert(r.type == Value::TUPLE);
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
