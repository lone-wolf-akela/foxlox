#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(this_, closure)
{
  auto [res, chunk] = compile(R"CODE(
class Foo {
  getClosure() {
    fun closure() {
      return this.toString();
    }
    return closure;
  }
  toString() { return "Foo"; }
}
var closure = Foo().getClosure();
return closure();
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(this_, nested_class)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r = ();
class Outer {
  __init__() {
    this.v = "Outer";
  }
  method() {
    r += this.v;
    class Inner {
      __init__() {
        this.v = "Inner";
      }
      method() {
        r += this.v;
      }
    }
    var in = Inner();
    in.method();
    return in.method;
  }
}

Outer().method()();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 3);
  ASSERT_EQ(v[0], "Outer");
  ASSERT_EQ(v[1], "Inner");
  ASSERT_EQ(v[2], "Inner");
}

TEST(this_, nested_closure)
{
  auto [res, chunk] = compile(R"CODE(
class Foo {
  getClosure() {
    fun f() {
      fun g() {
        fun h() {
          return this.toString();
        }
        return h;
      }
      return g;
    }
    return f;
  }
  toString() { return "Foo"; }
}
var closure = Foo().getClosure();
closure()()();
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(this_, this_at_top_level)
{
  auto [res, chunk] = compile(R"CODE(
this;
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(this_, this_in_method)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
class Foo {
  bar() { return this; }
  baz() { return "baz"; }
}
return Foo().bar().baz();
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "baz");
}

TEST(this_, this_in_top_level_function)
{
  auto [res, chunk] = compile(R"CODE(
fun foo() {
  this;
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}