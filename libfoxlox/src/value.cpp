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
  std::string wrongtype_msg_fmt(Args ... expected) requires (std::same_as<Args, Value::Type> && ...)
  {
    std::string msg = "Value type error. Expected: ";
    (fmt::format_to(std::back_inserter(msg), "{}, ", magic_enum::enum_name(expected)), ...);
    return std::move(msg);
  }

  template<typename ... Args> 
  ValueTypeError exception_wrongtype_binop(Value::Type got1, Value::Type got2, Args ... expected) requires (std::same_as<Args, Value::Type> && ...)
  {
    std::string msg = wrongtype_msg_fmt(expected...);
    fmt::format_to(std::back_inserter(msg), "got: {} and {}.", magic_enum::enum_name(got1), magic_enum::enum_name(got2));
    return ValueTypeError(msg.c_str());
  }

  template<typename ... Args> requires (std::same_as<Args, Value::Type> && ...)
  ValueTypeError exception_wrongtype(Value::Type got, Args ... expected)
  {
    std::string msg = wrongtype_msg_fmt(expected...);
    fmt::format_to(std::back_inserter(msg), "got: {}.", magic_enum::enum_name(got));
    return ValueTypeError(msg.c_str());
  }

  constexpr void type_check(Value::Type got, Value::Type expected)
  {
    if (got != expected)
    {
      throw exception_wrongtype(got, expected);
    }
  }
}

namespace foxlox
{
  double Value::get_double() const
  {
    if (type == F64) { return v.f64; }
    if (type != I64)
    {
      throw exception_wrongtype(type, I64, F64);
    }
    return static_cast<double>(v.i64);
  }
  int64_t Value::get_int64() const
  {
    if (type == I64) { return v.i64; }
    if (type != F64)
    {
      throw exception_wrongtype(type, I64, F64);
    }
    return static_cast<int64_t>(v.f64);
  }

  bool Value::get_bool() const
  {
    if (type == NIL ||
      (type == BOOL && v.b == false))
    {
      return false;
    }
    return true;
  }

  std::string_view Value::get_strview() const
  {
    type_check(type, STR);
    return v.str->get_view();
  }

  std::span<const Value> Value::get_tuplespan() const
  {
    type_check(type, TUPLE);
    return v.tuple->get_span();
  }

  std::partial_ordering operator<=>(const Value& l, const Value& r)
  {
    if (l.type == Value::NIL && r.type == Value::NIL)
    {
      return std::partial_ordering::equivalent;
    }
    if (l.type == Value::I64 && r.type == Value::I64)
    {
      return l.v.i64 <=> r.v.i64;
    }
    if ((l.type == Value::I64 || l.type == Value::F64) &&
      (r.type == Value::I64 || r.type == Value::F64))
    {
      return l.get_double() <=> r.get_double();
    }
    if (l.type == Value::BOOL && r.type == Value::BOOL)
    {
      return l.v.b <=> r.v.b;
    }
    if (l.type == Value::STR && r.type == Value::STR)
    {
      return *l.v.str <=> *r.v.str;
    }
    if (l.type == Value::TUPLE && r.type == Value::TUPLE)
    {
      return l.v.tuple == r.v.tuple ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.type == Value::FUNC && r.type == Value::FUNC)
    {
      return l.v.func == r.v.func ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    return std::partial_ordering::unordered;
  }
  double operator/(const Value& l, const Value& r)
  {
    if ((l.type == Value::I64 || l.type == Value::F64) &&
      (r.type == Value::I64 || r.type == Value::F64))
    {
      return l.get_double() / r.get_double();
    }
    throw exception_wrongtype_binop(l.type, r.type, Value::I64, Value::F64);
  }
  Value operator*(const Value& l, const Value& r)
  {
    if (l.type == Value::I64 && r.type == Value::I64)
    {
      return Value(l.v.i64 * r.v.i64);
    }
    if ((l.type == Value::I64 || l.type == Value::F64) &&
      (r.type == Value::I64 || r.type == Value::F64))
    {
      return Value(l.get_double() * r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, Value::I64, Value::F64);
  }
  Value operator+(const Value& l, const Value& r)
  {
    if (l.type == Value::I64 && r.type == Value::I64)
    {
      return Value(l.v.i64 + r.v.i64);
    }
    if ((l.type == Value::I64 || l.type == Value::F64) &&
      (r.type == Value::I64 || r.type == Value::F64))
    {
      return Value(l.get_double() + r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, Value::I64, Value::F64);
  }
  Value operator-(const Value& l, const Value& r)
  {
    if (l.type == Value::I64 && r.type == Value::I64)
    {
      return Value(l.v.i64 - r.v.i64);
    }
    if ((l.type == Value::I64 || l.type == Value::F64) &&
      (r.type == Value::I64 || r.type == Value::F64))
    {
      return Value(l.get_double() - r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, Value::I64, Value::F64);
  }
  Value operator-(const Value& val)
  {
    if (val.type == Value::F64) { return Value(-val.v.f64); }
    if (val.type != Value::I64)
    {
      throw exception_wrongtype(val.type, Value::I64, Value::F64);
    }
    return Value(-val.v.i64);
  }
  bool operator!(const Value& val)
  {
    type_check(val.type, Value::BOOL);
    return !val.v.b;
  }
  int64_t intdiv(const Value& l, const Value& r)
  {
    if (l.type == Value::I64 && r.type == Value::I64)
    {
      return l.v.i64 / r.v.i64;
    }
    return static_cast<int64_t>(l.get_double() / r.get_double());
  }
  bool operator==(const Value& l, const Value& r)
  {
    if (l.type == Value::STR && r.type == Value::STR)
    {
      return *l.v.str == *r.v.str;
    }
    return (l <=> r) == std::partial_ordering::equivalent;
  }

  std::string Value::to_string() const
  {
    
    switch (type)
    {
    case NIL:
      return "nil";
    case BOOL:
      return v.b ? "true" : "false";
    case F64:
      return fmt::format("{}", v.f64);
    case I64:
      return fmt::format("{}", v.i64);
    case STR:
      return fmt::format("\"{}\"", v.str->get_view());
    case TUPLE:
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
    case FUNC:
      return fmt::format("<fn {}>", v.func->get_funcname());
    case CPP_FUNC:
      return fmt::format("<native fn {}>", static_cast<void*>(v.cppfunc));
    default:
      throw FatalError(fmt::format("Unknown ValueType: {}", magic_enum::enum_name(type)).c_str());
    }
  } 
}