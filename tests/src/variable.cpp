#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

using namespace foxlox;

TEST(variable, in_nested_block)
{
  auto [res, chunk] = compile(R"(
{
  var a = "outer";
  {
    return a;
  }
}
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::STR);
  ASSERT_EQ(v.get_strview(), "outer");
}

TEST(variable, in_middle_of_block)
{
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "a";
  r += a; # expect: "a"
  var b = a + " b";
  r += b; # expect: "a b"
  var c = a + " c";
  r += c; # expect: "a c"
  var d = b + " d";
  r += d; # expect: "a b d"
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::TUPLE);
  auto s = v.get_tuplespan();
  ASSERT_EQ(s.size(), 4);
  ASSERT_EQ(s[0].type, Value::STR);
  ASSERT_EQ(s[0].get_strview(), "a");
  ASSERT_EQ(s[1].type, Value::STR);
  ASSERT_EQ(s[1].get_strview(), "a b");
  ASSERT_EQ(s[2].type, Value::STR);
  ASSERT_EQ(s[2].get_strview(), "a c");
  ASSERT_EQ(s[3].type, Value::STR);
  ASSERT_EQ(s[3].get_strview(), "a b d");
}

TEST(variable, scope_reuse_in_different_blocks)
{
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "first";
  r += a; # expect: first
}
{
  var a = "second";
  r += a; # expect: second
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::TUPLE);
  auto s = v.get_tuplespan();
  ASSERT_EQ(s.size(), 2);
  ASSERT_EQ(s[0].type, Value::STR);
  ASSERT_EQ(s[0].get_strview(), "first");
  ASSERT_EQ(s[1].type, Value::STR);
  ASSERT_EQ(s[1].get_strview(), "second");
}

TEST(variable, shadow_and_local)
{
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "outer";
  {
    r += a; # expect: outer
    var a = "inner";
    r += a; # expect: inner
  }
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::TUPLE);
  auto s = v.get_tuplespan();
  ASSERT_EQ(s.size(), 2);
  ASSERT_EQ(s[0].type, Value::STR);
  ASSERT_EQ(s[0].get_strview(), "outer");
  ASSERT_EQ(s[1].type, Value::STR);
  ASSERT_EQ(s[1].get_strview(), "inner");
}

TEST(variable, shadow_global)
{
  auto [res, chunk] = compile(R"(
var r = ();
var a = "global";
{
  var a = "shadow";
  r += a; # expect: shadow
}
r += a; # expect: global
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::TUPLE);
  auto s = v.get_tuplespan();
  ASSERT_EQ(s.size(), 2);
  ASSERT_EQ(s[0].type, Value::STR);
  ASSERT_EQ(s[0].get_strview(), "shadow");
  ASSERT_EQ(s[1].type, Value::STR);
  ASSERT_EQ(s[1].get_strview(), "global");
}

TEST(variable, shadow_local)
{
  auto [res, chunk] = compile(R"(
var r = ();
{
  var a = "local";
  {
    var a = "shadow";
    r += a; # expect: shadow
  }
  r += a; # expect: local
}
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::TUPLE);
  auto s = v.get_tuplespan();
  ASSERT_EQ(s.size(), 2);
  ASSERT_EQ(s[0].type, Value::STR);
  ASSERT_EQ(s[0].get_strview(), "shadow");
  ASSERT_EQ(s[1].type, Value::STR);
  ASSERT_EQ(s[1].get_strview(), "local");
}

TEST(variable, uninitialized)
{
  auto [res, chunk] = compile(R"(
var a;
return a;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::NIL);
}