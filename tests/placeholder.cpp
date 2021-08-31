#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(placeholder, basic_assign)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var str = "string";
_ = str;
return str;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "string");
}

TEST(placeholder, basic_declare)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var str = "string";
var _ = str;
return str;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "string");
}

TEST(placeholder, chain_assign)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var str;
str = _ = "string";
return str;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "string");
}

TEST(placeholder, tuple_unpack)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var a;
var b;
(a, _, b) = (1, 2, 3);
return (a, b);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], 1);
  ASSERT_EQ(v[1], 3);
}

TEST(placeholder, tuple_unpack_chained)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var a;
var b;
var c;
(a, b, c) = (a, _, b) = (1, 2, 3);
return (a, b, c);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 3);
  ASSERT_EQ(v[0], 1);
  ASSERT_EQ(v[1], 2);
  ASSERT_EQ(v[2], 3);
}

TEST(placeholder, assigned_from)
{
  auto [res, chunk] = compile(R"(
var _ = "placeholder";
var v = _;
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(placeholder, construct_tuple)
{
  auto [res, chunk] = compile(R"(
(1, 2, _);
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(placeholder, op)
{
  {
    auto [res, chunk] = compile(R"(
1 + _;
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
_ + 1;
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
_ == 1;
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
1 == _;
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
_ != 1;
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
1 != _;
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
_ >= 1;
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
1 >= _;
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
}

TEST(placeholder, as_func_params)
{
  auto [res, chunk] = compile(R"(
fun foo(a, b, c) {
# do nothing
}
foo(1, _, 3);
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}