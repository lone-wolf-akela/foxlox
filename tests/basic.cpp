#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(basic, empty_file)
{
  VM vm;
  auto [res, chunk] = compile(R"()");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, nil);
}

TEST(basic, precedence)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
# * has higher precedence than +.
r +=  2 + 3 * 4; # expect: 14

# * has higher precedence than -.
r +=  20 - 3 * 4; # expect: 8

# / has higher precedence than +.
r +=  2 + 6 / 3; # expect: 4.0

# / has higher precedence than -.
r +=  2 - 6 / 3; # expect: 0.0

# // has higher precedence than +.
r +=  2 + 6 // 3; # expect: 4

# // has higher precedence than -.
r +=  2 - 6 // 3; # expect: 0

# < has higher precedence than ==.

r +=  false == 2 < 1; # expect: true

# > has higher precedence than ==.
r +=  false == 1 > 2; # expect: true

# <= has higher precedence than ==.
r +=  false == 2 <= 1; # expect: true

# >= has higher precedence than ==.
r +=  false == 1 >= 2; # expect: true

# 1 - 1 is not space-sensitive.
r +=  1 - 1; # expect: 0
r +=  1 -1;  # expect: 0
r +=  1- 1;  # expect: 0
r +=  1-1;   # expect: 0

# Using () for grouping.
r +=  (2 * (6 - (2 + 2))); # expect: 4

return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 15);
  ASSERT_EQ(v[0], 14_i64);
  ASSERT_EQ(v[1], 8_i64);
  ASSERT_EQ(v[2], 4.0);
  ASSERT_EQ(v[3], 0.0);
  ASSERT_EQ(v[4], 4_i64);
  ASSERT_EQ(v[5], 0_i64);
  ASSERT_EQ(v[6], true);
  ASSERT_EQ(v[7], true);
  ASSERT_EQ(v[8], true);
  ASSERT_EQ(v[9], true);
  ASSERT_EQ(v[10], 0_i64);
  ASSERT_EQ(v[11], 0_i64);
  ASSERT_EQ(v[12], 0_i64);
  ASSERT_EQ(v[13], 0_i64);
  ASSERT_EQ(v[14], 4_i64);
}

TEST(basic, unexpected_character)
{
  auto [res, chunk] = compile(R"(foo(1 | 1);)");
  std::ignore = chunk;
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}