#ifndef FOXLOC_VALUE_H
#define FOXLOC_VALUE_H

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "util.h"

namespace foxlox
{
  struct Value
  {
    enum : uint8_t
    {
      F64
    } type;
    union 
    {
      double f64;
    } v;

    Value(double f64);
    std::string to_string() const;
  };
  using ValueArray = std::vector<Value>;
}

#endif // FOXLOC_VALUE_H