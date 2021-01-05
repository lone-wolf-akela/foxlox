#include "value.h"
namespace foxlox
{
  Value::Value(double f64)
  {
    type = F64;
    v.f64 = f64;
  }
  std::string Value::to_string() const
  {
    switch (type)
    {
    case F64:
      return fmt::format("{}", v.f64);
    default:
      assert(false);
      return "";
    }
  }
}