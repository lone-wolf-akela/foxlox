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
    if (values.size() == 1)
    {
      std::cout << values.front().to_string();
    }
    else
    {
      assert(values.size() >= 1);
      assert(values.front().type == Value::STR);
      fmt::dynamic_format_arg_store<fmt::format_context> store;
      for (auto& v : values | std::ranges::views::drop(1))
      {
        store.push_back(v.to_string());
      }
      std::cout << fmt::vformat(values.front().v.str->get_view(), store);
    }
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