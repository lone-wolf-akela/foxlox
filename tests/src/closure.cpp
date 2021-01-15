#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

using namespace foxlox;

TEST(closure, assign_to_closure)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = ();
var f;
var g;
{
  var local = "local";
  fun f_() {
    r = r + local;
    local = "after f";
    r = r + local;
  }
  f = f_;

  fun g_() {
    r = r + local;
    local = "after g";
    r = r + local;
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 4);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "local");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "after f");
    ASSERT_EQ(s[2].type, Value::STR);
    ASSERT_EQ(s[2].get_strview(), "after f");
    ASSERT_EQ(s[3].type, Value::STR);
    ASSERT_EQ(s[3].get_strview(), "after g");
  }
}

TEST(closure, assign_to_shadowed_later)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = ();
var a = "global";
{
  fun assign() {
    a = "assigned";
  }
  var a = "inner";
  assign();
  r = r + a; # expect: inner
}
r = r + a; # expect: assigned
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "inner");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "assigned");
  }
}

TEST(closure, close_over_function_parameter)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r;
var f;
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "param");
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
  var a = "a";
  var b = "b";
  fun g() {
    r = r + b; # expect: b
    r = r + a; # expect: a
  }
  g();
}
f();
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "b");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "a");
  }
}

TEST(closure, closed_closure_in_function)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r;
var f;
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "local");
  }
}

TEST(closure, nested_closure)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = ();
var f;
fun f1() {
  var a = "a";
  fun f2() {
    var b = "b";
    fun f3() {
      var c = "c";
      fun f4() {
        r = r + a;
        r = r + b;
        r = r + c;
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_EQ(s[2].type, Value::STR);
    ASSERT_EQ(s[2].get_strview(), "c");
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "local");
  }
}

TEST(closure, reference_closure_multiple_times)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = ();
var f;
{
  var a = "a";
  fun f_() {
    r = r + a;
    r = r + a;
  }
  f = f_;
}
f();
# expect: a
# expect: a
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "a");
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "a");
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
      r = r + foo; # expect: closure
      var foo = "shadow";
      r = r + foo; # expect: shadow
    }
    r = r + foo; # expect: closure
  }
  f();
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "closure");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "shadow");
    ASSERT_EQ(s[2].type, Value::STR);
    ASSERT_EQ(s[2].get_strview(), "closure");
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "ok");
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "a");
  }
}