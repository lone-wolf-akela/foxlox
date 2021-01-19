#include <gtest/gtest.h>

#include <foxlox/vm.h>

// I wish all tests in this file could move to compile time...
TEST(static_test, zero_value_is_nil)
{
  foxlox::Value v;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
  memset(&v, 0, sizeof(v));
#pragma GCC diagnostic pop
  ASSERT_TRUE(v.is_nil());
}