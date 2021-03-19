#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

using namespace foxlox;

TEST(import_, import_)
{
  VM vm;
  auto [res, chunk] = compile(R"(
import fox.algorithm;
return (algorithm.max(4,7,5,-3), algorithm.min(4,7,5,-3),);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 2);
  ASSERT_EQ(to_variant(s[0]), FoxValue(7_i64));
  ASSERT_EQ(to_variant(s[1]), FoxValue(-3_i64));
}

TEST(import_, import_as)
{
  VM vm;
  auto [res, chunk] = compile(R"(
import fox.algorithm as algo;
return (algo.max(4,7,5,-3), algo.min(4,7,5,-3),);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 2);
  ASSERT_EQ(to_variant(s[0]), FoxValue(7_i64));
  ASSERT_EQ(to_variant(s[1]), FoxValue(-3_i64));
}

TEST(import_, from_single)
{
  VM vm;
  auto [res, chunk] = compile(R"(
from fox.algorithm import min;
return min(4,7,5,-3);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue(-3_i64));
}

TEST(import_, from_multi)
{
  VM vm;
  auto [res, chunk] = compile(R"(
from fox.algorithm import min, max;
return (max(4,7,5,-3), min(4,7,5,-3),);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 2);
  ASSERT_EQ(to_variant(s[0]), FoxValue(7_i64));
  ASSERT_EQ(to_variant(s[1]), FoxValue(-3_i64));
}

