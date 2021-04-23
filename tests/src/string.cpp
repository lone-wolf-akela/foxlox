#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>
#include <foxlox/except.h>

using namespace foxlox;

TEST(string, error_after_multiline)
{
  VM vm;
  auto [res, chunk] = compile(R"(
# Tests that we correctly track the line info across multiline strings.
var a = "1
2
3
";
a(); # error
)");
  ASSERT_EQ(res, CompilerResult::OK);
  bool catched_except = false;
  try
  {
    vm.run(chunk);
  }
  catch (const RuntimeError& e)
  {
    ASSERT_EQ(e.source, "a(); # error");
    ASSERT_EQ(e.line, 7);
    catched_except = true;
  }
  ASSERT_TRUE(catched_except);
}

TEST(string, literals)
{
  VM vm;
  auto [res, chunk] = compile(R"literals(
var r = ();
r += "(" + ")";
r += "a string";
return r;
)literals");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 2);
  ASSERT_EQ(to_variant(s[0]), FoxValue("()"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("a string"));
}

TEST(string, non_ascii)
{
  VM vm;
  auto [res, chunk] = compile(R"(
return "你好，世界！";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("\xe4\xbd\xa0\xe5\xa5\xbd\xef\xbc\x8c\xe4\xb8\x96\xe7\x95\x8c\xef\xbc\x81"));
}

TEST(string, multiline)
{
  VM vm;
  auto [res, chunk] = compile(R"(
return "1
2
3";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("1\n2\n3"));
}

TEST(string, unterminated)
{
  auto [res, chunk] = compile(R"(
return "this string has no close quote
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(string, escape)
{
  VM vm;
  auto [res, chunk] = compile(R"(
return "\'\"\?\\\a\b\f\r\n\t\v\1\12\123\129\1234\xa\xab\xabx\u4e5d\U00024b62\xA\xAB\xABX\u4E5D\U00024B62\xAb\xaBX\u4E5d\u4e5D\0";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
#pragma warning(disable:4125)
  using namespace std::literals;
  ASSERT_EQ(v, FoxValue("\'\"\?\\\a\b\f\r\n\t\v\1\12\123\129\1234\xa\xab\xabx\u4e5d\U00024b62\xA\xAB\xABX\u4E5D\U00024B62\xAb\xaBX\u4E5d\u4e5D\0"sv));
#pragma warning(default:4125)
}
