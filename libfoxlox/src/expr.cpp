#include "expr.h"

namespace foxlox::expr
{
  Assign::Assign(Token&& tk, std::unique_ptr<Expr>&& v) : 
    name(std::move(tk)), 
    value(std::move(v)) 
  {
  }
  Binary::Binary(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) :
    left(std::move(l)), 
    op(std::move(tk)), 
    right(std::move(r))
  {
  }
  Logical::Logical(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) :
    left(std::move(l)), 
    op(std::move(tk)), 
    right(std::move(r))
  {
  }
  Grouping::Grouping(std::unique_ptr<Expr>&& expr) 
    : expression(std::move(expr)) 
  {
  }
  Literal::Literal(CompiletimeValue&& v) : 
    value(std::move(v)) 
  {
  }
  Literal::Literal(const CompiletimeValue& v) : 
    value(v) 
  {
  }
  Call::Call(std::unique_ptr<Expr>&& ce, Token&& tk, std::vector<std::unique_ptr<Expr>>&& augs) :
    callee(std::move(ce)), 
    paren(std::move(tk)), 
    arguments(std::move(augs))
  {
  }
  Variable::Variable(Token&& tk) : 
    name(std::move(tk)) 
  {
  }
  Get::Get(std::unique_ptr<Expr>&& o, Token&& tk) : 
    obj(std::move(o)), 
    name(std::move(tk)) 
  {
  }
  Set::Set(std::unique_ptr<Expr>&& o, Token&& tk, std::unique_ptr<Expr>&& v) :
    obj(std::move(o)), 
    name(std::move(tk)), 
    value(std::move(v))
  {
  }
  Super::Super(Token&& key, Token&& mthd) : 
    keyword(std::move(key)), 
    method(std::move(mthd)) 
  {
  }
  This::This(Token&& tk) : 
    keyword(tk) 
  {
  }
  Tuple::Tuple(std::vector<std::unique_ptr<Expr>>&& es):
    exprs(std::move(es))
  {
  }
}