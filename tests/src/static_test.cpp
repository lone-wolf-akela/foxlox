#include <gtest/gtest.h>

#include <foxlox/vm.h>

// I wish all tests in this file could move to compile time...
TEST(static_test, zero_value_is_nil)
{
  foxlox::Value v;
#ifndef _MSC_VER 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
  memset(&v, 0, sizeof(v));
#ifndef _MSC_VER 
#pragma GCC diagnostic pop
#endif
  ASSERT_TRUE(v.is_nil());
}