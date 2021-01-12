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
    void edit_code(gsl::index idx, int16_t c);
    gsl::index get_code_num();
    const LineInfo& get_lines() const;
  private:
    std::vector<uint8_t> code;
    LineInfo lines;
  };

  class Chunk
  {
  public:
    Chunk();
    ~Chunk();
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&& r) noexcept;
    Chunk& operator=(Chunk&& r) noexcept;

    std::vector<Closure>& get_closures();
    const std::vector<Closure>& get_closures() const;
    const std::vector<Value>& get_constants() const;
    const std::vector<String*>& get_const_strings() const;
    
    uint16_t add_constant(Value v);
    uint16_t add_closure();
    uint16_t add_string(std::string_view str);
    uint16_t add_static_value();
    uint16_t get_static_value_num();
  private:
    void clean();
    bool is_moved;

    std::vector<Closure> closures;

    std::vector<Value> constants;
    std::vector<String*> const_strings;

    uint16_t static_value_num;
  };
}
#endif // FOXLOX_CHUNK_H
