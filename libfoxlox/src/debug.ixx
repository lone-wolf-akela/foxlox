export module foxlox:debug;

import <string_view>;
import <iostream>;
import <format>;

import :chunk;

namespace foxlox
{
  export class VM;

  export class Debugger
  {
  public:
    Debugger(bool colored_output) noexcept :
      colored(colored_output)
    {
    }
    gsl::index disassemble_inst(const VM& vm, const Subroutine& subroutine, gsl::index index);
    void disassemble_chunk(const VM& vm, const Subroutine& subroutine, std::string_view name)
    {
      std::cout << std::format("== {} ==\n", name);
      gsl::index i = 0;
      while (i < ssize(subroutine.get_code()))
      {
        i += disassemble_inst(vm, subroutine, i);
      }
    }
    void print_vm_stack(VM& vm);
  private:
    [[maybe_unused]] bool colored;
  };
}