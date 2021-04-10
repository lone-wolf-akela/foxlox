#include <array>
#include <concepts>

#include <gsl/gsl>
#include <fmt/format.h>
#include <magic_enum.hpp>

#include <foxlox/except.h>

#include <foxlox/chunk.h>
#include "value.h"
#include "object.h"

namespace
{
  using namespace foxlox;

  template<typename ... Args>
  std::string wrongtype_msg_fmt(Args ... expected) 
    requires ((std::same_as<Args, ValueType> || std::same_as<Args, ObjType>) && ...)
  {
    std::string msg = "Value type error. Expected: ";
    (fmt::format_to(std::back_inserter(msg), "{}, ", magic_enum::enum_name(expected)), ...);
    return msg;
  }

  template<typename T1, typename T2, typename ... Args>
  ValueError exception_wrongtype_binop(T1 got1, T2 got2, Args ... expected)
    requires (std::same_as<T1, ValueType> || std::same_as<T1, ObjType>) &&
    (std::same_as<T2, ValueType> || std::same_as<T2, ObjType>) &&
    ((std::same_as<Args, ValueType> || std::same_as<Args, ObjType>) && ...)
  {
    std::string msg = wrongtype_msg_fmt(expected...);
    fmt::format_to(std::back_inserter(msg), "got: {} and {}.", magic_enum::enum_name(got1), magic_enum::enum_name(got2));
    return ValueError(msg);
  }

  template<typename T, typename ... Args> 
  ValueError exception_wrongtype(T got, Args ... expected)
    requires (std::same_as<T, ValueType> || std::same_as<T, ObjType>) &&
    ((std::same_as<Args, ValueType> || std::same_as<Args, ObjType>) && ...)
  {
    std::string msg = wrongtype_msg_fmt(expected...);
    fmt::format_to(std::back_inserter(msg), "got: {}.", magic_enum::enum_name(got));
    return ValueError(msg);
  }

  constexpr void type_check(const Value& got, ValueType expected)
  {
    if (got.type != expected)
    {
      throw exception_wrongtype(got.type, expected);
    }
  }

  constexpr void type_check(const Value& got, ObjType expected)
  {
    if (got.type != ValueType::OBJ)
    {
      throw exception_wrongtype(got.type, expected);
    }
    if (got.v.obj == nullptr)
    {
      throw exception_wrongtype(ObjType::NIL, expected);
    }
    if (got.v.obj->type != expected)
    {
      throw exception_wrongtype(got.v.obj->type, expected);
    }
  }
}

namespace foxlox
{
  // keep Value small and fast!
  static_assert(sizeof(Value) == 16);
  static_assert(std::is_trivially_copyable_v<Value>);
  // I want to make sure a Value which is memset to all 0 is a nil
  // but I cannot check that at compile time
  // so that test is now in unittest

  ObjBase::ObjBase(ObjType t) noexcept : 
    type(t) 
  {
  }

  double Value::get_double() const
  {
    if (type == ValueType::F64) { return v.f64; }
    if (type != ValueType::I64)
    {
      throw exception_wrongtype(type, ValueType::I64, ValueType::F64);
    }
    return static_cast<double>(v.i64);
  }
  int64_t Value::get_int64() const
  {
    if (type == ValueType::I64) { return v.i64; }
    if (type != ValueType::F64)
    {
      throw exception_wrongtype(type, ValueType::I64, ValueType::F64);
    }
    return static_cast<int64_t>(v.f64);
  }

  bool Value::is_truthy() const noexcept
  {
    if ((type == ValueType::OBJ && v.obj == nullptr) ||
      (type == ValueType::BOOL && v.b == false))
    {
      return false;
    }
    return true;
  }

  Instance* Value::get_instance() const
  {
    type_check(*this, ObjType::INSTANCE);
    return v.instance;
  }

