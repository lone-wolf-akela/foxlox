import <tuple>;
import <gtest/gtest.h>;
import foxlox;

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

TEST(import_, value_)
{
  VM vm;
  auto [res, chunk] = compile(R"(
from fox.math import pi;
return pi;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue(std::numbers::pi));
}

TEST(import_, export_int)
{
  {
    std::ofstream ofs("exported.fox");
    ASSERT_TRUE(!!ofs);
    ofs << R"(
export var Ultimate_Answer = 42;
)";
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
from exported import Ultimate_Answer;
return Ultimate_Answer;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(42_i64));
  }
  std::filesystem::remove("exported.fox");
}

TEST(import_, export_func)
{
  {
    std::ofstream ofs("exported.fox");
    ASSERT_TRUE(!!ofs);
    ofs << R"(
export fun Ultimate_Answer(in)
{
  return in - 42;
}
)";
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
from exported import Ultimate_Answer;
return Ultimate_Answer(100);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(58_i64));
  }
  std::filesystem::remove("exported.fox");
}

TEST(import_, export_class)
{
  {
    std::ofstream ofs("exported.fox");
    ASSERT_TRUE(!!ofs);
    ofs << R"(
export class Ultimate_Answer
{
  __init__(first)
  {
    this.first = first;
  }
  calc(second)
  {
    return second - this.first;
  }
}
)";
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
from exported import Ultimate_Answer;
var k = Ultimate_Answer(42);
return k.calc(100);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(58_i64));
  }
  std::filesystem::remove("exported.fox");
}