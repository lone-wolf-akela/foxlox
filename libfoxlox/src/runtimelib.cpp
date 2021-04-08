#include <iostream>
#include <chrono>
#include <ranges>
#include <algorithm>
#include <numbers>

#include <fmt/format.h>
#pragma warning(disable:4702) // unreachable code
#include <range/v3/all.hpp>
#pragma warning(default:4702)

#include <foxlox/vm.h>
#include "value.h"
#include "object.h"

#include "runtimelib.h"

namespace foxlox::runtimelib
{
  Value print(VM& /*vm*/, std::span<Value> values)
  {
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
  Value println(VM& vm, std::span<Value> values)
  {
    print(vm, values);
    std::cout << std::endl;
    return Value();
  }
  Value clock(VM& /*vm*/, [[maybe_unused]] std::span<Value> values)
  {
    if (values.size() != 0)
    {
      throw RuntimeLibError("[clock]: This function does not need any paramters.");
    }
    using namespace std::chrono;
    const auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return ms.count() / 1000.0;
  }
  Value max(VM& /*vm*/, [[maybe_unused]] std::span<Value> values)
  {
    return *std::ranges::max_element(values);
  }
  Value min(VM& /*vm*/, [[maybe_unused]] std::span<Value> values)
  {
    return *std::ranges::min_element(values);
  }
}

namespace foxlox
{
  std::vector<ExportedLibElem> find_lib(std::span<const std::string_view> libpath)
  {
    if (libpath.size() == 1)
    {
      if (libpath.front() == "io")
      {
        return { 
          {"print", runtimelib::print},
          {"println", runtimelib::println}
        };
      }
      if (libpath.front() == "profiler")
      {
        return { 
          {"clock", runtimelib::clock}
        };
      }
      if (libpath.front() == "algorithm")
      {
        return {
          {"max", runtimelib::max},
          {"min", runtimelib::min}
        };
      }
      if (libpath.front() == "math")
      {
        return {
          {"pi", std::numbers::pi}
        };
      }
    }
    const auto libpath_str = libpath
      | ranges::views::join('.')
      | ranges::to<std::string>;
    throw RuntimeLibError(fmt::format("Unknown runtime lib with name: {}", libpath_str).c_str());
  }
}