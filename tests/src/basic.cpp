#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

using namespace foxlox;

TEST(basic, empty_file)
{
  VM vm;
  auto [res, chunk] = compile(R"()");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.interpret(chunk));
  ASSERT_EQ(v, FoxValue(nil));
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
  auto v = to_variant(vm.interpret(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 15);
  int i = 0;
  ASSERT_EQ(to_variant(s[i++]), FoxValue(14_i64));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(8_i64));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(4.0));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(0.0));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(4_i64));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(0_i64));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(true));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(true));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(true));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(true));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(0_i64));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(0_i64));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(0_i64));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(0_i64));
  ASSERT_EQ(to_variant(s[i++]), FoxValue(4_i64));
}

TEST(basic, unexpected_character)
{
  auto [res, chunk] = compile(R"(foo(1 | 1);)");
  std::ignore = chunk;
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}