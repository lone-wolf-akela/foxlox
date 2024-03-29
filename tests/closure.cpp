#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(closure, assign_to_closure)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = (), f, g;
{
  var local = "local";
  fun f_() {
    r += local;
    local = "after f";
    r += local;
  }
  f = f_;

  fun g_() {
    r += local;
    local = "after g";
    r += local;
  }
  g = g_;
}

f();
# expect: local
# expect: after f

g();
# expect: after f
# expect: after g
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 4);
    ASSERT_EQ(v[0], "local");
    ASSERT_EQ(v[1], "after f");
    ASSERT_EQ(v[2], "after f");
    ASSERT_EQ(v[3], "after g");
  }
}

TEST(closure, assign_to_shadowed_later)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = (), a = "global";
{
  fun assign() {
    a = "assigned";
  }
  var a = "inner";
  assign();
  r += a; # expect: inner
}
r += a; # expect: assigned
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 2);
    ASSERT_EQ(v[0], "inner");
    ASSERT_EQ(v[1], "assigned");
  }
}

TEST(closure, close_over_function_parameter)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r, f;
fun foo(param) {
  fun f_() {
    r = param;
  }
  f = f_;
}
foo("param");
f(); # expect: param
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "param");
  }
}

TEST(closure, close_over_later_variable)
{
// FROM LOX AUTHOR munificent:
// This is a regression test. There was a bug where if an upvalue for an
// earlier local (here "a") was captured *after* a later one ("b"), then it
// would crash because it walked to the end of the upvalue list (correct), but
// then didn't handle not finding the variable.
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = ();
fun f() {
  var a = "a", b = "b";
  fun g() {
    r += b; # expect: b
    r += a; # expect: a
  }
  g();
}
f();
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 2);
    ASSERT_EQ(v[0], "b");
    ASSERT_EQ(v[1], "a");
  }
}

TEST(closure, close_over_method_parameter)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r, f;

class Foo {
  method(param) {
    fun f_() {
      r = param;
    }
    f = f_;
  }
}
Foo().method("param");
f(); # expect: param
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "param");
  }
}

TEST(closure, closed_closure_in_function)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r, f;
{
  var local = "local";
  fun f_() {
    r = local;
  }
  f = f_;
}
f(); # expect: local
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "local");
  }
}

TEST(closure, nested_closure)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = (), f;
fun f1() {
  var a = "a";
  fun f2() {
    var b = "b";
    fun f3() {
      var c = "c";
      fun f4() {
        r += a;
        r += b;
        r += c;
      }
      f = f4;
    }
    f3();
  }
  f2();
}
f1();
f();
# expect: a
# expect: b
# expect: c
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "b");
    ASSERT_EQ(v[2], "c");
  }
}

TEST(closure, open_closure_in_function)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r;
{
  var local = "local";
  fun f() {
    r = local; # expect: local
  }
  f();
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "local");
  }
}

TEST(closure, reference_closure_multiple_times)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = (), f;
{
  var a = "a";
  fun f_() {
    r += a;
    r += a;
  }
  f = f_;
}
f();
# expect: a
# expect: a
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 2);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "a");
  }
}

TEST(closure, reuse_closure_slot)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r;
{
  var f;
  {
    var a = "a";
    fun f_() { r = a; }
    f = f_;
  }
  {
    # COPIED FROM LOX. I GUESS THIS IS NOT TRUE FOR FOXLOX:
    # Since a is out of scope, the local slot will be reused by b. Make sure
    # that f still closes over a.
    var b = "b";
    f(); # expect: a
  }
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "a");
  }
}

TEST(closure, shadow_closure_with_local)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = ();
{
  var foo = "closure";
  fun f() {
    {
      r += foo; # expect: closure
      var foo = "shadow";
      r += foo; # expect: shadow
    }
    r += foo; # expect: closure
  }
  f();
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    ASSERT_EQ(v[0], "closure");
    ASSERT_EQ(v[1], "shadow");
    ASSERT_EQ(v[2], "closure");
  }
}

TEST(closure, unused_closure)
{
// FROM LOX AUTHOR munificent:
// This is a regression test. There was a bug where the VM would try to close
// an upvalue even if the upvalue was never created because the codepath for
// the closure was not executed.
  VM vm;
  {
    auto [res, chunk] = compile(R"(
{
  var a = "a";
  if (false) {
    fun foo() { a; }
  }
}
# If we get here, we didn't segfault when a went out of scope.
return "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "ok");
  }
}

TEST(closure, unused_later_closure)
{
// FROM LOX AUTHOR munificent:
// This is a regression test. When closing upvalues for discarded locals, it
// wouldn't make sure it discarded the upvalue for the correct stack slot.
//
// Here we create two locals that can be closed over, but only the first one
// actually is. When "b" goes out of scope, we need to make sure we don't
// prematurely close "a".
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var closure;
{
  var a = "a";
  {
    var b = "b";
    fun returnA() {
      return a;
    }
    closure = returnA;
    if (false) {
      fun returnB() {
        return b;
      }
    }
  }
  return closure(); # expect: a
}
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "a");
  }
}