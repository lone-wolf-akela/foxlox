#pragma once

#include <map>
#include <vector>
#include <string>

#include "expr.h"
#include "stmt.h"
#include "parser.h"

namespace foxlox
{
  struct ValueInfo
  {
    bool is_ready;
    VarDeclareAt declare;
  };

  struct Scope
  {
    int function_level; // how many layer of nested function are we in?
    std::map<std::string, ValueInfo> vars; // name : info
  };

  class Resolver : public expr::IVisitor<void>, public stmt::IVisitor<void>
  {
  public:
    Resolver(AST&& a);
    AST resolve();
    bool get_had_error();
  private:
    AST ast;
    bool had_error;
    std::vector<Scope> scopes;
    
    enum class LoopType { NONE, WHILE, FOR } current_loop;
    enum class FunctionType { NONE, FUNCTION, METHOD, INITIALIZER } current_function;
    enum class ClassType { NONE, CLASS, SUBCLASS } current_class;

    void error(Token token, std::string_view message);

    void resolve(expr::Expr* expr);
    void resolve(stmt::Stmt* stmt);
    void resolve(std::vector<std::unique_ptr<stmt::Stmt>>& stmts);

    void begin_scope(bool is_new_function);
    void end_scope();
    
    [[nodiscard]] ValueInfo* declare(Token name);
    void declare_from_varstmt(stmt::Var* stmt);
    void declare_from_functionparam(stmt::Function* stmt, int param_index);
    void declare_from_class(stmt::Class* stmt);
    void declare_from_functionname(stmt::Function* stmt);
    void define(Token name);
    [[nodiscard]] VarDeclareAt resolve_local(Token name);
    void resolve_function(stmt::Function* function, FunctionType type);

    void visit_binary_expr(expr::Binary* expr) override;
    void visit_grouping_expr(expr::Grouping* expr) override;
    void visit_literal_expr(expr::Literal* expr) override;
    void visit_unary_expr(expr::Unary* expr) override;
    void visit_variable_expr(expr::Variable* expr) override;
    void visit_assign_expr(expr::Assign* expr) override;
    void visit_logical_expr(expr::Logical* expr) override;
    void visit_call_expr(expr::Call* expr) override;
    void visit_get_expr(expr::Get* expr) override;
    void visit_set_expr(expr::Set* expr) override;
    void visit_this_expr(expr::This* expr) override;
    void visit_super_expr(expr::Super* expr) override;

    void visit_expression_stmt(stmt::Expression* stmt) override;
    void visit_var_stmt(stmt::Var* stmt) override;
    void visit_block_stmt(stmt::Block* stmt) override;
    void visit_if_stmt(stmt::If* stmt) override;
    void visit_while_stmt(stmt::While* stmt) override;
    void visit_function_stmt(stmt::Function* stmt) override;
    void visit_return_stmt(stmt::Return* stmt) override;
    void visit_break_stmt(stmt::Break* stmt) override;
    void visit_continue_stmt(stmt::Continue* stmt) override;
    void visit_class_stmt(stmt::Class* stmt) override;
    void visit_for_stmt(stmt::For* stmt) override;
  };
}
