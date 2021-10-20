#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(assignment, associativity)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = (), a = "a", b = "b", c = "c";
# Assignment is right-associative.
a = b = c;
r += a; # expect: c
r += b; # expect: c
r += c; # expect: c
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 3);
  ASSERT_EQ(v[0], "c");
  ASSERT_EQ(v[1], "c");
  ASSERT_EQ(v[2], "c");
}

TEST(assignment, global)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = (), a = "before";
r += a; # expect: before

a = "after";
r += a; # expect: after

r += a = "arg"; # expect: arg
r += a; # expect: arg
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], "before");
  ASSERT_EQ(v[1], "after");
  ASSERT_EQ(v[2], "arg");
  ASSERT_EQ(v[3], "arg");
}

TEST(assignment, grouping)
{
  auto [res, chunk] = compile(R"(
var a = "a";
(a) = "value"; #
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(assignment, infix_operator)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var a = "a", b = "b";
a + b = "value";
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(assignment, local)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "before";
  r += a; # expect: before

  a = "after";
  r += a; # expect: after

  r += a = "arg"; # expect: arg
  r += a; # expect: arg
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], "before");
  ASSERT_EQ(v[1], "after");
  ASSERT_EQ(v[2], "arg");
  ASSERT_EQ(v[3], "arg");
}

TEST(assignment, prefix_operator)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var a = "a";
!a = "value";
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(assignment, syntax)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
# Assignment on RHS of variable.
var a = "before", c = a = "var";
r += a; # expect: var
r += c; # expect: var
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "var");
  ASSERT_EQ(v[1], "var");
}

TEST(assignment, to_this)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  Foo() {
    this = "value"; 
  }
}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(assignment, undefined)
{
  VM vm;
  auto [res, chunk] = compile(R"(
unknown = "what";
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}