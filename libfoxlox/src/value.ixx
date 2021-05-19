export module foxlox:value;

import <array>;
import <climits>;
import <stdexcept>;
import <cstdint>;
import <string>;
import <compare>;
import <string_view>;
import <span>;
import <concepts>;
import <type_traits>;
import <utility>;

import <gsl/gsl>;

namespace foxlox
{
  export class ValueError : public std::runtime_error
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

  export class String;
  export class Tuple;
  export class Subroutine;
  export class Class;
  export class Instance;
  export class Dict;
  export struct Value;
  export class VM;
  export class Chunk;

  export enum class ObjType : uint8_t
  {
    STR, TUPLE, CLASS, INSTANCE, DICT,
    // Not Impl yet:
    ARRAY, BYTES
  };
  export enum class ValueType : uintptr_t
  {
    NIL, OBJ, BOOL, F64, I64, FUNC, CPP_FUNC, METHOD,
    // Not Impl yet:
    CPP_INSTANCE, CPP_CLASS
  };
  export class ObjBase
  {
  public:
    ObjBase(ObjType t) noexcept;
    ObjType type;
  };

  export using CppFunc = Value(VM&, std::span<Value>);

  export struct Value
  {
    /* pointer packing */
    // according to https://unix.stackexchange.com/questions/509607/how-a-64-bit-process-virtual-address-space-is-divided-in-linux
    // and https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/virtual-address-spaces
    // on both windows & linux, and on both amd64 and arm64
    // the userspace address range is not longer than 56bit
    constexpr static auto userspace_addr_bits = 56;
    ValueType type : sizeof(uintptr_t)* CHAR_BIT - userspace_addr_bits;
    uintptr_t method_func_ptr : userspace_addr_bits;

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
      type(ValueType::NIL),
      method_func_ptr(0),
      v{ .i64 = 0 }
    {}

    constexpr Value(std::convertible_to<String*> auto str) noexcept :
      type(ValueType::OBJ),
      method_func_ptr(0),
      v{ .str = str }
    {}

    constexpr Value(std::convertible_to<Tuple*> auto tuple) noexcept :
      type(ValueType::OBJ),
      method_func_ptr(0),
      v{ .tuple = tuple }
    {}

    constexpr Value(std::convertible_to<Subroutine*> auto func) noexcept :
      type(ValueType::FUNC),
      method_func_ptr(0),
      v{ .func = func }
    {}

    constexpr Value(std::convertible_to<CppFunc*> auto cppfunc) noexcept :
      type(ValueType::CPP_FUNC),
      method_func_ptr(0),
      v{ .cppfunc = cppfunc }
    {}

    constexpr Value(std::convertible_to<Instance*> auto instance) noexcept :
      type(ValueType::OBJ),
      method_func_ptr(0),
      v{ .instance = instance }
    {}

    constexpr Value(std::convertible_to<Dict*> auto dict) noexcept :
      type(ValueType::OBJ),
      method_func_ptr(0),
      v{ .dict = dict }
    {}

    GSL_SUPPRESS(type.1)
      constexpr Value(
        std::convertible_to<Instance*> auto instance, 
        std::convertible_to<Subroutine*> auto func
      ) noexcept :
      type(ValueType::METHOD),
      method_func_ptr(reinterpret_cast<uintptr_t>(static_cast<Subroutine*>(func))),
      v{ .instance = instance }
    {}

    constexpr Value(std::convertible_to<Class*> auto klass) noexcept :
      type(ValueType::OBJ),
      method_func_ptr(0),
      v{ .klass = klass }
    {}

    constexpr Value(remove_cv_same_as<bool> auto b) noexcept :
      type(ValueType::BOOL),
      method_func_ptr(0),
      v{ .b = b }
    {}

    constexpr Value(std::floating_point auto f64) noexcept :
      type(ValueType::F64),
      method_func_ptr(0),
      v{ .f64 = f64 }
    {}

    constexpr Value(IntegralExcludeBool auto i64) noexcept :
      type(ValueType::I64),
      method_func_ptr(0),
      v{ .i64 = i64 }
    {}

    constexpr bool is_nil() const noexcept
    {
      return (type == ValueType::NIL);
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

    friend Value operator-(const Value& l, const Value& r);
    friend Value operator-(const Value& val);
    friend bool operator!(const Value& val);
    friend int64_t intdiv(const Value& l, const Value& r);
    friend std::partial_ordering operator<=>(const Value& l, const Value& r);
    friend bool operator==(const Value& l, const Value& r) noexcept;

    std::string to_string() const;
    std::array<uint64_t, 2> serialize() const noexcept;

    bool debug_type_is_valid() noexcept;
  };
}

