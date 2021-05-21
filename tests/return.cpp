#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(return_, after_else)
{
  VM vm;
  auto [res, chunk] = compile(R"(
fun f() {
  if (false) "no"; else return "ok";
}
return f();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(return_, after_if)
{
  VM vm;
  auto [res, chunk] = compile(R"(
fun f() {
  if (true) return "ok";
}
return f();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(return_, after_while)
{
  VM vm;
  auto [res, chunk] = compile(R"(
fun f() {
  while (true) return "ok";
}
return f();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(return_, after_for)
{
  VM vm;
  auto [res, chunk] = compile(R"(
fun f() {
  for(;;) return "ok";
}
return f();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(return_, at_top_level)
{
  VM vm;
  auto [res, chunk] = compile(R"(
return "ok";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(return_, in_function)
{
  VM vm;
  auto [res, chunk] = compile(R"(
fun f() {
  return "ok";
  return "bad";
}
return f();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(return_, in_method)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  method() {
    return "ok";
    return "bad";
  }
}
return Foo().method();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(return_, return_nil_if_no_value)
{
  VM vm;
  auto [res, chunk] = compile(R"(
fun f() {
  return;
  return "bad";
}
return f();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, nil);
}

TEST(return_, return_nil_at_top_level_by_default)
{
  VM vm;
  auto [res, chunk] = compile(R"(
1 + 1;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, nil);
}

