#ifndef FOXLOC_CHUNK_H
#define FOXLOC_CHUNK_H

#include <cstdint>
#include <vector>
#include <string>

#include <gsl/gsl>

#include "value.h"
#include "util.h"

namespace foxlox
{
  class Chunk;

  enum class OpCode : uint8_t
  {
    // no args
    OP_RETURN,
    // a
    OP_CONSTANT,
  };
  struct Inst
  {
    OpCode op;
    union
    {
      struct
      {
        uint8_t a, b, c;
      } abc;
    } data;

    Inst(OpCode op);
    Inst(OpCode op, uint8_t a);
    Inst(OpCode op, uint8_t a, uint8_t b);

    std::string to_string(const Chunk& chunk) const;
  };
  static_assert(sizeof(Inst) == 4);
  
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
    uint8_t add_constant(Value v);
  private:
    LineInfo lines;
    Code code;
    ValueArray constants;
  };
}
#endif // FOXLOC_CHUNK_H
