#include <gtest/gtest.h>
import foxlox;

//TODO

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