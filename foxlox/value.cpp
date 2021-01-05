#include "value.h"
namespace foxlox
{
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
  Value Value::intdiv(Value r)
  {
    num_have_double(*this, r);
    cast_int64();
    r.cast_int64();
    v.i64 /= r.v.i64;
    return *this;
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