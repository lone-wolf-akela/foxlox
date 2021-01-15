#include <iostream>
#include <ranges>
#include <chrono>

#include <fmt/format.h>

#include <foxlox/vm.h>

#include "scanner.h"
#include "parser.h"
#include "codegen.h"
#include "resolver.h"

#include <foxlox/compiler.h>

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
  std::tuple<CompilerResult, Chunk> compile(std::string_view source)
  {
    Scanner scanner(u8_to_u32(source));
    auto [tokens, src_per_line] = scanner.scan_tokens();

    Parser parser(std::move(tokens));

    parser.define("print", print);
    parser.define("println", println);
    parser.define("clock", clock);

    auto ast = parser.parse();
    if (parser.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, Chunk());
    }

    Resolver resolver(std::move(ast));
    auto resolved_ast = resolver.resolve();
    if (resolver.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, Chunk());
    }

    CodeGen codegen(std::move(resolved_ast));
    auto chunk = codegen.gen();
    if (codegen.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, Chunk());
    }

    chunk.set_source(std::move(src_per_line));
    return std::make_tuple(CompilerResult::OK, std::move(chunk));
  }
}