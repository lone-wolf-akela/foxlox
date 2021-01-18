#include "stmt.h"

namespace foxlox::stmt
{
  Expression::Expression(std::unique_ptr<expr::Expr>&& expr) noexcept :
    expression(std::move(expr)) 
  {
  }
  Var::Var(Token&& tk, std::unique_ptr<expr::Expr>&& init) noexcept :
    name(std::move(tk)), 
    initializer(std::move(init)),
    store_type{}
  {
  }
  While::While(std::unique_ptr<expr::Expr>&& cond, std::unique_ptr<Stmt>&& bd, Token&& r_paren) noexcept :
	  condition(std::move(cond)), 
    body(std::move(bd)),
    right_paren(std::move(r_paren))
  {
  }
  Block::Block(std::vector<std::unique_ptr<Stmt>>&& stmts) noexcept :
    statements(std::move(stmts)) 
  {
  }
  If::If(
    std::unique_ptr<expr::Expr>&& cond, 
    std::unique_ptr<Stmt>&& thenb, 
    std::unique_ptr<Stmt>&& elseb,
    Token&& r_paren
  ) noexcept :
    condition(std::move(cond)), 
    then_branch(std::move(thenb)), 
    else_branch(std::move(elseb)),
    right_paren(std::move(r_paren))
  {
  }
  Function::Function(Token&& tk, std::vector<Token>&& par, std::vector<std::unique_ptr<Stmt>>&& bd) noexcept :
    name(std::move(tk)), 
    param(std::move(par)), 
    body(std::move(bd)),
    name_store_type{}
  {
  }
  Return::Return(Token&& tk, std::unique_ptr<expr::Expr>&& v) noexcept :
    keyword(std::move(tk)), 
    value(std::move(v)) 
  {
  }
  Class::Class(Token&& tk, std::unique_ptr<expr::Expr>&& super, std::vector<std::unique_ptr<Function>>&& ms) noexcept :
    name(std::move(tk)), 
    superclass(std::move(super)), 
    methods(std::move(ms)),
    name_store_type{},
    this_store_type{}
  {
  }
  Break::Break(Token&& tk) noexcept :
    keyword(std::move(tk)) 
  {
  }
  Continue::Continue(Token&& tk) noexcept :
    keyword(std::move(tk)) 
  {
  }
  For::For(
    std::unique_ptr<Stmt>&& init, 
    std::unique_ptr<expr::Expr>&& cond, 
    std::unique_ptr<expr::Expr>&& incre, 
    std::unique_ptr<Stmt>&& bd,
    Token&& r_paren
  ) noexcept :
    initializer(std::move(init)), 
    condition(std::move(cond)), 
    increment(std::move(incre)), 
    body(std::move(bd)),
    right_paren(std::move(r_paren))
  {
  }
}
