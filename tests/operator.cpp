#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(operator_, add)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += 123 + 456;
r += "str" + "ing";
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], 579);
  ASSERT_EQ(v[1], "string");
}

TEST(operator_, add_bool_nil)
{
  VM vm;
  auto [res, chunk] = compile(R"(
true + nil;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, add_bool_num)
{
  VM vm;
  auto [res, chunk] = compile(R"(
true + 123;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, add_bool_string)
{
  VM vm;
  auto [res, chunk] = compile(R"(
true + "s";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, add_nil_nil)
{
  VM vm;
  auto [res, chunk] = compile(R"(
nil + nil;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, add_num_nil)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 + nil;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, add_string_nil)
{
  VM vm;
  auto [res, chunk] = compile(R"(
"s" + nil;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, comparison)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += 1 < 2;
r += 2 < 2;
r += 2 < 1;

r += 1 <= 2;
r += 2 <= 2;
r += 2 <= 1;

r += 1 > 2;
r += 2 > 2;
r += 2 > 1;

r += 1 >= 2;
r += 2 >= 2;
r += 2 >= 1;

####
r += 1.0 < 2;
r += 2.0 < 2;
r += 2.0 < 1;

r += 1.0 <= 2;
r += 2.0 <= 2;
r += 2.0 <= 1;

r += 1.0 > 2;
r += 2.0 > 2;
r += 2.0 > 1;

r += 1.0 >= 2;
r += 2.0 >= 2;
r += 2.0 >= 1;

####
r += 1 < 2.0;
r += 2 < 2.0;
r += 2 < 1.0;

r += 1 <= 2.0;
r += 2 <= 2.0;
r += 2 <= 1.0;

r += 1 > 2.0;
r += 2 > 2.0;
r += 2 > 1.0;

r += 1 >= 2.0;
r += 2 >= 2.0;
r += 2 >= 1.0;
####
r += 1.0 < 2.0;
r += 2.0 < 2.0;
r += 2.0 < 1.0;

r += 1.0 <= 2.0;
r += 2.0 <= 2.0;
r += 2.0 <= 1.0;

r += 1.0 > 2.0;
r += 2.0 > 2.0;
r += 2.0 > 1.0;

r += 1.0 >= 2.0;
r += 2.0 >= 2.0;
r += 2.0 >= 1.0;
# Zero and negative zero compare the same.
r += 0 < -0;
r += -0 < 0;
r += 0 > -0;
r += -0 > 0;
r += 0 <= -0;
r += -0 <= 0;
r += 0 >= -0;
r += -0 >= 0;
####
r += 0.0 < -0;
r += -0.0 < 0;
r += 0.0 > -0;
r += -0.0 > 0;
r += 0.0 <= -0;
r += -0.0 <= 0;
r += 0.0 >= -0;
r += -0.0 >= 0;
####
r += 0 < -0.0;
r += -0 < 0.0;
r += 0 > -0.0;
r += -0 > 0.0;
r += 0 <= -0.0;
r += -0 <= 0.0;
r += 0 >= -0.0;
r += -0 >= 0.0;
####
r += 0.0 < -0.0;
r += -0.0 < 0.0;
r += 0.0 > -0.0;
r += -0.0 > 0.0;
r += 0.0 <= -0.0;
r += -0.0 <= 0.0;
r += 0.0 >= -0.0;
r += -0.0 >= 0.0;
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 80);
  int i = 0;
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);

  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);

  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);

  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);

  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);

  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);

  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);

  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], false);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
  ASSERT_EQ(v[i++], true);
}

TEST(operator_, divide)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += 8 / 2;
r += 12.34 / 12.34;
r += 7 / 2;
r += 7 // 2;
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], 4.0);
  ASSERT_EQ(v[1], 1.0);
  ASSERT_EQ(v[2], 3.5);
  ASSERT_EQ(v[3], 3);
}

TEST(operator_, divide_num_nonnum)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 / "1";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, equals)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += nil == nil; 

r += true == true;
r += true == false; 
 
r += 1 == 1; 
r += 1 == 2; 
r += 1.0 == 1; 
r += 1.0 == 2; 
r += 1 == 1.0; 
r += 1 == 2.0; 
r += 1.0 == 1.0; 
r += 1.0 == 2.0; 

r += "str" == "str";
r += "str" == "ing"; 

r += nil == false; 
r += false == 0; 
r += 0 == "0"; 
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 16);
  ASSERT_EQ(v[0], true);
  ASSERT_EQ(v[1], true);
  ASSERT_EQ(v[2], false);
  ASSERT_EQ(v[3], true);
  ASSERT_EQ(v[4], false);
  ASSERT_EQ(v[5], true);
  ASSERT_EQ(v[6], false);
  ASSERT_EQ(v[7], true);
  ASSERT_EQ(v[8], false);
  ASSERT_EQ(v[9], true);
  ASSERT_EQ(v[10], false);
  ASSERT_EQ(v[11], true);
  ASSERT_EQ(v[12], false);
  ASSERT_EQ(v[13], false);
  ASSERT_EQ(v[14], false);
  ASSERT_EQ(v[15], false);
}

TEST(operator_, equals_class)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {}
class Bar {}

r += Foo == Foo;
r += Foo == Bar;
r += Bar == Foo; 
r += Bar == Bar;

