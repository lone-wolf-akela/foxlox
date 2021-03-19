#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

using namespace foxlox;

TEST(comments, line_at_eof)
{
  VM vm;
  auto [res, chunk] = compile(R"(
return "ok";
# comment)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("ok"));
}

TEST(comments, only_line_comment)
{
  VM vm;
  auto [res, chunk] = compile(R"(# comment)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<nil_t>(v));
}

TEST(comments, only_line_comment_and_line)
{
  VM vm;
  auto [res, chunk] = compile(R"(# comment
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<nil_t>(v));
}

TEST(comments, unicode)
{
  VM vm;
  auto [res, chunk] = compile(R"(
# Unicode characters are allowed in comments.
#
# Latin 1 Supplement: £§¶ÜÞ
# Latin Extended-A: ĐĦŋœ
# Latin Extended-B: ƂƢƩǁ
# Chinese: 你好！
# Other stuff: ឃᢆ᯽₪ℜ↩⊗┺░
# Emoji: ☃☺♣
return "ok";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("ok"));
}