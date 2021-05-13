#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(constructor, arguments)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {
  __init__(a, b) {
    r += "init"; # expect: init
    this.a = a;
    this.b = b;
  }
}
var foo = Foo(1, 2);
r += foo.a; # expect: 1
r += foo.b; # expect: 2
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 3);
  ASSERT_EQ(v[0], "init");
  ASSERT_EQ(v[1], 1);
  ASSERT_EQ(v[2], 2);
}

TEST(constructor, early_return)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r;
class Foo {
  __init__() {
    r = "init";
    return;
    r = "nope";
  }
}
var foo = Foo(); # expect: init
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "init");
}

TEST(constructor, call_init_explicitly)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
class Foo {
  __init__() {
  }
}
var foo = Foo();
foo.__init__();
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
class Foo {
  __init__() {
  }
  method() {
    this.__init__();
  }
}
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"(
class Foo {
  __init__() {
    this.__init__();
  }
}
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
}

TEST(constructor, default_)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {}
class Bar {
  method() {
    return "method";
  }
}
var foo = Foo(); 
return Bar().method();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "method");
}

TEST(constructor, default_arguments)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {}
var foo = Foo(1, 2, 3);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(constructor, extra_arguments)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  __init__(a, b) {
    this.a = a;
    this.b = b;
  }
}
var foo = Foo(1, 2, 3, 4); 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(constructor, init_not_method)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r;
class Foo {
  __init__(arg) {
    r = "class_" + arg;
  }
}
fun __init__(arg) {
  r = "non_class_" + arg;
}
__init__("func");
return  r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "non_class_func");
}

TEST(constructor, missing_arguments)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  __init__(a, b) {
    this.a = a;
    this.b = b;
  }
}
var foo = Foo(1); 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(constructor, return_in_nested_function)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {
  __init__() {
    fun __init__() {
      return "bar";
    }
    r += __init__(); # expect: bar
  }
  method() {
    return "method";
  }
}
var r2 = Foo().method(); # note: we can not directly do `r += ...'
                         # due to the load of left side `r' happens 
                         # before the call to the right.
r += r2; # expect: method
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "bar");
  ASSERT_EQ(v[1], "method");
}

TEST(constructor, return_value)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  __init__() {
    return "result"; 
  }
}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}