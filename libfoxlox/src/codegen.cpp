#include <cassert>

#include "codegen.h"

namespace foxlox
{
  CodeGen::CodeGen(AST&& a):
    ast(std::move(a))
  {
    current_line = 1;
    current_stack_size = 0;
    loop_start_stack_size = 0;
  }
  Chunk CodeGen::gen()
  {
    current_subroutine_idx = chunk.add_subroutine("script", 0);

    for (auto& stmt : ast)
    {
      compile(stmt.get());
    }

    current_line = -1; // <EOF>
    // emit_pop_to(0); // OP_RETURN will take charge of pop
    emit(OP_RETURN);
    return std::move(chunk);
  }

  Subroutine& CodeGen::current_subroutine()
  {
    return chunk.get_subroutines()[current_subroutine_idx];
  }

  void CodeGen::push_stack()
  {
    current_stack_size++;
  }
  void CodeGen::pop_stack(uint16_t n)
  {
    assert(n <= current_stack_size);
    current_stack_size -= n;
  }
  uint16_t CodeGen::idx_cast(uint16_t idx)
  {
    return current_stack_size - idx - 1;
  }
  void CodeGen::compile(expr::Expr* expr)
  {
    expr::IVisitor<void>::visit(expr);
  }
  void CodeGen::compile(stmt::Stmt* stmt)
  {
    stmt::IVisitor<void>::visit(stmt);
  }
  gsl::index CodeGen::emit_jump(OpCode c)
  {
    emit(c);
    const auto ip = current_subroutine().get_code_num();
    emit(int16_t{});
    return ip;
  }
  void CodeGen::patch_jump(gsl::index ip)
  {
    const auto jump_length = current_subroutine().get_code_num() - ip - 2;
    assert(jump_length <= std::numeric_limits<int16_t>::max());
    current_subroutine().edit_code(ip, gsl::narrow_cast<int16_t>(jump_length));
  }
  void CodeGen::patch_jumps(std::vector<gsl::index>& ips)
  {
    for (auto ip : ips)
    {
      patch_jump(ip);
    }
    ips.clear();
  }
  gsl::index CodeGen::prepare_loop()
  {
    return current_subroutine().get_code_num();
  }
  void CodeGen::emit_loop(gsl::index ip, OpCode c)
  {
    emit(c);
    const auto jump_length = ip - current_subroutine().get_code_num() - 2;
    assert(jump_length >= std::numeric_limits<int16_t>::min());
    emit(gsl::narrow_cast<int16_t>(jump_length));
  }
  void CodeGen::emit_pop_to(uint16_t stack_size_before)
  {
    assert(current_stack_size >= stack_size_before);
    const uint16_t new_stack_elem_num = current_stack_size - stack_size_before;
    if (new_stack_elem_num > 1)
    {
      emit(OP_POP_N, new_stack_elem_num);
    }
    else if (new_stack_elem_num == 1)
    {
      emit(OP_POP);
    }
  }
  void CodeGen::pop_stack_to(uint16_t stack_size_before)
  {
    assert(current_stack_size >= stack_size_before);
    const uint16_t new_stack_elem_num = current_stack_size - stack_size_before;
    pop_stack(new_stack_elem_num);
  }
  void CodeGen::visit_binary_expr(expr::Binary* expr)
  {
    current_line = expr->op.line;

    compile(expr->left.get());
    compile(expr->right.get());
    switch (expr->op.type)
    {
    case TokenType::MINUS:
      emit(OP_SUBTRACT);
      break;
    case TokenType::SLASH:
      emit(OP_DIVIDE);
      break;
    case TokenType::STAR:
      emit(OP_MULTIPLY);
      break;
    case TokenType::PLUS:
      emit(OP_ADD);
      break;
    case TokenType::SLASH_SLASH:
      emit(OP_INTDIV);
      break;
    case TokenType::GREATER:
      emit(OP_GT);
      break;
    case TokenType::GREATER_EQUAL:
      emit(OP_GE);
      break;
    case TokenType::LESS:
      emit(OP_LT);
      break;
    case TokenType::LESS_EQUAL:
      emit(OP_LE);
      break;
    case TokenType::BANG_EQUAL:
      emit(OP_NE);
      break;
    case TokenType::EQUAL_EQUAL: 
      emit(OP_EQ);
      break;
    default:
      assert(false);
    }
    pop_stack();
  }
  void CodeGen::visit_grouping_expr(expr::Grouping* expr)
  {
    compile(expr->expression.get());
  }
  void CodeGen::visit_tuple_expr(expr::Tuple* expr)
  {
    for (auto& e : expr->exprs)
    {
      compile(e.get());
    }
    const uint16_t tuple_size = gsl::narrow_cast<uint16_t>(expr->exprs.size());
    emit(OP_TUPLE, tuple_size);
    pop_stack(tuple_size);
    push_stack();
  }
  void CodeGen::visit_literal_expr(expr::Literal* expr)
  {
    auto& v = expr->value.v;
    if (std::holds_alternative<std::monostate>(v))
    {
      emit(OP_NIL);
    }
    else if (std::holds_alternative<double>(v))
    {
      const uint16_t constant = chunk.add_constant(Value(std::get<double>(v)));
      emit(OP_CONSTANT, constant);
    }
    else if (std::holds_alternative<int64_t>(v))
    {
      const int64_t i64 = std::get<int64_t>(v);
      const uint16_t constant = chunk.add_constant(Value(i64));
      emit(OP_CONSTANT, constant);
    }
    else if (std::holds_alternative<std::string>(v))
    {
      const uint16_t str_index = chunk.add_string(std::get<std::string>(v));
      emit(OP_STRING, str_index);
    }
    else if (std::holds_alternative<bool>(v))
    {
      emit(OP_BOOL, std::get<bool>(v));
    }
    else
    {
      assert(false);
    }
    push_stack();
  }
  void CodeGen::visit_unary_expr(expr::Unary* expr)
  {
    current_line = expr->op.line;

    compile(expr->right.get());
    switch (expr->op.type)
    {
    case TokenType::MINUS:
      emit(OP_NEGATE);
      break;
    case TokenType::BANG:
      emit(OP_NOT);
      break;
    default:
      assert(false);
    }
  }
  void CodeGen::visit_variable_expr(expr::Variable* expr)
  {
    current_line = expr->name.line;

    const auto info = value_idxs.at(expr->declare);
    if (info.type == stmt::VarStoreType::Stack)
    {
      emit(OP_LOAD_STACK, idx_cast(info.idx));
    }
    else
    {
      emit(OP_LOAD_STATIC, info.idx);
    }
    push_stack();
  }
  void CodeGen::visit_assign_expr(expr::Assign* expr)
  {
    current_line = expr->name.line;

    compile(expr->value.get());
    const auto info = value_idxs.at(expr->declare);
    if (info.type == stmt::VarStoreType::Stack)
    {
      emit(OP_STORE_STACK, idx_cast(info.idx));
    }
    else
    {
      emit(OP_STORE_STATIC, info.idx);
    }
  }
  void CodeGen::visit_logical_expr(expr::Logical* expr)
  {
    current_line = expr->op.line;
    compile(expr->left.get());
    if (expr->op.type == TokenType::OR)
    {
      const auto jump = emit_jump(OP_JUMP_IF_TRUE_NO_POP);
      pop_stack();
      emit(OP_POP);
      compile(expr->right.get());
      patch_jump(jump);
    }
    else // TokenType::AND
    {
      const auto jump = emit_jump(OP_JUMP_IF_FALSE_NO_POP);
      pop_stack();
      emit(OP_POP);
      compile(expr->right.get());
      patch_jump(jump);
    }
  }
  void CodeGen::visit_call_expr(expr::Call* expr)
  {
    compile(expr->callee.get());
    const auto enclosing_stack_size = current_stack_size;
    for (auto& e : expr->arguments)
    {
      compile(e.get());
    }
    emit(OP_CALL, gsl::narrow_cast<uint16_t>(expr->arguments.size()));
    pop_stack_to(enclosing_stack_size);
  }
  void CodeGen::visit_expression_stmt(stmt::Expression* stmt)
  {
    compile(stmt->expression.get());
    pop_stack();
    emit(OP_POP);
  }
  void CodeGen::visit_var_stmt(stmt::Var* stmt)
  {
    current_line = stmt->name.line;
    if (stmt->initializer.get() != nullptr)
    {
      compile(stmt->initializer.get());
    }
    else
    {
      emit(OP_NIL);
      push_stack();
    }

    if (stmt->store_type == stmt::VarStoreType::Stack)
    {
      // no code here, just let it stays in stack
      value_idxs[stmt] = ValueIdx{ stmt::VarStoreType::Stack, gsl::narrow_cast<uint16_t>(current_stack_size - 1) };
    }
    else
    {
      const uint16_t alloc_idx = chunk.add_static_value();
      emit(OP_STORE_STATIC, alloc_idx);
      pop_stack();
      emit(OP_POP);
      value_idxs[stmt] = ValueIdx{ stmt::VarStoreType::Static, alloc_idx };
    }
  }
  void CodeGen::visit_block_stmt(stmt::Block* stmt)
  {
    const uint16_t stack_size_before = current_stack_size;
    for (auto& s : stmt->statements)
    {
      compile(s.get());
    }
    emit_pop_to(stack_size_before);
    pop_stack_to(stack_size_before);
  }
  void CodeGen::visit_if_stmt(stmt::If* stmt)
  {
    compile(stmt->condition.get());
    const auto then_jump_ip = emit_jump(OP_JUMP_IF_FALSE);
    pop_stack();

    compile(stmt->then_branch.get());
    
    if (stmt->else_branch.get() != nullptr)
    {
      emit(OP_JUMP);
      const auto else_jump_ip = emit_jump(OP_JUMP);

      patch_jump(then_jump_ip);

      compile(stmt->else_branch.get());

      patch_jump(else_jump_ip);
    }
    else
    {
      patch_jump(then_jump_ip);
    }
  }
  void CodeGen::visit_while_stmt(stmt::While* stmt)
  {
    const auto start = prepare_loop();

    compile(stmt->condition.get());
    
    const auto jump_to_end = emit_jump(OP_JUMP_IF_FALSE);
    pop_stack();

    const auto enclosing_loop_start_stack_size = loop_start_stack_size;
    loop_start_stack_size = current_stack_size;

    compile(stmt->body.get());

    patch_jumps(continue_stmts);
    emit_loop(start, OP_JUMP);
    patch_jumps(break_stmts);
    patch_jump(jump_to_end);

    loop_start_stack_size = enclosing_loop_start_stack_size;
  }
  void CodeGen::visit_function_stmt(stmt::Function* stmt)
  {
    current_line = stmt->name.line;

    const uint16_t subroutine_idx =
      chunk.add_subroutine(stmt->name.lexeme, gsl::narrow_cast<int>(ssize(stmt->param)));
    emit(OP_FUNC, subroutine_idx);
    if (stmt->name_store_type == stmt::VarStoreType::Stack)
    {
      // no code here, just let it stays in stack
      push_stack();
      value_idxs[stmt] = ValueIdx{ stmt::VarStoreType::Stack, gsl::narrow_cast<uint16_t>(current_stack_size - 1) };
    }
    else
    {
      const uint16_t alloc_idx = chunk.add_static_value();
      emit(OP_STORE_STATIC, alloc_idx);
      emit(OP_POP);
      value_idxs[stmt] = ValueIdx{ stmt::VarStoreType::Static, alloc_idx };
    }

    for (gsl::index i = 0; i < ssize(stmt->param); i++)
    {
      if (stmt->param_store_types[i] == stmt::VarStoreType::Stack)
      {
        // no code here, just let it stays in stack
        push_stack();
        value_idxs[stmt] = ValueIdx{ stmt::VarStoreType::Stack, gsl::narrow_cast<uint16_t>(current_stack_size - 1) };
      }
      else
      {
        const uint16_t alloc_idx = chunk.add_static_value();
        value_idxs[stmt] = ValueIdx{ stmt::VarStoreType::Static, alloc_idx };
      }
    }

    const auto stack_size_before = current_stack_size;
    const auto enclosing_subroutine_idx = current_subroutine_idx;
    current_subroutine_idx = subroutine_idx;

    for (auto& s : stmt->body)
    {
      compile(s.get());
    }
    //emit_pop_to(stack_size_before); // emit_pop_to(0); // OP_RETURN will take charge of pop
    pop_stack_to(stack_size_before);
    // make sure we have a return at the end of a func
    emit(OP_RETURN);
    current_subroutine_idx = enclosing_subroutine_idx;
  }
  void CodeGen::visit_return_stmt(stmt::Return* stmt)
  {
    current_line = stmt->keyword.line;
    if (stmt->value.get() != nullptr)
    {
      compile(stmt->value.get());
      emit(OP_RETURN_V);
      pop_stack();
    }
    else
    {
      emit(OP_RETURN);
    }
  }
  void CodeGen::visit_break_stmt(stmt::Break* /*stmt*/)
  {
    // do not call pop_stack_to() here
    // as that should be done at the end of the block
    emit_pop_to(loop_start_stack_size);
    break_stmts.push_back(emit_jump(OP_JUMP));
  }
  void CodeGen::visit_continue_stmt(stmt::Continue* /*stmt*/)
  {
    // do not call pop_stack_to() here
    // as that should be done at the end of the block
    emit_pop_to(loop_start_stack_size);
    continue_stmts.push_back(emit_jump(OP_JUMP));
  }
  void CodeGen::visit_for_stmt(stmt::For* stmt)
  {
    const auto stack_size_before_initializer = current_stack_size;

    compile(stmt->initializer.get());

    const auto start = prepare_loop();

    gsl::index jump_to_end{};
    if (stmt->condition.get() != nullptr)
    {
      compile(stmt->condition.get());
      jump_to_end = emit_jump(OP_JUMP_IF_FALSE);
      pop_stack();
    }    

    const auto enclosing_loop_start_stack_size = loop_start_stack_size;
    loop_start_stack_size = current_stack_size;

    compile(stmt->body.get());

    patch_jumps(continue_stmts);
    if (stmt->increment.get() != nullptr)
    {
      compile(stmt->increment.get());
      pop_stack();
      emit(OP_POP);
    }
    emit_loop(start, OP_JUMP);
    patch_jumps(break_stmts);
    if (stmt->condition.get() != nullptr)
    {
      patch_jump(jump_to_end);
    }
    loop_start_stack_size = enclosing_loop_start_stack_size;

    emit_pop_to(stack_size_before_initializer);
    pop_stack_to(stack_size_before_initializer);
  }
}