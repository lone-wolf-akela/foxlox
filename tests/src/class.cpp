#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

using namespace foxlox;

TEST(class_, empty)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {}
return Foo;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<Class*>(v));
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(to_variant(s[0]), FoxValue("in foo"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("in bar"));
  ASSERT_EQ(to_variant(s[2]), FoxValue("in baz"));
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("in b"));
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("foo_self"));
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("foo_self"));
}