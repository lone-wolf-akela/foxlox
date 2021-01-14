#include <gsl/gsl>
#include <fmt/format.h>
#include <magic_enum.hpp>

#include <foxexcept.h>

#include "value.h"
namespace foxlox
{
  Value::Value()
  {
    type = NIL;
  }
  Value::Value(String* str)
  {
    type = STR;
    v.str = str;
  }
  Value::Value(Tuple* tuple)
  {
    type = TUPLE;
    v.tuple = tuple;
  }
  Value::Value(bool b)
  {
    type = BOOL;
    v.b = b;
  }
  Value::Value(double f64)
  {
    type = F64;
    v.f64 = f64;
  }
  Value::Value(int64_t i64)
  {
    type = I64;
    v.i64 = i64;
  }
  double Value::get_double() const
  {
    if (type == F64) { return v.f64; }
    assert(type == I64);
    return static_cast<double>(v.i64);
  }
  int64_t Value::get_int64() const
  {
    if (type == I64) { return v.i64; }
    assert(type == F64);
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
    assert(type == STR);
    return v.str->get_view();
  }

  std::span<const Value> Value::get_tuplespan() const
  {
    assert(type == TUPLE);
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
      return l.v.tuple <=> r.v.tuple;
    }
    return std::partial_ordering::unordered;
  }
  String::TStrViewComp operator<=>(const String& l, const String& r)
  {
#if __cpp_lib_three_way_comparison >= 201907L
#pragma message("We get __cpp_lib_three_way_comparison support!")
    return l.get_view() <=> r.get_view();
#else
#pragma message("No __cpp_lib_three_way_comparison support!")
    const auto res = l.get_view().compare(r.get_view());
    if (res < 0) { return std::weak_ordering::less; }
    if (res == 0) { return std::weak_ordering::equivalent; }
    return std::weak_ordering::greater;
#endif
  }
  bool operator==(const String& l, const String& r)
  {
    return l.get_view() == r.get_view();
  }
  double operator/(const Value& l, const Value& r)
  {
    if ((l.type == Value::I64 || l.type == Value::F64) &&
      (r.type == Value::I64 || r.type == Value::F64))
    {
      return l.get_double() / r.get_double();
    }
    assert(false);
    return {};
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
    assert(false);
    return {};
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
    assert(false);
    return {};
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
    assert(false);
    return {};
  }
  Value operator-(const Value& val)
  {
    if (val.type == Value::F64) { return Value(-val.v.f64); }
    assert(val.type == Value::I64);
    return Value(-val.v.i64);
  }
  bool operator!(const Value& val)
  {
    assert(val.type == Value::BOOL);
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
    default:
      throw FatalError(fmt::format("Unknown ValueType: {}", magic_enum::enum_name(type)).c_str());
    }
  }
  std::string_view String::get_view() const
  {
    return std::string_view(str, length);
  }
  std::span<const Value> Tuple::get_span() const
  {
    return std::span{ elems, length };
  }
}