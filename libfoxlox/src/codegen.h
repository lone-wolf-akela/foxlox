#pragma once

#include <map>

#include "stmt.h"
#include "expr.h"
#include "chunk.h"
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
    std::vector<uint32_t> closure_stack;
    Closure& current_closure();

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
      current_closure().add_code(arg1, current_line);
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

    virtual void visit_binary_expr(expr::Binary* expr) override;
    virtual void visit_grouping_expr(expr::Grouping* expr) override;
    virtual void visit_tuple_expr(expr::Tuple* expr) override;
    virtual void visit_literal_expr(expr::Literal* expr) override;
    virtual void visit_unary_expr(expr::Unary* expr) override;
    virtual void visit_variable_expr(expr::Variable* expr) override;
    virtual void visit_assign_expr(expr::Assign* expr) override;
    virtual void visit_logical_expr(expr::Logical* expr) override;
    virtual void visit_call_expr(expr::Call* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    virtual void visit_get_expr(expr::Get* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    virtual void visit_set_expr(expr::Set* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    virtual void visit_this_expr(expr::This* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    virtual void visit_super_expr(expr::Super* expr) override { /*TODO*/ std::ignore = expr; assert(false); }

    virtual void visit_expression_stmt(stmt::Expression* stmt) override;
    virtual void visit_var_stmt(stmt::Var* stmt) override;
    virtual void visit_block_stmt(stmt::Block* stmt) override;
    virtual void visit_if_stmt(stmt::If* stmt) override;
    virtual void visit_while_stmt(stmt::While* stmt) override;
    virtual void visit_function_stmt(stmt::Function* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    virtual void visit_return_stmt(stmt::Return* stmt) override;
    virtual void visit_break_stmt(stmt::Break* stmt) override;
    virtual void visit_continue_stmt(stmt::Continue* stmt) override;
    virtual void visit_class_stmt(stmt::Class* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    virtual void visit_for_stmt(stmt::For* stmt) override;
  };
}
