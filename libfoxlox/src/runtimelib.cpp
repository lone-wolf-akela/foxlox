#include <iostream>
#include <ranges>
#include <chrono>

#include <fmt/format.h>

#include <foxlox/vm.h>
#include "value.h"
#include "object.h"

#include "runtimelib.h"

namespace
{
  using namespace foxlox;
  Value print(VM& /*vm*/, std::span<Value> values)
  {
    assert(values.size() >= 1);
    bool multi_args = (values.size()) > 1;
    assert(!multi_args || values.front().is_str());

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
    assert(values.size() == 0);
    using namespace std::chrono;
    const auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return double(ms.count()) / 1000.;
  }
}

namespace foxlox
{
  void register_lib(Parser& parser)
  {
    parser.define("print", print);
    parser.define("println", println);
    parser.define("clock", clock);
  }
}