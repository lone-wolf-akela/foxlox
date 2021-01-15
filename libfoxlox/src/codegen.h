#pragma once

#include <map>

#include <foxlox/foxexcept.h>
#include <foxlox/chunk.h>

#include "stmt.h"
#include "expr.h"
#include "parser.h"

namespace foxlox
{
  class CodeGen : public expr::IVisitor<void>, public stmt::IVisitor<void>
  {
  public:
    CodeGen(AST&& a);
    Chunk gen();
  private:
    AST ast;
    Chunk chunk;
    int current_line;
    uint16_t current_subroutine_idx;
    Subroutine& current_subroutine();

    struct ValueIdx
    {
      stmt::VarStoreType type;
      uint16_t idx;
    };
    std::map<VarDeclareAt, ValueIdx> value_idxs;
    uint16_t current_stack_size;
    void push_stack();
    void pop_stack(uint16_t n = 1);
    // convert a stack idx between "idx from the stack bottom" and "idx from the stack top"
    uint16_t idx_cast(uint16_t idx);

    uint16_t loop_start_stack_size;

    void compile(expr::Expr* expr);
    void compile(stmt::Stmt* stmt);

    template<typename Arg1, typename ... Args>
    void emit(Arg1 arg1, Args ... args)
    {
      current_subroutine().add_code(arg1, current_line);
      if constexpr(sizeof...(Args) >= 1)
      {
        emit(std::forward<Args>(args)...);
      }
    }
    gsl::index emit_jump(OpCode c);
    void patch_jump(gsl::index ip);
    void patch_jumps(std::vector<gsl::index>& ips);
    gsl::index prepare_loop();
    void emit_loop(gsl::index ip, OpCode c);    
    void emit_pop_to(uint16_t stack_size_before);
    void pop_stack_to(uint16_t stack_size_before);
    std::vector<gsl::index> break_stmts;
    std::vector<gsl::index> continue_stmts;

    void visit_binary_expr(gsl::not_null<expr::Binary*> expr) final;
    void visit_grouping_expr(gsl::not_null<expr::Grouping*> expr) final;
    void visit_tuple_expr(gsl::not_null<expr::Tuple*> expr) final;
    void visit_literal_expr(gsl::not_null<expr::Literal*> expr) final;
    void visit_unary_expr(gsl::not_null<expr::Unary*> expr) final;
    void visit_variable_expr(gsl::not_null<expr::Variable*> expr) final;
    void visit_assign_expr(gsl::not_null<expr::Assign*> expr) final;
    void visit_logical_expr(gsl::not_null<expr::Logical*> expr) final;
    void visit_call_expr(gsl::not_null<expr::Call*> expr) final;
    void visit_get_expr(gsl::not_null<expr::Get*> /*expr*/) final { throw UnimplementedError(); }
    void visit_set_expr(gsl::not_null<expr::Set*> /*expr*/) final { throw UnimplementedError(); }
    void visit_this_expr(gsl::not_null<expr::This*> /*expr*/) final { throw UnimplementedError(); }
    void visit_super_expr(gsl::not_null<expr::Super*> /*expr*/) final { throw UnimplementedError(); }

    void visit_expression_stmt(gsl::not_null<stmt::Expression*> stmt) final;
    void visit_var_stmt(gsl::not_null<stmt::Var*> stmt) final;
    void visit_block_stmt(gsl::not_null<stmt::Block*> stmt) final;
    void visit_if_stmt(gsl::not_null<stmt::If*> stmt) final;
    void visit_while_stmt(gsl::not_null<stmt::While*> stmt) final;
    void visit_function_stmt(gsl::not_null<stmt::Function*> stmt) final;
    void visit_return_stmt(gsl::not_null<stmt::Return*> stmt) final;
    void visit_break_stmt(gsl::not_null<stmt::Break*> stmt) final;
    void visit_continue_stmt(gsl::not_null<stmt::Continue*> stmt) final;
    void visit_class_stmt(gsl::not_null<stmt::Class*> /*stmt*/) final { throw UnimplementedError(); }
    void visit_for_stmt(gsl::not_null<stmt::For*> stmt) final;
  };
}
