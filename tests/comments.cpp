#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(comments, line_at_eof)
{
  VM vm;
  auto [res, chunk] = compile(R"(
return "ok";
# comment)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}

TEST(comments, only_line_comment)
{
  VM vm;
  auto [res, chunk] = compile(R"(# comment)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<nil_t>());
}

TEST(comments, only_line_comment_and_line)
{
  VM vm;
  auto [res, chunk] = compile(R"(# comment
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<nil_t>());
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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "ok");
}