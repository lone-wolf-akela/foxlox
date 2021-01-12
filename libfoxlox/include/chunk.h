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

  class Closure
  {
  public:
    const std::vector<uint8_t>& get_code() const;
    void add_code(bool c, int line_num);
    void add_code(uint8_t c, int line_num);
    void add_code(int16_t c, int line_num);
    void add_code(uint16_t c, int line_num);
    const LineInfo& get_lines() const;
  private:
    std::vector<uint8_t> code;
    LineInfo lines;
  };

  class Chunk
  {
  public:
    std::vector<Closure>& get_closures();
    const std::vector<Closure>& get_closures() const;
    const std::vector<Value>& get_constants() const;
    const std::vector<String*>& get_const_strings() const;
    
    uint16_t add_constant(Value v);
    uint16_t add_closure();
    uint16_t add_string(std::string_view str);
    int16_t add_static_value(Value v);

    ~Chunk();
  private:
    std::vector<Closure> closures;

    std::vector<Value> constants;
    std::vector<String*> const_strings;
    
    std::vector<Value> static_value_pool;
  };
}
#endif // FOXLOX_CHUNK_H
