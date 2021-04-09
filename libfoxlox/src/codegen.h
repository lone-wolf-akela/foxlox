#pragma once

#include <map>
#include <string_view>

#include <foxlox/except.h>
#include <foxlox/chunk.h>

#include "common.h"
#include "stmt.h"
#include "expr.h"
#include "parser.h"

namespace foxlox
{
  class CodeGen : public expr::IVisitor<void>, public stmt::IVisitor<void>
  {
  public:
    explicit CodeGen(AST&& a);
    Chunk gen(std::string_view src_name);
    bool get_had_error() noexcept;
  private:
    AST ast;
    Chunk chunk;
    int current_line;
    uint16_t current_subroutine_idx;
    std::string_view source_name;

    Subroutine& current_subroutine();

    bool had_error;
    void error(Token token, std::string_view message);

    struct ValueIdx
    {
      stmt::VarStoreType type;
      uint16_t idx;
    };
    std::map<VarDeclareAt, ValueIdx> value_idxs;
    uint16_t current_stack_size;
    void push_stack() noexcept;
    void pop_stack(uint16_t n = 1);
    // convert a stack idx between "idx from the stack bottom" and "idx from the stack top"
    uint16_t idx_cast(uint16_t idx) noexcept;

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
    gsl::index emit_jump(OP c);
    void patch_jump(gsl::index ip, Token tk);
    void patch_jumps(std::vector<gsl::index>& ips, Token tk);
    gsl::index prepare_loop();
    void emit_loop(gsl::index ip, OP c, Token tk);
    void emit_pop_to(uint16_t stack_size_before);
    void pop_stack_to(uint16_t stack_size_before);
    std::vector<gsl::index> break_stmts;
    std::vector<gsl::index> continue_stmts;

    void declare_a_var(gsl::not_null<stmt::VarDeclareBase*> stmt);
    void declare_a_var_from_list(gsl::not_null<stmt::VarDeclareListBase*> stmt, int index);

    uint16_t gen_subroutine(gsl::not_null<stmt::Function*> stmt, stmt::Class* klass);

    void visit_binary_expr(gsl::not_null<expr::Binary*> expr) final;
    void visit_grouping_expr(gsl::not_null<expr::Grouping*> expr) final;
    void visit_tuple_expr(gsl::not_null<expr::Tuple*> expr) final;
    void visit_literal_expr(gsl::not_null<expr::Literal*> expr) final;
    void visit_unary_expr(gsl::not_null<expr::Unary*> expr) final;
    void visit_variable_expr(gsl::not_null<expr::Variable*> expr) final;
    void visit_assign_expr(gsl::not_null<expr::Assign*> expr) final;
    void visit_logical_expr(gsl::not_null<expr::Logical*> expr) final;
    void visit_call_expr(gsl::not_null<expr::Call*> expr) final;
    void visit_get_expr(gsl::not_null<expr::Get*> expr) final;
    void visit_set_expr(gsl::not_null<expr::Set*> expr) final;
    void visit_this_expr(gsl::not_null<expr::This*> expr) final;
    void visit_super_expr(gsl::not_null<expr::Super*> expr) final;

    void visit_expression_stmt(gsl::not_null<stmt::Expression*> stmt) final;
    void visit_var_stmt(gsl::not_null<stmt::Var*> stmt) final;
    void visit_block_stmt(gsl::not_null<stmt::Block*> stmt) final;
    void visit_if_stmt(gsl::not_null<stmt::If*> stmt) final;
    void visit_while_stmt(gsl::not_null<stmt::While*> stmt) final;
    void visit_function_stmt(gsl::not_null<stmt::Function*> stmt) final;
    void visit_return_stmt(gsl::not_null<stmt::Return*> stmt) final;
    void visit_break_stmt(gsl::not_null<stmt::Break*> stmt) final;
    void visit_continue_stmt(gsl::not_null<stmt::Continue*> stmt) final;
    void visit_class_stmt(gsl::not_null<stmt::Class*> stmt) final;
    void visit_for_stmt(gsl::not_null<stmt::For*> stmt) final;
    void visit_import_stmt(gsl::not_null<stmt::Import*> stmt) final;
    void visit_from_stmt(gsl::not_null<stmt::From*> stmt) final;
    void visit_export_stmt(gsl::not_null<stmt::Export*> stmt) final;
  };
}
