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
  ASSERT_NO_THROW(vm.run(chunk));
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
  ASSERT_NO_THROW(vm.run(chunk));
}

TEST(regression, init_not_found_after_rehash)
{
  VM vm;
  auto [res, chunk] = compile(R"(
# there was a bug
# it hapens when there are 7 entries in string pool
# which trigers the rehash of the pool
# and the pool wrongly use the old capacity mask instead of the new one
# during rehashing
class C {
  __init__(a) {
    this.one = 1;
    this.two = 2;
    this.three = 3;
    this.four = 4;
    this.five = 5;
    # __init__ is the 6th
    # and the vm will try add another `__init__' (the 7th entry) into the pool
    # which triger the rehash
  }
}
C(0);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_NO_THROW(vm.run(chunk));
}