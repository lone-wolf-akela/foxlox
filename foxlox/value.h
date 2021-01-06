#ifndef FOXLOX_VALUE_H
#define FOXLOX_VALUE_H

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <compare>

#include <fmt/format.h>

namespace foxlox
{
  struct Value
  {
    enum : uint8_t
    {
      NIL, BOOL, F64, I64, STR,
    } type;
    union 
    {
      bool b;
      double f64;
      int64_t i64;
    } v;

    Value();
    Value(bool b);
    Value(double f64);
    Value(int64_t i64);

    void cast_double();
    void cast_int64();

    Value neg();
    Value add(Value r);
    Value sub(Value r);
    Value mul(Value r);
    Value div(Value r);
    Value intdiv(Value r);

    std::partial_ordering operator<=>(Value& r);

    std::string to_string() const;
  };
  using ValueArray = std::vector<Value>;

  bool num_is_double(Value v);

  template<typename ... Args>
  bool num_have_double(Args ... args)
  {
    return (num_is_double(args) || ...);
  }
}

#endif // FOXLOX_VALUE_H