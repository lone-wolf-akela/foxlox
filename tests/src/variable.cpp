#include <gtest/gtest.h>

#include <vm.h>
#include <compiler.h>

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
{
  var a = "a";
  # print a; // expect: a
  var b = a + " b";
  # print b; // expect: a b
  var c = a + " c";
  # print c; // expect: a c
  var d = b + " d";
  # print d; // expect: a b d
  return d;
}
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::STR);
  ASSERT_EQ(v.get_strview(), "a b d");
}