#pragma once

#include <cstdint>
#include <string>
#include <compare>
#include <string_view>
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
    { f(std::size_t{}) } -> std::convertible_to<char*>;
  };
  template<typename F>
  concept Deallocator = requires(F f) {
    { f(reinterpret_cast<char*>(0), std::size_t{}) };
  };

  class String;
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
}