  std::string_view Value::get_strview() const
  {
    type_check(*this, ObjType::STR);
    return v.str->get_view();
  }

  std::span<Value> Value::get_tuplespan() const
  {
    type_check(*this, ObjType::TUPLE);
    return v.tuple->get_span();
  }

  Subroutine* Value::method_func() const noexcept
  {
    GSL_SUPPRESS(type.1)
    return reinterpret_cast<Subroutine*>(method_func_ptr << method_func_shift);
  }

  std::partial_ordering operator<=>(const Value& l, const Value& r)
  {
    if (l.is_nil() && r.is_nil())
    {
      return std::partial_ordering::equivalent;
    }
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return l.v.i64 <=> r.v.i64;
    }
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return l.get_double() <=> r.get_double();
    }
    if (l.type == ValueType::BOOL && r.type == ValueType::BOOL)
    {
      return l.v.b <=> r.v.b;
    }
    if (l.is_str() && r.is_str())
    {
      return *l.v.str <=> *r.v.str;
    }
    if (l.is_tuple() && r.is_tuple())
    {
      return l.v.tuple == r.v.tuple ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.type == ValueType::FUNC && r.type == ValueType::FUNC)
    {
      return l.v.func == r.v.func ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.type == ValueType::CPP_FUNC && r.type == ValueType::CPP_FUNC)
    {
      return l.v.cppfunc == r.v.cppfunc ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.type == ValueType::METHOD && r.type == ValueType::METHOD)
    {
      return l.method_func() == r.method_func() && l.v.instance == r.v.instance ? 
        std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.is_class() && r.is_class())
    {
      return l.v.klass == r.v.klass ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.is_instance() && r.is_instance())
    {
      return l.v.instance == r.v.instance ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.is_dict() && r.is_dict())
    {
      return l.v.dict == r.v.dict ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.is_array() && r.is_array())
    {
      throw UnimplementedError("");
    }
    return std::partial_ordering::unordered;
  }
  double operator/(const Value& l, const Value& r)
  {
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return l.get_double() / r.get_double();
    }
    throw exception_wrongtype_binop(l.type, r.type, ValueType::I64, ValueType::F64);
  }
  Value operator*(const Value& l, const Value& r)
  {
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return Value(l.v.i64 * r.v.i64);
    }
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return Value(l.get_double() * r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, ValueType::I64, ValueType::F64);
  }
  Value operator+(const Value& l, const Value& r)
  {
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return Value(l.v.i64 + r.v.i64);
    }
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return Value(l.get_double() + r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, ValueType::I64, ValueType::F64);
  }
  Value operator-(const Value& l, const Value& r)
  {
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return Value(l.v.i64 - r.v.i64);
    }
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return Value(l.get_double() - r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, ValueType::I64, ValueType::F64);
  }
  Value operator-(const Value& val)
  {
    if (val.type == ValueType::F64) { return Value(-val.v.f64); }
    if (val.type != ValueType::I64)
    {
      throw exception_wrongtype(val.type, ValueType::I64, ValueType::F64);
    }
    return Value(-val.v.i64);
  }
  bool operator!(const Value& val)
  {
    type_check(val, ValueType::BOOL);
    return !val.v.b;
  }
  int64_t intdiv(const Value& l, const Value& r)
  {
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return l.v.i64 / r.v.i64;
    }
    return static_cast<int64_t>(l.get_double() / r.get_double());
  }

  bool operator==(const Value& l, const Value& r)
  {
    // directly compare raw data 
    // TODO: use bit_cast when gcc supports
    std::array<uint64_t, 2> u64s_l;
    std::array<uint64_t, 2> u64s_r;
    std::memcpy(&u64s_l, &l, sizeof(l));
    std::memcpy(&u64s_r, &r, sizeof(r));
    return (u64s_l[0] == u64s_r[0]) && (u64s_l[1] == u64s_r[1]);
  }

  Dict* Value::get_dict() const
  {
    type_check(*this, ObjType::DICT);
    return v.dict;
  }

  std::string Value::to_string() const
  {
    switch (type)
    {
    case ValueType::BOOL:
      return v.b ? "true" : "false";
    case ValueType::F64:
      return fmt::format("{}", v.f64);
    case ValueType::I64:
      return fmt::format("{}", v.i64);
    case ValueType::FUNC:
      return fmt::format("<fn {}>", v.func->get_funcname());
    case ValueType::CPP_FUNC:
      GSL_SUPPRESS(type.1)
      //return fmt::format("<native fn {}>", reinterpret_cast<void*>(v.cppfunc));
      return "<native fn>";
    case ValueType::METHOD:
      return fmt::format("<class {} method {}>", v.instance->get_class()->get_name(), method_func()->get_funcname());
    case ValueType::OBJ:
    {
      if (v.obj == nullptr) { return "nil"; }
      switch (v.obj->type)
      {
      case ObjType::STR:
        return fmt::format("\"{}\"", v.str->get_view());
      case ObjType::TUPLE:
      {
        std::string str = "(";
        auto s = v.tuple->get_span();
        for (auto& elem : s)
        {
          str += elem.to_string() + ", ";
        }
        str += ")";
        return str;
      }
      case ObjType::CLASS:
        return fmt::format("<class {}>", v.klass->get_name());
      case ObjType::INSTANCE:
        return fmt::format("<{} instance>", v.instance->get_class()->get_name());
      case ObjType::DICT:
        return "<dict>";
      case ObjType::ARRAY:
        return "<array>";
      default:
        throw FatalError(fmt::format("Unknown ObjType: {}", magic_enum::enum_name(v.obj->type)));
      }
    }
    default:
      throw FatalError(fmt::format("Unknown ValueType: {}", magic_enum::enum_name(type)));
    }
  }
  bool Value::debug_type_is_valid() noexcept
  {
    if (type == ValueType::OBJ)
    {
      // NIL, STR, TUPLE, CLASS, INSTANCE, DICT, ARRAY
      if (v.obj == nullptr)
      {
        return true;
      }
      return (v.obj->type == ObjType::STR)
        || (v.obj->type == ObjType::TUPLE)
        || (v.obj->type == ObjType::CLASS)
        || (v.obj->type == ObjType::INSTANCE)
        || (v.obj->type == ObjType::DICT);
    }
    if (type == ValueType::BOOL)
    {
      return (v.i64 == 0) || (v.i64 == 1);
    }
    if (type == ValueType::F64 || type == ValueType::I64)
    {
      return true;
    }
    if (type == ValueType::FUNC)
    {
      return (v.func != nullptr) && (reinterpret_cast<uintptr_t>(v.func) % alignof(decltype(*v.func)) == 0);
    }
    if (type == ValueType::CPP_FUNC)
    {
      return v.cppfunc != nullptr;
    }
    if (type == ValueType::METHOD)
    {
      return (v.instance != nullptr)
        && (reinterpret_cast<uintptr_t>(v.instance) % alignof(decltype(*v.instance)) == 0)
        && (method_func() != nullptr)
        && (reinterpret_cast<uintptr_t>(method_func()) % alignof(decltype(*method_func())) == 0);
    }
    return false;
  }
  Value Value::get_property(gsl::not_null<String*> name)
  {
    if (is_instance())
    {
      return v.instance->get_property(name);
    }
    if (is_dict())
    {
      return v.dict->get(name);
    }
    if (is_nil())
    {
      throw exception_wrongtype(ObjType::NIL, ObjType::INSTANCE, ObjType::DICT);
    }
    if (type != ValueType::OBJ)
    {
      throw exception_wrongtype(type, ObjType::INSTANCE, ObjType::DICT);
    }
    throw exception_wrongtype(v.obj->type, ObjType::INSTANCE, ObjType::DICT);
  }
}
