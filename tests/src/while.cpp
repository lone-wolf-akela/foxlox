#include <gtest/gtest.h>

#include <vm.h>
#include <compiler.h>

using namespace foxlox;

TEST(while_, syntax)
{
  VM vm;
  {
    // Single-expression body.
    auto [res, chunk] = compile(R"(
var r = ();
var c = 0;
while (c < 3) r = r + (c = c + 1);
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    for (int i = 0; i < 3; i++)
    {
      ASSERT_EQ(s[i].type, Value::I64);
      ASSERT_EQ(s[i].get_int64(), i + 1);
    }
  }
  {
    // Block body.
    auto [res, chunk] = compile(R"(
var r = ();
var a = 0;
while (a < 3) {
  r = r + a;
  a = a + 1;
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    for (int i = 0; i < 3; i++)
    {
      ASSERT_EQ(s[i].type, Value::I64);
      ASSERT_EQ(s[i].get_int64(), i);
    }
  }
  {
    // Statement bodies.
    auto [res, chunk] = compile(R"(
while (false) if (true) 1; else 2;
while (false) while (true) 1;
while (false) for (;;) 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::NIL);
  }
}

TEST(while_, break_)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var c = 0;
while (c < 10) if ((c = c + 1) >= 3) break;
return c;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::I64);
    ASSERT_EQ(v.get_int64(), 3);
  }
  {
    auto [res, chunk] = compile(R"(
var c = 0;
while (c < 10) {
  c = c + 1;  
  if (c >= 3) {
    break;
  }
}
return c;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::I64);
    ASSERT_EQ(v.get_int64(), 3);
  }
}

TEST(while_, continue_)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var s = 0;
var c = 0;
while (c < 5) {
  c = c + 1;  
  if (c == 3) {
    continue;
  }
  s = s + c;
}
return s;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::I64);
    ASSERT_EQ(v.get_int64(), 1 + 2 + 3 + 4 + 5 - 3);
  }
}

TEST(while_, nested_break)
{
  VM vm;
  {
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "mid");
  }
  {
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
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "mid");
  }
}

TEST(while_, nested_continue)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var outer_sum = 0;
var i = 11;
while(i <= 13) { 
  var inner_sum = 0;
  var j = 0;
  while(j <= 3) {
    j = j + 1;
    if (j == 2) continue;
    inner_sum = inner_sum + j;
  }
  outer_sum = outer_sum + i * inner_sum;
  i = i + 1;
}
return outer_sum;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::I64);
    ASSERT_EQ(v.get_int64(), 11 * (1 + 3) + 12 * (1 + 3) + 13 * (1 + 3));
  }
  {
    auto [res, chunk] = compile(R"(
var outer_sum = 0;
for(var i = 11; i <= 13; i = i + 1) { 
  var inner_sum = 0;
  var j = 0;
  while(j <= 3) {
    j = j + 1;
    if (j == 2) continue;
    inner_sum = inner_sum + j;
  }
  outer_sum = outer_sum + i * inner_sum;
}
return outer_sum;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::I64);
    ASSERT_EQ(v.get_int64(), 11 * (1 + 3) + 12 * (1 + 3) + 13 * (1 + 3));
  }
}