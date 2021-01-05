#ifndef FOXLOX_VALUE_H
#define FOXLOX_VALUE_H

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace foxlox
{
  struct Value
  {
    enum : uint8_t
    {
      F64, I64, STR
    } type;
    union 
    {
      double f64;
      int64_t i64;
    } v;

    Value() = default;
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