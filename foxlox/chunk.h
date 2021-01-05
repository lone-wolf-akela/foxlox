#ifndef FOXLOX_CHUNK_H
#define FOXLOX_CHUNK_H

#include <cstdint>
#include <vector>
#include <string>

#include <gsl/gsl>

#include "value.h"

namespace foxlox
{
  constexpr uint32_t INST_A_MAX = 0xfff;
  
  class Chunk;

  enum class OpCode : uint32_t
  {
    // no args
    OP_RETURN,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_INTDIV,
    // a
    OP_CONSTANT,
  };


  union Inst
  {
    struct
    {
      OpCode op : 8;
      uint32_t : 24;
    } N;
    struct
    {
      OpCode op : 8;
      uint32_t a : 24;
    } A;
    struct
    {
      OpCode op : 8;
      uint32_t a : 12;
      uint32_t b : 12;
    }AB;

    Inst(OpCode o);
    Inst(OpCode o, uint32_t a);

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
    uint32_t add_constant(Value v);
  private:
    LineInfo lines;
    Code code;
    ValueArray constants;
  };
}
#endif // FOXLOX_CHUNK_H
