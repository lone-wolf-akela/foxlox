import <tuple>;
import <gtest/gtest.h>;
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 3);
  ASSERT_EQ(to_variant(s[0]), FoxValue("init"));
  ASSERT_EQ(to_variant(s[1]), FoxValue(1_i64));
  ASSERT_EQ(to_variant(s[2]), FoxValue(2_i64));
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("init"));
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("method"));
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("non_class_func"));
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
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 2);
  ASSERT_EQ(to_variant(s[0]), FoxValue("bar"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("method"));
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