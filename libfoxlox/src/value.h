#pragma once
#include <climits>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <compare>
#include <string_view>
#include <span>
#include <concepts>
#include <type_traits>
#include <utility>

#include <gsl/gsl>

namespace foxlox
{
  class ValueError : public std::runtime_error
  { 
  public:
    using std::runtime_error::runtime_error;
  };

  template<typename T1, typename T2>
  concept remove_cv_same_as = std::same_as<std::remove_cv_t<T1>, std::remove_cv_t<T2>>;

  template<typename T>
  concept IntegralExcludeBool = std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;
  
  template<typename F>
  concept Allocator = std::is_invocable_r_v<char*, F, size_t>;

  template<typename F>
  concept Deallocator = std::is_invocable_r_v<void, F, char* const, size_t>;

  class String;
  class Tuple;
  class Subroutine;
  class Class;
  class Instance;
  class Dict;
  struct Value;
  class VM;
  class Chunk;

  enum class ObjType : uint8_t
  {
    NIL, STR, TUPLE, CLASS, INSTANCE, DICT, ARRAY
  };
  enum class ValueType : uintptr_t
  {
    OBJ, BOOL, F64, I64, FUNC, CPP_FUNC, METHOD
  };
  class ObjBase
  {
  public:
    ObjBase(ObjType t) noexcept;
    ObjType type;
  };

  using CppFunc = Value(VM&, std::span<Value>);

  struct Value
  {
    // pointer packing
    constexpr static auto method_func_shift = 3;
    uintptr_t method_func_ptr : sizeof(uintptr_t)* CHAR_BIT - method_func_shift;
    ValueType type : method_func_shift;

    union
    {
      bool b;
      double f64;
      int64_t i64;
      String* str;
      Tuple* tuple;
      Subroutine* func;
      CppFunc* cppfunc;
      Class* klass;
      Instance* instance;
      Dict* dict;
      ObjBase* obj;
    } v;


    constexpr Value() noexcept : 
      method_func_ptr(0), 
      type(ValueType::OBJ), 
      v{ .obj = nullptr } 
    {}

    template<std::convertible_to<String*> T>
    constexpr Value(T str) noexcept : 
      method_func_ptr(0), 
      type(ValueType::OBJ), 
      v{ .str = str } 
    {}

    template<std::convertible_to<Tuple*> T>
    constexpr Value(T tuple) noexcept : 
      method_func_ptr(0), 
      type(ValueType::OBJ), 
      v{ .tuple = tuple } 
    {}

    template<std::convertible_to<Subroutine*> T>
    constexpr Value(T func) noexcept : 
      method_func_ptr(0), 
      type(ValueType::FUNC), 
      v{ .func = func } 
    {}

    template<std::convertible_to<CppFunc*> T>
    constexpr Value(T cppfunc) noexcept : 
      method_func_ptr(0), 
      type(ValueType::CPP_FUNC), 
      v{ .cppfunc = cppfunc } 
    {}

    template<std::convertible_to<Instance*> T>
    constexpr Value(T instance) noexcept : 
      method_func_ptr(0), 
      type(ValueType::OBJ), 
      v{ .instance = instance } 
    {}

    template<std::convertible_to<Dict*> T>
    constexpr Value(T dict) noexcept : 
      method_func_ptr(0), 
      type(ValueType::OBJ), 
      v{ .dict = dict } 
    {}

    template<std::convertible_to<Instance*> I, std::convertible_to<Subroutine*> S >
    GSL_SUPPRESS(type.1)
    constexpr Value(I instance, S func) noexcept :
      method_func_ptr(reinterpret_cast<uintptr_t>(static_cast<Subroutine*>(func)) >> method_func_shift),
      type(ValueType::METHOD), 
      v{ .instance = instance } 
    {}

    template<std::convertible_to<Class*> T>
    constexpr Value(T klass) noexcept : 
      method_func_ptr(0), 
      type(ValueType::OBJ), 
      v{ .klass = klass } 
    {}

    template<remove_cv_same_as<bool> T>
    constexpr Value(T b) noexcept : 
      method_func_ptr(0), 
      type(ValueType::BOOL), 
      v{ .b = b } 
    {}

    template<std::floating_point T>
    constexpr Value(T f64) noexcept : 
      method_func_ptr(0), 
      type(ValueType::F64), 
      v{ .f64 = f64 } 
    {}

    template<IntegralExcludeBool T>
    constexpr Value(T i64) noexcept : 
      method_func_ptr(0), 
      type(ValueType::I64), 
      v{ .i64 = i64 } 
    {}

    constexpr bool is_nil() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj == nullptr);
    }

    constexpr bool is_str() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::STR);
    }

    constexpr bool is_tuple() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::TUPLE);
    }

    constexpr bool is_class() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::CLASS);
    }

    constexpr bool is_instance() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::INSTANCE);
    }

    constexpr bool is_dict() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::DICT);
    }

    constexpr bool is_array() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::ARRAY);
    }

    double get_double() const;
    int64_t get_int64() const;
    
    Instance* get_instance() const;
    Dict* get_dict() const;
    std::string_view get_strview() const;
    std::span<Value> get_tuplespan() const;
    Value get_property(gsl::not_null<String*> name);

    bool is_truthy() const noexcept;
    Subroutine* method_func() const noexcept;

    friend double operator/(const Value& l, const Value& r);
    friend Value operator*(const Value& l, const Value& r);
    friend Value operator+(const Value& l, const Value& r);

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
