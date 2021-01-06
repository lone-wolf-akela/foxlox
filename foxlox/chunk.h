#ifndef FOXLOX_CHUNK_H
#define FOXLOX_CHUNK_H

#include <cstdint>
#include <vector>
#include <string>

#include <gsl/gsl>

#include "value.h"
#include "opcode.h"

namespace foxlox
{
  class LineInfo
  {
  public:
    void add_line(gsl::index code_index, int line_num);
    int get_line(gsl::index code_index) const;
  private:
    struct LineNum
    {
      gsl::index code_index;
      int line_num;
    };
    std::vector<LineNum> lines;
  };
  class Chunk
  {
  public:
    using Code = std::vector<Inst>;
    const Code& get_code() const;
    const ValueArray& get_constants() const;
    const LineInfo& get_lines() const;
    void add_code(Inst c, int line_num);
    uint32_t add_constant(Value v);
  private:
    LineInfo lines;
    Code code;
    ValueArray constants;
  };
}
#endif // FOXLOX_CHUNK_H
