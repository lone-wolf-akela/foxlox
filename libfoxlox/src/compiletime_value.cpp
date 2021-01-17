#include <cassert>

#include "util.h"

#include "compiletime_value.h"

namespace foxlox
{
  CompiletimeValue::CompiletimeValue()
  {
    v = nullptr;
  }
  CompiletimeValue::CompiletimeValue(double f64)
  {
    v = f64;
  }
  CompiletimeValue::CompiletimeValue(int64_t i64)
  {
    v = i64;
  }
  CompiletimeValue::CompiletimeValue(std::string_view str)
  {
    v = std::string(str);
  }
  CompiletimeValue::CompiletimeValue(bool b)
  {
    v = b;
  }
  CompiletimeValue::CompiletimeValue(CppFunc* cppfunc)
  {
    v = cppfunc;
  }
  std::string CompiletimeValue::to_string() const
  {
    if (std::holds_alternative<nullptr_t>(v))
    {
      return "nil";
    }
    else if (std::holds_alternative<double>(v))
    {
      return num_to_str(std::get<double>(v));
    }
    else if (std::holds_alternative<int64_t>(v))
    {
     return num_to_str(std::get<int64_t>(v));
    }
    else if (std::holds_alternative<std::string>(v))
    {
      return std::get<std::string>(v);
    }
    else if (std::holds_alternative<bool>(v))
    {
      return std::get<bool>(v) ? "true" : "false";
    }
    assert(false);
    return "";
  }
}