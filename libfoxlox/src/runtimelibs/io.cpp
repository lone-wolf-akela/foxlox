#include <iostream>
#include <ranges>

#include <fmt/format.h>

#include "io.h"

namespace foxlox::lib
{
  FOXLOX_LIB_FUN(print)
  {
    std::ignore = vm;
    if (values.empty())
    {
      throw RuntimeLibError("[print]: Requires at least one parameter to print.");
    }
    const bool multi_args = (values.size()) > 1;
    if (multi_args && !values.front().is_str())
    {
      throw RuntimeLibError("[print]: When print multiple values, the first parameter must be a format string.");
    }

    fmt::dynamic_format_arg_store<fmt::format_context> store;
    for (auto& v : values | std::ranges::views::drop(multi_args ? 1 : 0))
    {
      switch (v.type)
      {
      case ValueType::I64:
        store.push_back(v.v.i64);
        break;
      case ValueType::F64:
        store.push_back(v.v.f64);
        break;
      case ValueType::BOOL:
        store.push_back(v.v.b);
        break;
      default:
        if (v.is_str())
        {
          store.push_back(v.v.str->get_view());
        }
        else
        {
          store.push_back(v.to_string());
        }
        break;
      }
    }
    std::cout << fmt::vformat(multi_args ? values.front().v.str->get_view() : "{}", store);
    return Value();
  }
  FOXLOX_LIB_FUN(println)
  {
    print(vm, values);
    std::cout << std::endl;
    return Value();
  }
}