r += Foo == "Foo";
r += Foo == nil;
r += Foo == 123;
r += Foo == true;
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 8);
  ASSERT_EQ(v[0], true);
  ASSERT_EQ(v[1], false);
  ASSERT_EQ(v[2], false);
  ASSERT_EQ(v[3], true);
  ASSERT_EQ(v[4], false);
  ASSERT_EQ(v[5], false);
  ASSERT_EQ(v[6], false);
  ASSERT_EQ(v[7], false);
}

TEST(operator_, equals_method)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
# Bound methods have identity equality.
class Foo {
  method() {}
}

var foo = Foo(), bar = Foo(), fooMethod = foo.method;

r += fooMethod == fooMethod; 
r += foo.method == foo.method;
r += fooMethod == bar.method;
r += foo.method == bar.method;
r += bar.method == bar.method;
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 5);
  ASSERT_EQ(v[0], true);
  ASSERT_EQ(v[1], true);
  ASSERT_EQ(v[2], false);
  ASSERT_EQ(v[3], false);
  ASSERT_EQ(v[4], true);
}

TEST(operator_, greater_nonnum_num)
{
  VM vm;
  auto [res, chunk] = compile(R"(
"1" > 1; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, greater_num_nonnum)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 > "1";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, greater_or_equal_nonnum_num)
{
  VM vm;
  auto [res, chunk] = compile(R"(
"1" >= 1;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, greater_or_equal_num_nonnum)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 >= "1";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, less_nonnum_num)
{
  VM vm;
  auto [res, chunk] = compile(R"(
"1" < 1;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, less_num_nonnum)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 < "1";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, less_or_equal_nonnum_num)
{
  VM vm;
  auto [res, chunk] = compile(R"(
"1" <= 1;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, less_or_equal_num_nonnum)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 <= "1";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, multiply)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += 5*3;
r += 5*3.5;
r += 5.5*3;
r += 12.4*0.3;
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], 5*3);
  ASSERT_EQ(v[1], 5*3.5);
  ASSERT_EQ(v[2], 5.5*3);
  ASSERT_EQ(v[3], 12.4*0.3);
}

TEST(operator_, multiply_nonnum_num)
{
  VM vm;
  auto [res, chunk] = compile(R"(
"1" * 1;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, multiply_num_nonnum)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 * "1";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, negate)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += -(3);
r += - -(3);
r += - - -(3);
r += -(3.5);
r += - -(3.5);
r += - - -(3.5);
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 6);
  ASSERT_EQ(v[0], -3);
  ASSERT_EQ(v[1], 3);
  ASSERT_EQ(v[2], -3);
  ASSERT_EQ(v[3], -3.5);
  ASSERT_EQ(v[4], 3.5);
  ASSERT_EQ(v[5], -3.5);
}

TEST(operator_, negate_nonnum)
{
  VM vm;
  auto [res, chunk] = compile(R"(
-"s";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, not_)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += !true;
r += !false;
r += !!true; 

r += !123;
r += !0;
r += !123.5;
r += !0.0;

r += !nil;

r += !"";

fun foo() {}
r += !foo;
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 10);
  ASSERT_EQ(v[0], false);
  ASSERT_EQ(v[1], true);
  ASSERT_EQ(v[2], true);
  ASSERT_EQ(v[3], false);
  ASSERT_EQ(v[4], false);
  ASSERT_EQ(v[5], false);
  ASSERT_EQ(v[6], false);
  ASSERT_EQ(v[7], true);
  ASSERT_EQ(v[8], false);
  ASSERT_EQ(v[9], false);
}

TEST(operator_, not_class)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
class Bar {}
r += !Bar;
r += !Bar();
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], false);
  ASSERT_EQ(v[1], false);
}

TEST(operator_, not_equals)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += nil != nil;

r += true != true;
r += true != false;

r += 1 != 1;
r += 1 != 2;
r += 1.0 != 1;
r += 1.0 != 2;
r += 1 != 1.0;
r += 1 != 2.0;
r += 1.0 != 1.0;
r += 1.0 != 2.0;

r += "str" != "str";
r += "str" != "ing";

r += nil != false;
r += false != 0;
r += 0 != "0";
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 16);
  ASSERT_EQ(v[0], false);
  ASSERT_EQ(v[1], false);
  ASSERT_EQ(v[2], true);
  ASSERT_EQ(v[3], false);
  ASSERT_EQ(v[4], true);
  ASSERT_EQ(v[5], false);
  ASSERT_EQ(v[6], true);
  ASSERT_EQ(v[7], false);
  ASSERT_EQ(v[8], true);
  ASSERT_EQ(v[9], false);
  ASSERT_EQ(v[10], true);
  ASSERT_EQ(v[11], false);
  ASSERT_EQ(v[12], true);
  ASSERT_EQ(v[13], true);
  ASSERT_EQ(v[14], true);
  ASSERT_EQ(v[15], true);
}

TEST(operator_, subtract)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
r += 5-3;
r += 5-3.5;
r += 5.5-3;
r += 12.4-0.3;
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], 5 - 3);
  ASSERT_EQ(v[1], 5 - 3.5);
  ASSERT_EQ(v[2], 5.5 - 3);
  ASSERT_EQ(v[3], 12.4 - 0.3);
}

TEST(operator_, subtract_nonnum_num)
{
  VM vm;
  auto [res, chunk] = compile(R"(
"1" - 1;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(operator_, subtract_num_nonnum)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 - "1";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}