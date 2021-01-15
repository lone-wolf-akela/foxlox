#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

using namespace foxlox;

TEST(for_, scope)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = ();
{
  var i = "before";

  # New variable is in inner scope.
  for (var i = 0; i < 1; ++i) {
    r = r + i; # expect: 0

    # Loop body is in second inner scope.
    var i = -1;
    r = r + i; # expect: -1
  }
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    ASSERT_EQ(s[0].type, Value::I64);
    ASSERT_EQ(s[0].get_int64(), 0);
    ASSERT_EQ(s[1].type, Value::I64);
    ASSERT_EQ(s[1].get_int64(), -1);
  }
  {
    auto [res, chunk] = compile(R"(
var r = ();
{
  # New variable shadows outer variable.
  for (var i = 0; i > 0; ++i) {}

  # Goes out of scope after loop.
  var i = "after";
  r = r + i; # expect: after

  # Can reuse an existing variable.
  for (i = 0; i < 1; i = ++i) {
    r = r + i; # expect: 0
  }
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "after");
    ASSERT_EQ(s[1].type, Value::I64);
    ASSERT_EQ(s[1].get_int64(), 0);
  }
}

TEST(for_, syntax)
{
  VM vm;
  // Single-expression body.
  {
    auto [res, chunk] = compile(R"(
var r = ();
for (var c = 0; c < 3;) r = r + (++c);
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
  // Block body.
  {
    auto [res, chunk] = compile(R"(
var r = ();
for (var a = 0; a < 3; ++a) {
  r = r + a;
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
  // No clauses.
  {
    auto [res, chunk] = compile(R"(
for (;;) return "done";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "done");
  }
  // No variable.
  {
    auto [res, chunk] = compile(R"(
var r = ();
var i = 0;
for (; i < 2; ++i) r = r + i;
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    for (int i = 0; i < 2; i++)
    {
      ASSERT_EQ(s[i].type, Value::I64);
      ASSERT_EQ(s[i].get_int64(), i);
    }
  }
  // No condition.
  {
    auto [res, chunk] = compile(R"(
var r = ();
for (var i = 0;; ++i) {
  r = r + i;
  if (i >= 2) return r;
}
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
  // No increment.
  {
    auto [res, chunk] = compile(R"(
var r = ();
for (var i = 0; i < 2;) {
  r = r + i;
  ++i;
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    for (int i = 0; i < 2; i++)
    {
      ASSERT_EQ(s[i].type, Value::I64);
      ASSERT_EQ(s[i].get_int64(), i);
    }
  }
  // Statement bodies.
  {
    auto [res, chunk] = compile(R"(
for (; false;) if (true) 1; else 2;
for (; false;) while (true) 1;
for (; false;) for (;;) 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::NIL);
  }
}

TEST(for_, break_)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var c;
for(c = 0; c < 10; ++c) if (c >= 3) break;
return c;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::I64);
    ASSERT_EQ(v.get_int64(), 3);
  }
  {
    auto [res, chunk] = compile(R"(
var c;
for(c = 0; c < 10; ++c) {
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
  // no init
  {
    auto [res, chunk] = compile(R"(
var c = 0;
for(; c < 10; ++c) {
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
  // no cond
  {
    auto [res, chunk] = compile(R"(
var c;
for(c = 0;; ++c) {
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
  // no incre
  {
    auto [res, chunk] = compile(R"(
var c;
for(c = 0; c < 10;) {
  if (c >= 3) {
    break;
  }
  ++c;
}
return c;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::I64);
    ASSERT_EQ(v.get_int64(), 3);
  }
}

TEST(for_, continue_)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var s = 0;
for(var c = 1; c <= 5; ++c) { 
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
  // no init
  {
    auto [res, chunk] = compile(R"(
var s = 0;
var c = 0;
for(; c <= 5; ++c) { 
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
  // no cond
  {
    auto [res, chunk] = compile(R"(
var s = 0;
for(var c = 1;; ++c) { 
  if (c > 5) break;
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
  // no incre
  {
    auto [res, chunk] = compile(R"(
var s = 0;
for(var c = 0; c < 5;) { 
  ++c;
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

TEST(for_, nested_break)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
for(;;) { 
  for(;;) {
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
while(true) { 
  for(;;) {
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

TEST(for_, nested_continue)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var outer_sum = 0;
for(var i = 11; i <= 13; ++i) { 
  var inner_sum = 0;
  for(var j = 1; j <= 3; ++j) {
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
    ASSERT_EQ(v.get_int64(), 11*(1+3) + 12*(1+3) + 13*(1+3));
  }
  {
    auto [res, chunk] = compile(R"(
var outer_sum = 0;
var i = 11;
while(i <= 13) { 
  var inner_sum = 0;
  for(var j = 1; j <= 3; ++j) {
    if (j == 2) continue;
    inner_sum = inner_sum + j;
  }
  outer_sum = outer_sum + i * inner_sum;
  ++i;
}
return outer_sum;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::I64);
    ASSERT_EQ(v.get_int64(), 11 * (1 + 3) + 12 * (1 + 3) + 13 * (1 + 3));
  }
}