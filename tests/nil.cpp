#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(nil, literal_)
{
  auto [res, chunk] = compile(R"(
return nil;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, nil);
}