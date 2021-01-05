#include <cassert>
#include <limits>

#include <fmt/format.h>
#include <gsl/gsl>

#include "chunk.h"

namespace
{
  using namespace foxlox;
  enum class OpType
  {
    N, A, AB,
  };
  OpType get_optype(OpCode op)
  {
    switch (op)
    {
    case OpCode::OP_RETURN:
      return OpType::N;
    case OpCode::OP_CONSTANT:
      return OpType::A;
    default:
      assert(false);
      return OpType::N;
    }
  }
}

namespace foxlox
{
  const Chunk::Code& Chunk::get_code() const
  {
    return code;
  }
  const ValueArray& Chunk::get_constants() const
  {
    return constants;
  }
  void Chunk::add_code(Inst c, int line_num)
  {
    lines.add_line(ssize(code), line_num);
    code.push_back(c);
  }
  uint8_t Chunk::add_constant(Value v)
  {
    constants.push_back(v);
    const auto index = constants.size() - 1;
    assert(constants.size() - 1 <= std::numeric_limits<uint8_t>::max());
    return gsl::narrow<uint8_t>(index);
  }
  const LineInfo& Chunk::get_lines() const
  {
    return lines;
  }
  void LineInfo::add_line(gsl::index code_index, int line_num)
  {
    if (!lines.empty() && line_num == lines.back().line_num) { return; }
    lines.emplace_back(code_index, line_num);
  }
  int LineInfo::get_line(gsl::index code_index) const
  {
    for (auto& line : lines)
    {
      if (line.code_index >= code_index) { return line.line_num; }
    }
    return lines.back().line_num;
  }
  Inst::Inst(OpCode op)
  {
    assert(get_optype(op) == OpType::N);
    this->op = op;
  }
  Inst::Inst(OpCode op, uint8_t a)
  {
    assert(get_optype(op) == OpType::A);
    this->op = op;
    this->data.abc.a = a;
  }
  std::string Inst::to_string(const Chunk& chunk) const
  {
    switch (op)
    {
    case OpCode::OP_RETURN:
      return "OP_RETURN";
    case OpCode::OP_CONSTANT:
    {
      const auto constant = data.abc.a;
      return fmt::format("{:<16} {:>4} `{}'",
        "OP_CONSTANT", constant, chunk.get_constants().at(constant).to_string());
    }
    default:
      assert(false);
      return "";
    }
  }
  Inst::Inst(OpCode op, uint8_t a, uint8_t b)
  {
    assert(get_optype(op) == OpType::AB);
    this->op = op;
    this->data.abc.a = a;
    this->data.abc.b = b;
  }
}