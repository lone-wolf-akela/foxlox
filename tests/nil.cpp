#include <foxlox/vm.h>
#include <foxlox/compiler.h>

#include <gtest/gtest.h>

using namespace foxlox;

TEST(nil, literal_)
{
  auto [res, chunk] = compile(R"(
return nil;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.run(chunk);
  ASSERT_TRUE(v.is_nil());
}