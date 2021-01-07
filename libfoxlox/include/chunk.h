#ifndef FOXLOX_CHUNK_H
#define FOXLOX_CHUNK_H

#include <cstdint>
#include <vector>
#include <string>

#include <gsl/gsl>

#include "../src/value.h"
#include "../src/opcode.h"

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
    using ConstStringPool = std::vector<String*>;

    const Code& get_code() const;
    const ValueArray& get_constants() const;
    const ConstStringPool& get_strings() const;
    const LineInfo& get_lines() const;
    void add_code(Inst c, int line_num);
    uint32_t add_constant(Value v);
    uint32_t add_string(std::string_view str);

    ~Chunk();
  private:
    LineInfo lines;
    Code code;
    ValueArray constants;
    ConstStringPool strings;
  };
}
#endif // FOXLOX_CHUNK_H
