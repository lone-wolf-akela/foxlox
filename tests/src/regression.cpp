#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

using namespace foxlox;

// SWITCH ON DEBUG_STRESS_GC TO TEST THIS
TEST(regression, gc_declared_but_not_used)
{
  VM vm;
  auto [res, chunk] = compile(R"(
# r is only declared but not used in global subroutine
# but it should still be in global subroutine's reference list
# otherwise, r will be recycled before it is accessed in inFoo()
var r = ();
class Foo {
  inFoo() {
    r += "in foo";
  }
}
var foo = Foo();
foo.inFoo();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_NO_THROW(vm.interpret(chunk));
}

TEST(regression, wrong_jump_length)
{
  VM vm;
  auto [res, chunk] = compile(R"(
# I was making some stupid bug...
if(true)
{
  1 + 1;
  2 + 2;
}
else
{
  2 + 2;
  1 + 1;
}
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_NO_THROW(vm.interpret(chunk));
}