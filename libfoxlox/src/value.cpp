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
    return std::move(msg);
  }

  template<typename T1, typename T2, typename ... Args>
  ValueError exception_wrongtype_binop(T1 got1, T2 got2, Args ... expected)
    requires (std::same_as<T1, ValueType> || std::same_as<T1, ObjType>) &&
    (std::same_as<T2, ValueType> || std::same_as<T2, ObjType>) &&
    ((std::same_as<Args, ValueType> || std::same_as<Args, ObjType>) && ...)
  {
    std::string msg = wrongtype_msg_fmt(expected...);
    fmt::format_to(std::back_inserter(msg), "got: {} and {}.", magic_enum::enum_name(got1), magic_enum::enum_name(got2));
    return ValueError(msg.c_str());
  }

  template<typename T, typename ... Args> 
  ValueError exception_wrongtype(T got, Args ... expected)
    requires (std::same_as<T, ValueType> || std::same_as<T, ObjType>) &&
    ((std::same_as<Args, ValueType> || std::same_as<Args, ObjType>) && ...)
  {
    std::string msg = wrongtype_msg_fmt(expected...);
    fmt::format_to(std::back_inserter(msg), "got: {}.", magic_enum::enum_name(got));
    return ValueError(msg.c_str());
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
  static_assert(sizeof(Value) == 16);

  bool Value::is_nil() const noexcept
  {
    return (type == ValueType::OBJ) && (v.obj == nullptr);
  }

  bool Value::is_str() const noexcept
  {
    return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::STR);
  }

  bool Value::is_tuple() const noexcept
  {
    return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::TUPLE);
  }

  bool Value::is_class() const noexcept
  {
    return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::CLASS);
  }

  inline bool Value::is_instance() const noexcept
  {
    return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::INSTANCE);
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

  bool Value::get_bool() const
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

  std::span<const Value> Value::get_tuplespan() const
  {
    type_check(*this, ObjType::TUPLE);
    return v.tuple->get_span();
  }

  Subroutine* Value::get_method_func() const
  {
    return reinterpret_cast<Subroutine*>(method_func << method_func_shift);
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
    if (l.is_class() && r.is_class())
    {
      return l.v.klass == r.v.klass ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
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
    if (l.is_str() && r.is_str())
    {
      return *l.v.str == *r.v.str;
    }
    return (l <=> r) == std::partial_ordering::equivalent;
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
      return fmt::format("<native fn {}>", static_cast<void*>(v.cppfunc));
    case ValueType::METHOD:
      return fmt::format("<class {} method {}>", v.instance->get_class()->get_name(), get_method_func()->get_funcname());
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
      default:
        throw FatalError(fmt::format("Unknown ObjType: {}", magic_enum::enum_name(v.obj->type)).c_str());
      }
    }
    default:
      throw FatalError(fmt::format("Unknown ValueType: {}", magic_enum::enum_name(type)).c_str());
    }
  } 
}