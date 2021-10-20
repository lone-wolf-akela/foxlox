#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(variable, in_nested_block)
{
  auto [res, chunk] = compile(R"(
{
  var a = "outer";
  {
    return a;
  }
}
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "outer");
}

TEST(variable, in_middle_of_block)
{
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "a";
  r += a; # expect: "a"
  var b = a + " b";
  r += b; # expect: "a b"
  var c = a + " c";
  r += c; # expect: "a c"
  var d = b + " d";
  r += d; # expect: "a b d"
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], "a");
  ASSERT_EQ(v[1], "a b");
  ASSERT_EQ(v[2], "a c");
  ASSERT_EQ(v[3], "a b d");
}

TEST(variable, scope_reuse_in_different_blocks)
{
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "first";
  r += a; # expect: first
}
{
  var a = "second";
  r += a; # expect: second
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "first");
  ASSERT_EQ(v[1], "second");
}

TEST(variable, shadow_and_local)
{
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "outer";
  {
    r += a; # expect: outer
    var a = "inner";
    r += a; # expect: inner
  }
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);

  ASSERT_EQ(v[0], "outer");
  ASSERT_EQ(v[1], "inner");
}

TEST(variable, shadow_global)
{
  auto [res, chunk] = compile(R"(
var r = (), a = "global";
{
  var a = "shadow";
  r += a; # expect: shadow
}
r += a; # expect: global
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "shadow");
  ASSERT_EQ(v[1], "global");
}

TEST(variable, shadow_local)
{
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "local";
  {
    var a = "shadow";
    r += a; # expect: shadow
  }
  r += a; # expect: local
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "shadow");
  ASSERT_EQ(v[1], "local");
}

TEST(variable, uninitialized)
{
  auto [res, chunk] = compile(R"(
var a;
return a;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, nil);
}

TEST(variable, collide_with_parameter)
{
  auto [res, chunk] = compile(R"CODE(
fun foo(a) {
  var a;
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, duplicate_local)
{
  auto [res, chunk] = compile(R"CODE(
{
  var a = "value", a = "other";
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, duplicate_global)
{
  auto [res, chunk] = compile(R"CODE(
var a = "value", a;
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, duplicate_parameter)
{
  auto [res, chunk] = compile(R"CODE(
fun foo(arg, arg) {
  "body";
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, undefined_global)
{
  auto [res, chunk] = compile(R"CODE(
notDefined;
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, undefined_local)
{
  auto [res, chunk] = compile(R"CODE(
{
  print notDefined;
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, unreached_undefined)
{
  auto [res, chunk] = compile(R"CODE(
if (false) {
  notDefined;
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, use_false_as_var)
{
  auto [res, chunk] = compile(R"CODE(
var false = "value";
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, use_local_in_initializer)
{
  auto [res, chunk] = compile(R"CODE(
var a = "outer";
{
  var a = a;
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, use_global_in_initializer)
{
  auto [res, chunk] = compile(R"CODE(
var a = a;
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, use_nil_as_var)
{
  auto [res, chunk] = compile(R"CODE(
var nil = "value";
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, use_this_as_var)
{
  auto [res, chunk] = compile(R"CODE(
var this = "value";
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(variable, early_bound)
{
  auto [res, chunk] = compile(R"(
var r = (), a = "outer";
{
  fun foo() {
    r += a;
  }
  foo();
  var a = "inner";
  foo();
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "outer");
  ASSERT_EQ(v[1], "outer");
}

TEST(variable, local_from_method)
{
  auto [res, chunk] = compile(R"(
var foo = "variable";
class Foo {
  method() {
    return foo;
  }
}
return Foo().method();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "variable");
}

TEST(variable, declare_multi_vars)
{
  auto [res, chunk] = compile(R"(
var a = "a", b, c, d = "d";
c = "c";
return (a, b, c, d);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], "a");
  ASSERT_EQ(v[1], nil);
  ASSERT_EQ(v[2], "c");
  ASSERT_EQ(v[3], "d");
}

TEST(variable, declare_multi_and_use_in_same_stmt)
{
  auto [res, chunk] = compile(R"(
var a = 0, b = a + 1, c = b + 1, d = a + 1;
return (a, b, c, d);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], 0);
  ASSERT_EQ(v[1], 1);
  ASSERT_EQ(v[2], 2);
  ASSERT_EQ(v[3], 1);
}

TEST(variable, declare_multi_and_use_before_decl)
{
  auto [res, chunk] = compile(R"(
var a = 0, b = c + 1, c = 1;
return (a, b, c);
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}