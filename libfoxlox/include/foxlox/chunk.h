#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <span>

#include <gsl/gsl>

#include "../../src/value.h"
#include "../../src/opcode.h"

namespace foxlox
{
  class LineInfo
  {
  public:
    void add_line(gsl::index code_index, int line_num);
    int get_line(gsl::index code_index) const noexcept;
  private:
    struct LineNum
    {
      gsl::index code_index;
      int line_num;
    };
    std::vector<LineNum> lines;
  };

  class Subroutine
  {
  public:
    Subroutine(std::string_view func_name, int num_of_params);
    std::span<const uint8_t> get_code() const noexcept;
    void add_code(bool c, int line_num);
    void add_code(uint8_t c, int line_num);
    void add_code(int16_t c, int line_num);
    void add_code(uint16_t c, int line_num);
    void edit_code(gsl::index idx, int16_t c);
    void add_referenced_static_value(uint16_t idx);
    std::span<const uint16_t> get_referenced_static_values() const noexcept;
    gsl::index get_code_num() const noexcept;
    const LineInfo& get_lines() const noexcept;
    int get_arity() const noexcept;
    std::string_view get_funcname() const noexcept;

    bool gc_mark;
  private:
    const int arity;
    std::vector<uint8_t> code;
    
    // for error report
    const std::string name;
    LineInfo lines;
    
    // for memory management
    std::vector<uint16_t> referenced_static_values;
  };

  class Chunk
  {
  public:
    Chunk() noexcept;
    ~Chunk();
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&& r) noexcept;
    Chunk& operator=(Chunk&& r) noexcept;

    std::vector<Subroutine>& get_subroutines() noexcept;
    std::span<const Subroutine> get_subroutines() const noexcept;
    std::span<const Value> get_constants() const;
    std::span<const String* const> get_const_strings() const;
    std::span<String*> get_const_strings();
    void set_source(std::vector<std::string>&& src) noexcept;
    std::string_view get_source(gsl::index line_num) const;

    uint16_t add_constant(Value v);
    uint16_t add_subroutine(std::string_view func_name, int num_of_params);
    uint16_t add_string(std::string_view str);
    uint16_t add_static_value() noexcept;
    uint16_t get_static_value_num() const noexcept;
  private:
    void clean();
    bool is_moved;

    std::vector<std::string> source;

    std::vector<Subroutine> subroutines;

    std::vector<Value> constants;
    std::vector<String*> const_strings;

    uint16_t static_value_num;
  };
}

