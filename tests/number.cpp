#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(number, literals)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += 123;
r += 987654;
r += 0;
r += -0;
r += 123.456;
r += -0.001;
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 6);
  ASSERT_EQ(v[0], 123);
  ASSERT_EQ(v[1], 987654);
  ASSERT_EQ(v[2], 0);
  ASSERT_EQ(v[3], 0);
  ASSERT_EQ(v[4], 123.456);
  ASSERT_EQ(v[5], -0.001);
}

TEST(number, nan_equality)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
var nan = 0.0/0.0;
r += nan == 0;
r += nan != 1; 
# NaN is not equal to self.
r += nan == nan;
r += nan != nan; 
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], false);
  ASSERT_EQ(v[1], true);
  ASSERT_EQ(v[2], false);
  ASSERT_EQ(v[3], true);
}

TEST(number, trailing_dot)
{
  VM vm;
  auto [res, chunk] = compile(R"(
123.;
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(number, leading_dot)
{
  VM vm;
  auto [res, chunk] = compile(R"(
.123;
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(number, decimal_point_at_eof)
{
  VM vm;
  auto [res, chunk] = compile(R"(123.)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}