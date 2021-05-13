#include <gtest/gtest.h>
import <numbers>;
import <fstream>;
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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], 7);
  ASSERT_EQ(v[1], -3);
}

TEST(import_, import_as)
{
  VM vm;
  auto [res, chunk] = compile(R"(
import fox.algorithm as algo;
return (algo.max(4,7,5,-3), algo.min(4,7,5,-3),);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], 7);
  ASSERT_EQ(v[1], -3);
}

TEST(import_, from_single)
{
  VM vm;
  auto [res, chunk] = compile(R"(
from fox.algorithm import min;
return min(4,7,5,-3);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, -3);
}

TEST(import_, from_multi)
{
  VM vm;
  auto [res, chunk] = compile(R"(
from fox.algorithm import min, max;
return (max(4,7,5,-3), min(4,7,5,-3),);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], 7);
  ASSERT_EQ(v[1], -3);
}

TEST(import_, value_)
{
  VM vm;
  auto [res, chunk] = compile(R"(
from fox.math import pi;
return pi;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, std::numbers::pi);
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 42);
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 58);
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 58);
  }
  std::filesystem::remove("exported.fox");
}