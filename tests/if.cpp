import <tuple>;
import <gtest/gtest.h>;
import foxlox;

using namespace foxlox;

TEST(if_, class_in_else)
{
  auto [res, chunk] = compile(R"(
if (true) "ok"; else class Foo {}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(if_, class_in_then)
{
  auto [res, chunk] = compile(R"(
if (true) class Foo {}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(if_, dangling_else)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (true) if (false) return "bad"; else return "good";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue("good"));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (false) if (true) return "bad"; else return "bad";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(nil));
  }
}

TEST(if_, if_)
{
  // Evaluate the 'then' expression if the condition is true.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (true) return "good";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "good");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (false) return "bad";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_nil());
  }
  // Allow block body.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (true) { return "block"; }
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "block");
  }
  // Assignment in if condition.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var a = false;
if (a = true) return a; 
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.v.b, true);
  }
}

TEST(if_, else_)
{
  // Evaluate the 'else' expression if the condition is false.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (true) return "good"; else return "bad";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "good");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (false) return "bad"; else return "good";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "good");
  }
  // Allow block body.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (false) nil; else { return "block"; }
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "block");
  }
}

TEST(if_, truth)
{
  // False and nil are false.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (false) return "bad"; else return "false";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "false");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (nil) return "bad"; else return "nil";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "nil");
  }
  // Everything else is true.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (true) return true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.v.b, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if (0) return 0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_EQ(v.type, ValueType::I64);
    ASSERT_EQ(v.get_int64(), 0);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
if ("") return "empty";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "empty");
  }
}