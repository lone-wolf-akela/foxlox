#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(while_, syntax)
{
  {
    VM vm;
    // Single-expression body.
    auto [res, chunk] = compile(R"(
var r = (), c = 0;
while (c < 3) r += (++c);
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    for (int i = 0; i < 3; i++)
    {
      ASSERT_EQ(v[i], i + 1);
    }
  }
  {
    VM vm;
    // Block body.
    auto [res, chunk] = compile(R"(
var r = (), a = 0;
while (a < 3) {
  r += a;
  ++a;
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    for (int i = 0; i < 3; i++)
    {
      ASSERT_EQ(v[i], i);
    }
  }
  {
    VM vm;
    // Statement bodies.
    auto [res, chunk] = compile(R"(
while (false) if (true) 1; else 2;
while (false) while (true) 1;
while (false) for (;;) 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, nil);
  }
}

TEST(while_, break_)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var c = 0;
while (c < 10) if ((++c) >= 3) break;
return c;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 3);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var c = 0;
while (c < 10) {
  ++c;  
  if (c >= 3) {
    break;
  }
}
return c;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 3);
  }
}

TEST(while_, continue_)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var s = 0, c = 0;
while (c < 5) {
  ++c;  
  if (c == 3) {
    continue;
  }
  s = s + c;
}
return s;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 1 + 2 + 3 + 4 + 5 - 3);
  }
}

TEST(while_, nested_break)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
while(true) { 
  while(true) {
    break;
    return "inner";
  }
  return "mid";
}
return "outer";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "mid");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
for(;;) { 
  while(true) {
    break;
    return "inner";
  }
  return "mid";
}
return "outer";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "mid");
  }
}

TEST(while_, nested_continue)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var outer_sum = 0, i = 11;
while(i <= 13) { 
  var inner_sum = 0, j = 0;
  while(j < 3) {
    ++j;
    if (j == 2) continue;
    inner_sum = inner_sum + j;
  }
  outer_sum = outer_sum + i * inner_sum;
  ++i;
}
return outer_sum;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 11 * (1 + 3) + 12 * (1 + 3) + 13 * (1 + 3));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var outer_sum = 0;
for(var i = 11; i <= 13; ++i) { 
  var inner_sum = 0, j = 0;
  while(j < 3) {
    ++j;
    if (j == 2) continue;
    inner_sum = inner_sum + j;
  }
  outer_sum = outer_sum + i * inner_sum;
}
return outer_sum;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 11 * (1 + 3) + 12 * (1 + 3) + 13 * (1 + 3));
  }
}

TEST(while_, class_in_body)
{
  auto [res, chunk] = compile(R"CODE(
while (true) class Foo {}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(while_, fun_in_body)
{
  auto [res, chunk] = compile(R"CODE(
while (true) fun foo() {}
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(while_, var_in_body)
{
  auto [res, chunk] = compile(R"CODE(
while (true) var foo;
)CODE");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(while_, closure_in_body)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r = (), f1, f2, f3, i = 1;
while (i < 4) {
  var j = i;
  fun f() { r += j; }
  if (j == 1) f1 = f;
  else if (j == 2) f2 = f;
  else f3 = f;
  i = i + 1;
}
f1();
f2();
f3();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 3);
  ASSERT_EQ(v[0], 3);
  ASSERT_EQ(v[1], 3);
  ASSERT_EQ(v[2], 3);
}

TEST(while_, return_closure)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
var r;
fun f() {
  while (true) {
    var i = "i";
    fun g() { r = i; }
    return g;
  }
}
var h = f();
h();
return r;
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "i");
}

TEST(while_, return_inside)
{
  VM vm;
  auto [res, chunk] = compile(R"CODE(
fun f() {
  while (true) {
    var i = "i";
    return i;
  }
}
return f();
)CODE");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "i");
}
