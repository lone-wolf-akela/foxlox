#ifndef FOXLOX_RESOLVER_H
#define FOXLOX_RESOLVER_H

#include "parser.h"

namespace foxlox
{
  class Resolver : public expr::IVisitor<void>, public stmt::IVisitor<void>
  {
  public:
    Resolver(AST&& a);
    AST resolve();
  private:
    AST ast;

    enum class LoopType { NONE, WHILE, FOR } current_loop;
    enum class FunctionType { NONE, FUNCTION, METHOD, INITIALIZER } current_function;
    enum class ClassType { NONE, CLASS } current_class;

    void resolve_expr(const expr::Expr* expr);
    void resolve_stmt(const stmt::Stmt* stmt);

    void visit_binary_expr(const expr::Binary* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_grouping_expr(const expr::Grouping* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_literal_expr(const expr::Literal* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_unary_expr(const expr::Unary* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_variable_expr(const expr::Variable* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_assign_expr(const expr::Assign* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_logical_expr(const expr::Logical* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_call_expr(const expr::Call* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_get_expr(const expr::Get* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_set_expr(const expr::Set* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_this_expr(const expr::This* expr) override { /*TODO*/ std::ignore = expr; assert(false); }
    void visit_super_expr(const expr::Super* expr) override { /*TODO*/ std::ignore = expr; assert(false); }

    void visit_expression_stmt(const stmt::Expression* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_var_stmt(const stmt::Var* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_block_stmt(const stmt::Block* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_if_stmt(const stmt::If* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_while_stmt(const stmt::While* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_function_stmt(const stmt::Function* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_return_stmt(const stmt::Return* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_break_stmt(const stmt::Break* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_continue_stmt(const stmt::Continue* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_class_stmt(const stmt::Class* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
    void visit_for_stmt(const stmt::For* stmt) override { /*TODO*/ std::ignore = stmt; assert(false); }
  };
}

#endif // RESOLVER