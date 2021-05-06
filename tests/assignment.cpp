#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

#include <gtest/gtest.h>

using namespace foxlox;

TEST(assignment, associativity)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
var a = "a";
var b = "b";
var c = "c";
# Assignment is right-associative.
a = b = c;
r += a; # expect: c
r += b; # expect: c
r += c; # expect: c
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 3);
  ASSERT_EQ(to_variant(s[0]), FoxValue("c"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("c"));
  ASSERT_EQ(to_variant(s[2]), FoxValue("c"));
}

TEST(assignment, global)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
var a = "before";
r += a; # expect: before

a = "after";
r += a; # expect: after

r += a = "arg"; # expect: arg
r += a; # expect: arg
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 4);
  ASSERT_EQ(to_variant(s[0]), FoxValue("before"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("after"));
  ASSERT_EQ(to_variant(s[2]), FoxValue("arg"));
  ASSERT_EQ(to_variant(s[3]), FoxValue("arg"));
}

TEST(assignment, grouping)
{
  VM vm;
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
var a = "a";
var b = "b";
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 4);
  ASSERT_EQ(to_variant(s[0]), FoxValue("before"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("after"));
  ASSERT_EQ(to_variant(s[2]), FoxValue("arg"));
  ASSERT_EQ(to_variant(s[3]), FoxValue("arg"));
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
var a = "before";
var c = a = "var";
r += a; # expect: var
r += c; # expect: var
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 2);
  ASSERT_EQ(to_variant(s[0]), FoxValue("var"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("var"));
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