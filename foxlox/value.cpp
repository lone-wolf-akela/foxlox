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
  void Value::cast_double()
  {
    if (type == F64) { return; }
    assert(type == I64);
    type = F64;
    v.f64 = static_cast<double>(v.i64);
  }
  void Value::cast_int64()
  {
    if (type == I64) { return; }
    assert(type == F64);
    type = I64;
    v.i64 = static_cast<int64_t>(v.f64);
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
  Value Value::neg()
  {
    if (num_is_double(*this))
    {
      v.f64 = -v.f64;
    }
    else
    {
      v.i64 = -v.i64;
    }
    return *this;
  }
  Value Value::add(Value r)
  {
    if (num_have_double(*this, r))
    {
      cast_double();
      r.cast_double();
      v.f64 += r.v.f64;
    }
    else
    {
      v.i64 += r.v.i64;
    }
    return *this;
  }
  Value Value::sub(Value r)
  {
    if (num_have_double(*this, r))
    {
      cast_double();
      r.cast_double();
      v.f64 -= r.v.f64;
    }
    else
    {
      v.i64 -= r.v.i64;
    }
    return *this;
  }
  Value Value::mul(Value r)
  {
    if (num_have_double(*this, r))
    {
      cast_double();
      r.cast_double();
      v.f64 *= r.v.f64;
    }
    else
    {
      v.i64 *= r.v.i64;
    }
    return *this;
  }
  Value Value::div(Value r)
  {
    num_have_double(*this, r);
    cast_double();
    r.cast_double();
    v.f64 /= r.v.f64;
    return *this;
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
      // TODO
      assert(false);
      return std::partial_ordering::unordered;
    }
    return std::partial_ordering::unordered;
  }
  int64_t intdiv(const Value& l, const Value& r)
  {
    auto il = l.get_int64();
    auto ir = r.get_int64();
    return il / ir;
  }
  bool operator==(const Value& l, const Value& r)
  {
    return (l <=> r) == std::partial_ordering::equivalent;
  }

  std::string Value::to_string() const
  {
    switch (type)
    {
    case F64:
      return fmt::format("{}", v.f64);
    case I64:
      return fmt::format("{}", v.i64);
    default:
      assert(false);
      return "";
    }
  }
  bool num_is_double(Value v)
  {
    if (v.type == Value::F64)
    {
      return true;
    }
    assert(v.type == Value::I64);
    return false;
  }
}