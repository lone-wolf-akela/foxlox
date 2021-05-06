#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

#include <gtest/gtest.h>

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
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("ok"));
}

TEST(block, scope)
{
  auto [res, chunk] = compile(R"(
var a = "outer";
var r = ();
{
  var a = "inner";
  r += a;
}
return r + a; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(to_variant(s[0]), FoxValue("inner"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("outer"));
}