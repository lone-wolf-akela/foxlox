#ifndef FOXLOX_CODEGEN_H
#define FOXLOX_CODEGEN_H

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

    void compile_expr(const expr::Expr* expr);
    void compile_stmt(const stmt::Stmt* stmt);

    template<typename ... Args>
    void emit(Args ... args)
    {
      chunk.add_code(Inst(std::forward<Args>(args) ...), current_line);
    }

    void visit_binary_expr(const expr::Binary* expr) override;
    void visit_grouping_expr(const expr::Grouping* expr) override;
    void visit_literal_expr(const expr::Literal* expr) override;
    void visit_unary_expr(const expr::Unary* expr) override;
    void visit_variable_expr(const expr::Variable* expr) override;
    void visit_assign_expr(const expr::Assign* expr) override;
    void visit_logical_expr(const expr::Logical* expr) override;
    void visit_call_expr(const expr::Call* expr) override;
    void visit_get_expr(const expr::Get* expr) override;
    void visit_set_expr(const expr::Set* expr) override;
    void visit_this_expr(const expr::This* expr) override;
    void visit_super_expr(const expr::Super* expr) override;

    void visit_expression_stmt(const stmt::Expression* stmt) override;
    void visit_var_stmt(const stmt::Var* stmt) override;
    void visit_block_stmt(const stmt::Block* stmt) override;
    void visit_if_stmt(const stmt::If* stmt) override;
    void visit_while_stmt(const stmt::While* stmt) override;
    void visit_function_stmt(const stmt::Function* stmt) override;
    void visit_return_stmt(const stmt::Return* stmt) override;
    void visit_break_stmt(const stmt::Break* stmt) override;
    void visit_continue_stmt(const stmt::Continue* stmt) override;
    void visit_class_stmt(const stmt::Class* stmt) override;
    void visit_for_stmt(const stmt::For* stmt) override;
  };
}

#endif // FOXLOX_CODEGEN_H