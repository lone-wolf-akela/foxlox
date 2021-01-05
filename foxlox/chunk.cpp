#include <cassert>
#include <limits>
#include <map>
#include <string_view>

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

  const std::map<OpCode, OpType> optype
  {
    {OpCode::OP_RETURN, OpType::N},
    {OpCode::OP_NEGATE, OpType::N},
    {OpCode::OP_ADD, OpType::N},
    {OpCode::OP_SUBTRACT, OpType::N},
    {OpCode::OP_MULTIPLY, OpType::N},
    {OpCode::OP_DIVIDE, OpType::N},
    {OpCode::OP_INTDIV, OpType::N},

    {OpCode::OP_CONSTANT, OpType::A},
  };
  const std::map<OpCode, std::string_view> opname
  {
    {OpCode::OP_RETURN, "OP_RETURN"},
    {OpCode::OP_NEGATE, "OP_NEGATE"},
    {OpCode::OP_ADD, "OP_ADD"},
    {OpCode::OP_SUBTRACT, "OP_SUBTRACT"},
    {OpCode::OP_MULTIPLY, "OP_MULTIPLY"},
    {OpCode::OP_DIVIDE, "OP_DIVIDE"},
    {OpCode::OP_INTDIV, "OP_INTDIV"},

    {OpCode::OP_CONSTANT, "OP_CONSTANT"},
  };
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
  uint32_t Chunk::add_constant(Value v)
  {
    constants.push_back(v);
    const auto index = constants.size() - 1;
    assert(index <= INST_A_MAX);
    return gsl::narrow<uint32_t>(index);
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
    auto last_line_num = lines.front().line_num;
    for (auto& line : lines)
    {
      if (line.code_index > code_index) { return last_line_num; }
      last_line_num = line.line_num;
    }
    return last_line_num;
  }
  Inst::Inst(OpCode o)
  {
    assert(optype.at(o) == OpType::N);
    N.op = o;
  }
  Inst::Inst(OpCode o, uint32_t a)
  {
    assert(optype.at(o) == OpType::A);
    assert(a <= INST_A_MAX);
    A.op = o;
    A.a = a;
  }

  std::string Inst::to_string(const Chunk& chunk) const
  {
    if (optype.at(N.op) == OpType::N)
    {
      return std::string(opname.at(N.op));
    }
    switch (N.op)
    {
    case OpCode::OP_CONSTANT:
    {
      const auto constant = A.a;
      return fmt::format("{:<16} {:>4} `{}'",
        opname.at(N.op), constant, chunk.get_constants().at(constant).to_string());
    }
    default:
      assert(false);
      return "";
    }
  }
}