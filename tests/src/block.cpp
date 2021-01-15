#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

using namespace foxlox;

TEST(block, empty)
{
  auto [res, chunk] = compile(R"(
{} # By itself.

# In a statement.
if (true) {}
if (false) {} else {}

return "ok";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::STR);
  ASSERT_EQ(v.get_strview(), "ok");
}

TEST(block, scope)
{
  auto [res, chunk] = compile(R"(
var a = "outer";
var r = ();
{
  var a = "inner";
  r = r + a;
}
return r + a; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::TUPLE);
  auto s = v.get_tuplespan();
  ASSERT_EQ(s.size(), 2);
  ASSERT_EQ(s[0].type, Value::STR);
  ASSERT_EQ(s[0].get_strview(), "inner");
  ASSERT_EQ(s[1].type, Value::STR);
  ASSERT_EQ(s[1].get_strview(), "outer");
}