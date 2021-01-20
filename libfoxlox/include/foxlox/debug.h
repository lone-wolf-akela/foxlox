#pragma once

#include <string_view>

#include <foxlox/chunk.h>

namespace foxlox 
{
  class VM;

  class Debugger
  {
  public:
    Debugger(bool colored_output) noexcept;
    gsl::index disassemble_inst(const VM& vm, const Subroutine& subroutine, gsl::index index);
    void disassemble_chunk(const VM& vm, const Subroutine& subroutine, std::string_view name);
    void print_vm_stack(VM& vm);
  private:
    [[maybe_unused]] bool colored;
  };
}
