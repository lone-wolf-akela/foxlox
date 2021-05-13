#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(class_, empty)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {}
return Foo;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<Class*>());
}

TEST(class_, inherit_self)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo : Foo {}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(class_, inherited_method)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {
  inFoo() {
    r += "in foo";
  }
}
class Bar : Foo {
  inBar() {
    r += "in bar";
  }
}
class Baz : Bar {
  inBaz() {
    r += "in baz";
  }
}
var baz = Baz();
baz.inFoo(); # expect: in foo
baz.inBar(); # expect: in bar
baz.inBaz(); # expect: in baz
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 3);
  ASSERT_EQ(v[0], "in foo");
  ASSERT_EQ(v[1], "in bar");
  ASSERT_EQ(v[2], "in baz");
}

TEST(class_, local_inherit_other)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class A {}
fun f() {
  class B : A {
    f() { return "in b"; }
  }
  return B;
}
return f()().f(); # expect: "in b"
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "in b");
}

TEST(class_, local_inherit_self)
{
  VM vm;
  auto [res, chunk] = compile(R"(
{
  class Foo : Foo {} 
}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(class_, local_reference_self)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r;
{
  class Foo {
    returnSelf() {
      return Foo;
    }
    bar() {
      return "foo_self";
    }
  }
  r = Foo().returnSelf(); # expect: Foo
}
return r().bar();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "foo_self");
}

TEST(class_, reference_self)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r;
class Foo {
  returnSelf() {
    return Foo;
  }
  bar() {
    return "foo_self";
  }
}

return Foo().returnSelf()().bar();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "foo_self");
}