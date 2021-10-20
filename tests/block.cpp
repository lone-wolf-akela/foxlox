#include <gtest/gtest.h>
import foxlox;

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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(block, scope)
{
  auto [res, chunk] = compile(R"(
var a = "outer", r = ();
{
  var a = "inner";
  r += a;
}
return r + a; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "inner");
  ASSERT_EQ(v[1], "outer");
}