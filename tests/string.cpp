#include <gtest/gtest.h>
import foxlox;

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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "()");
  ASSERT_EQ(v[1], "a string");
}

TEST(string, non_ascii)
{
  VM vm;
  auto [res, chunk] = compile(R"(
return "你好，世界！";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "\xe4\xbd\xa0\xe5\xa5\xbd\xef\xbc\x8c\xe4\xb8\x96\xe7\x95\x8c\xef\xbc\x81");
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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "1\n2\n3");
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
  auto v = FoxValue(vm.run(chunk));
#pragma warning(disable:4125)
  using namespace std::literals;
  ASSERT_EQ(v, "\'\"\?\\\a\b\f\r\n\t\v\1\12\123\129\1234\xa\xab\xabx\u4e5d\U00024b62\xA\xAB\xABX\u4E5D\U00024B62\xAb\xaBX\u4E5d\u4e5D\0"sv);
#pragma warning(default:4125)
}
