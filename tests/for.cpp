#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(for_, class_in_body)
{
  auto [res, chunk] = compile(R"(
for (;;) class Foo {}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(for_, closure_in_body)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();

var f1, f2, f3;
for (var i = 1; i < 4; ++i) {
  var j = i;
  fun f() {
    r += i;
    r += j;
  }
  if (j == 1) f1 = f;
  else if (j == 2) f2 = f;
  else f3 = f;
}
f1(); # expect: 4
      # expect: 3
f2(); # expect: 4
      # expect: 3
f3(); # expect: 4
      # expect: 3

return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 6);
  ASSERT_EQ(v[0], 4);
  ASSERT_EQ(v[1], 3);
  ASSERT_EQ(v[2], 4);
  ASSERT_EQ(v[3], 3);
  ASSERT_EQ(v[4], 4);
  ASSERT_EQ(v[5], 3);
}

TEST(for_, fun_in_body)
{
  auto [res, chunk] = compile(R"(
for (;;) fun foo() {}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(for_, return_closure)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r;
fun f() {
  for (;;) {
    var i = "i";
    fun g() { r = i; }
    return g;
  }
}
var h = f();
h(); # expect: i
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "i");
}

TEST(for_, return_inside)
{
  VM vm;
  auto [res, chunk] = compile(R"(
fun f() {
  for (;;) {
    var i = "i";
    return i;
  }
}
return f(); # expect: i
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "i");
}

TEST(for_, scope)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var r = ();
{
  var i = "before";

  # New variable is in inner scope.
  for (var i = 0; i < 1; ++i) {
    r += i; # expect: 0

    # Loop body is in second inner scope.
    var i = -1;
    r += i; # expect: -1
  }
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 2);
    ASSERT_EQ(v[0], 0);
    ASSERT_EQ(v[1], -1);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var r = ();
{
  # New variable shadows outer variable.
  for (var i = 0; i > 0; ++i) {}

  # Goes out of scope after loop.
  var i = "after";
  r += i; # expect: after

  # Can reuse an existing variable.
  for (i = 0; i < 1; i = ++i) {
    r += i; # expect: 0
  }
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 2);
    ASSERT_EQ(v[0], "after");
    ASSERT_EQ(v[1], 0);
  }
}

TEST(for_, statement_condition)
{
  auto [res, chunk] = compile(R"(
for (var a = 1; {}; a = a + 1) {}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(for_, statement_increment)
{
  auto [res, chunk] = compile(R"(
for (var a = 1; a < 2; {}) {}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(for_, statement_initializer)
{
  auto [res, chunk] = compile(R"(
for ({}; a < 2; a = a + 1) {}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(for_, syntax)
{
  // Single-expression body.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var r = ();
for (var c = 0; c < 3;) r += (++c);
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    for (int64_t i = 0; i < 3; i++)
    {
      ASSERT_EQ(v[i], i + 1);
    }
  }
  // Block body.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var r = ();
for (var a = 0; a < 3; ++a) {
  r += a;
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    for (int64_t i = 0; i < 3; i++)
    {
      ASSERT_EQ(v[i], i);
    }
  }
  // No clauses.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
for (;;) return "done";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "done");
  }
  // No variable.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var r = (), i = 0;
for (; i < 2; ++i) r += i;
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 2);
    for (int64_t i = 0; i < 2; i++)
    {
      ASSERT_EQ(v[i], i);
    }
  }
  // No condition.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var r = ();
for (var i = 0;; ++i) {
  r += i;
  if (i >= 2) return r;
}
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    for (int64_t i = 0; i < 3; i++)
    {
      ASSERT_EQ(v[i], i);
    }
  }
  // No increment.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var r = ();
for (var i = 0; i < 2;) {
  r += i;
  ++i;
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 2);
    for (int64_t i = 0; i < 2; i++)
    {
      ASSERT_EQ(v[i], i);
    }
  }
  // Statement bodies.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
for (; false;) if (true) 1; else 2;
for (; false;) while (true) 1;
for (; false;) for (;;) 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, nil);
  }
}

TEST(for_, var_in_body)
{
  auto [res, chunk] = compile(R"(
for (;;) var foo;
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(for_, break_)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var c;
for(c = 0; c < 10; ++c) if (c >= 3) break;
return c;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 3);
  }
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 3);
  }
  // no init
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 3);
  }
  // no cond
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 3);
  }
  // no incre
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, 3);
  }
}

TEST(for_, continue_)
{
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, int64_t(1 + 2 + 3 + 4 + 5 - 3));
  }
  // no init
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var s = 0, c = 0;
for(; c <= 5; ++c) { 
  if (c == 3) {
    continue;
  }
  s = s + c;
}
return s;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, int64_t(1 + 2 + 3 + 4 + 5 - 3));
  }
  // no cond
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, int64_t(1 + 2 + 3 + 4 + 5 - 3));
  }
  // no incre
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, int64_t(1 + 2 + 3 + 4 + 5 - 3));
  }
}

TEST(for_, nested_break)
{
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "mid");
  }
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "mid");
  }
}

TEST(for_, nested_continue)
{
  {
    VM vm;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, int64_t(11 * (1 + 3) + 12 * (1 + 3) + 13 * (1 + 3)));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var outer_sum = 0, i = 11;
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, int64_t(11 * (1 + 3) + 12 * (1 + 3) + 13 * (1 + 3)));
  }
}