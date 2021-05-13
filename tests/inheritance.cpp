#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(inheritance, constructor)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class A {
  __init__(param) {
    this.field = param;
  }
  test() {
    return this.field;
  }
}
class B : A {}
var b = B("value");
return b.test();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "value");
}

TEST(inheritance, inherit_from_function)
{
  VM vm;
  auto [res, chunk] = compile(R"(
fun foo() {}
class Subclass : foo {}
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(inheritance, inherit_from_nil)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var Nil = nil;
class Foo : Nil {}
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(inheritance, inherit_from_number)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var Number = 123;
class Foo : Number {}
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(inheritance, inherit_methods)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {
  methodOnFoo() { return "foo"; }
  override() { return "foo"; }
}

class Bar : Foo {
  methodOnBar() { return "bar"; }
  override() { return "bar"; }
}
var bar = Bar();
r += bar.methodOnFoo();
r += bar.methodOnBar();
r += bar.override();
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 3);
  ASSERT_EQ(v[0], "foo");
  ASSERT_EQ(v[1], "bar");
  ASSERT_EQ(v[2], "bar");
}

TEST(inheritance, superclass_from_func)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  foo_value() {
    return "foo";
  }
}
fun get_foo() {
  return Foo;
} 
class Bar : get_foo() {}
var bar = Bar();
return bar.foo_value();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "foo");
}

TEST(inheritance, set_fields_from_base_class)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {
  foo(a, b) {
    this.field1 = a;
    this.field2 = b;
  }
  fooPrint() {
    r += this.field1;
    r += this.field2;
  }
}
class Bar : Foo {
  bar(a, b) {
    this.field1 = a;
    this.field2 = b;
  }

  barPrint() {
    r += this.field1;
    r += this.field2;
  }
}

var bar = Bar();
bar.foo("foo 1", "foo 2");
bar.fooPrint();
bar.bar("bar 1", "bar 2");
bar.barPrint();
bar.fooPrint();
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 6);
  ASSERT_EQ(v[0], "foo 1");
  ASSERT_EQ(v[1], "foo 2");
  ASSERT_EQ(v[2], "bar 1");
  ASSERT_EQ(v[3], "bar 2");
  ASSERT_EQ(v[4], "bar 1");
  ASSERT_EQ(v[5], "bar 2");
}