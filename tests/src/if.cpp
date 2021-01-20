#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

using namespace foxlox;

TEST(if_, if_)
{
  VM vm;
  // Evaluate the 'then' expression if the condition is true.
  {
    auto [res, chunk] = compile(R"(
if (true) return "good";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "good");
  }
  {
    auto [res, chunk] = compile(R"(
if (false) return "bad";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_nil());
  }
  // Allow block body.
  {
    auto [res, chunk] = compile(R"(
if (true) { return "block"; }
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "block");
  }
  // Assignment in if condition.
  {
    auto [res, chunk] = compile(R"(
var a = false;
if (a = true) return a; 
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.v.b, true);
  }
}

TEST(if_, else_)
{
  VM vm;
  // Evaluate the 'else' expression if the condition is false.
  {
    auto [res, chunk] = compile(R"(
if (true) return "good"; else return "bad";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "good");
  }
  {
    auto [res, chunk] = compile(R"(
if (false) return "bad"; else return "good";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "good");
  }
  // Allow block body.
  {
    auto [res, chunk] = compile(R"(
if (false) nil; else { return "block"; }
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "block");
  }
}

TEST(if_, truth)
{
  VM vm;
  // False and nil are false.
  {
    auto [res, chunk] = compile(R"(
if (false) return "bad"; else return "false";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "false");
  }
  {
    auto [res, chunk] = compile(R"(
if (nil) return "bad"; else return "nil";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "nil");
  }
  // Everything else is true.
  {
    auto [res, chunk] = compile(R"(
if (true) return true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.v.b, true);
  }
  {
    auto [res, chunk] = compile(R"(
if (0) return 0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::I64);
    ASSERT_EQ(v.get_int64(), 0);
  }
  {
    auto [res, chunk] = compile(R"(
if ("") return "empty";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "empty");
  }
}