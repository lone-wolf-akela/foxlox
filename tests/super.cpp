#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(super, bound_method)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
class A {
  method(arg) {
    return "A.method(" + arg + ")";
  }
}
class B : A {
  getClosure() {
    return super.method;
  }
  method(arg) {
    return "B.method(" + arg + ")";
  }
}
var closure = B().getClosure();
return closure("arg");
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "A.method(arg)");
}

TEST(super, call_other_method)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r = ();
class Base {
  foo() {
    r += "Base.foo()";
  }
}
class Derived : Base {
  bar() {
    r += "Derived.bar()";
    super.foo();
  }
}
Derived().bar();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "Derived.bar()");
  ASSERT_EQ(v[1], "Base.foo()");
}

TEST(super, call_same_method)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r = ();
class Base {
  foo() {
    r += "Base.foo()";
  }
}
class Derived : Base {
  foo() {
    r += "Derived.foo()";
    super.foo();
  }
}
Derived().foo();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "Derived.foo()");
  ASSERT_EQ(v[1], "Base.foo()");
}

TEST(super, capture_super)
{
  auto [res, chunk] = compile(R"CODE(
class Base {
  toString() { return "Base"; }
}
class Derived : Base {
  getClosure() {
    fun closure() {
      return super.toString();
    }
    return closure;
  }
  toString() { return "Derived"; }
}
var closure = Derived().getClosure();
return closure();
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(super, constructor)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r = ();
class Base {
  __init__(a, b) {
    r += "Base.__init__(" + a + ", " + b + ")";
  }
}
class Derived : Base {
  __init__() {
    r += "Derived.__init__()";
    super.__init__("a", "b");
  }
}
Derived();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "Derived.__init__()");
  ASSERT_EQ(v[1], "Base.__init__(a, b)");
}

TEST(super, extra_arguments)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
class Base {
  foo(a, b) {
  }
}
class Derived : Base {
  foo() {
    super.foo("a", "b", "c", "d");
  }
}
Derived().foo();
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(super, indirectly_inherited)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r = ();
class A {
  foo() {
    r += "A.foo()";
  }
}
class B : A {}
class C : B {
  foo() {
    r += "C.foo()";
    super.foo();
  }
}
C().foo();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "C.foo()");
  ASSERT_EQ(v[1], "A.foo()");
}

TEST(super, missing_arguments)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
class Base {
  foo(a, b) {
  }
}
class Derived : Base {
  foo() {
    super.foo(1);
  }
}
Derived().foo();
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(super, no_superclass_bind)
{
  auto [res, chunk] = compile(R"CODE(
class Base {
  foo() {
    super.doesNotExist;
  }
}
Base().foo();
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(super, no_superclass_call)
{
  auto [res, chunk] = compile(R"CODE(
class Base {
  foo() {
    super.doesNotExist(1);
  }
}
Base().foo();
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(super, no_superclass_method)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
class Base {}
class Derived : Base {
  foo() {
    super.doesNotExist(1);
  }
}
Derived().foo();
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(super, parenthesized)
{
  auto [res, chunk] = compile(R"CODE(
class A {
  method() {}
}
class B : A {
  method() {
    (super).method();
  }
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(super, reassign_superclass)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r = ();
class Base {
  method() {
    r += "Base.method()";
  }
}
class Derived : Base {
  method() {
    super.method();
  }
}
class OtherBase {
  method() {
    r += "OtherBase.method()";
  }
}
var derived = Derived();
derived.method();
Base = OtherBase;
derived.method();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "Base.method()");
  ASSERT_EQ(v[1], "Base.method()");
}

TEST(super, super_at_top_level)
{
  {
    auto [res, chunk] = compile(R"CODE(
super.foo("bar");
)CODE");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"CODE(
super.foo;
)CODE");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
}

TEST(super, super_in_closure_in_inherited_method)
{
  auto [res, chunk] = compile(R"CODE(
var r;
class A {
  say() {
    r = "A";
  }
}
class B : A {
  getClosure() {
    fun closure() {
      super.say();
    }
    return closure;
  }
  say() {
    r = "B";
  }
}
class C : B {
  say() {
    r = "C";
  }
}
C().getClosure()();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(super, super_in_inherited_method)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
class A {
  say() {
    return "A";
  }
}
class B : A {
  test() {
    return super.say();
  }
  say() {
    return "B";
  }
}
class C : B {
  say() {
    return "C";
  }
}
return C().test();
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "A");
}

TEST(super, super_in_multilevel_inherited_method)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
class Z {
  say() {
    return "Z";
  }
  test() {
    return "Ztest";
  }
}
class A : Z {
  say() {
    return "A";
  }
}
class B : A {
  test() {
    return super.say();
  }
  say() {
    return "B";
  }
}
class C : B {
  say() {
    return "C";
  }
}
class D : C {
  say() {
    return "D";
  }
}
return D().test();
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "A");
}

TEST(super, super_in_top_level_function)
{
  auto [res, chunk] = compile(R"CODE(
super.bar();
fun foo() {}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(super, super_without_dot)
{
  auto [res, chunk] = compile(R"CODE(
class A {}
class B : A {
  method() {
    super;
  }
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(super, super_without_name)
{
  auto [res, chunk] = compile(R"CODE(
class A {}
class B : A {
  method() {
    super.;
  }
}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(super, this_in_superclass_method)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r = ();
class Base {
  __init__(a) {
    this.a = a;
  }
}
class Derived : Base {
  __init__(a, b) {
    super.__init__(a);
    this.b = b;
  }
}
var derived = Derived("a", "b");
r += derived.a;
r += derived.b;
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "a");
  ASSERT_EQ(v[1], "b");
}