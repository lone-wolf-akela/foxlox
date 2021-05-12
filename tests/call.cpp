import <tuple>;
import <gtest/gtest.h>;
import foxlox;

using namespace foxlox;

TEST(call, bool_)
{
  VM vm;
  auto [res, chunk] = compile(R"(
true();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(call, nil)
{
  VM vm;
  auto [res, chunk] = compile(R"(
nil();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(call, int64)
{
  VM vm;
  auto [res, chunk] = compile(R"(
123();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(call, float64)
{
  VM vm;
  auto [res, chunk] = compile(R"(
123.0();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(call, object)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {}
var foo = Foo();
foo();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(call, string)
{
  VM vm;
  auto [res, chunk] = compile(R"(
"str"();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}