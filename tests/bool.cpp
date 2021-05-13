#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(bool_, equality)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == true; 
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  // Not equal to other types.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == 0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == 1.0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == 0.0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == "true";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == "false";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == "";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == nil;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }

  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != true; 
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  // Not equal to other types.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != 0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != 1.0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != 0.0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != "true";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != "false";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != "";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != nil;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
}

TEST(bool_, not_)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return !true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, false);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return !false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return !!true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, true);
  }
}