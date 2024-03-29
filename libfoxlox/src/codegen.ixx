module;
export module foxlox:codegen;

import <map>;
import <string_view>;
import <format>;
import <ranges>;

import <gsl/gsl>;

import :except;
import :chunk;
import :stmt;
import :expr;
import :parser;
import :format_error;

namespace foxlox
{
  export class CodeGen : public expr::IVisitor<void>, public stmt::IVisitor<void>
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
    void push_stack(uint16_t n = 1) noexcept;
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
      if constexpr (sizeof...(Args) >= 1)
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

    void declare_a_var_from_list(gsl::not_null<stmt::VarDeclareListBase*> stmt, gsl::index index);

    uint16_t gen_subroutine(gsl::not_null<stmt::Function*> stmt, stmt::Class* klass);

    void visit_binary_expr(gsl::not_null<expr::Binary*> expr) final;
    void visit_tupleunpack_expr(gsl::not_null<expr::TupleUnpack*> expr) final;
    void visit_noop_expr(gsl::not_null<expr::NoOP*> expr) noexcept final;
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

namespace foxlox
{
  CodeGen::CodeGen(AST&& a) :
    ast(std::move(a)), current_subroutine_idx{}
  {
    current_line = 1;
    current_stack_size = 0;
    loop_start_stack_size = 0;

    had_error = false;
  }
  Chunk CodeGen::gen(std::string_view src_name)
  {
    source_name = src_name;
    try
    {
      current_subroutine_idx = chunk.add_subroutine(source_name, 0);
    }
    catch (ChunkOperationError&)
    {
      throw FatalError("Fatal: unable to add the first subroutine.");
    }

    for (auto& stmt : ast)
    {
      compile(stmt.get());
    }

    current_line = -1; // <EOF>
    // emit_pop_to(0); // OP::RETURN will take charge of pop
    emit(OP::RETURN);
    return std::move(chunk);
  }

  bool CodeGen::get_had_error() noexcept
  {
    return had_error;
  }

  Subroutine& CodeGen::current_subroutine()
  {
    return chunk.get_subroutines().at(current_subroutine_idx);
  }

  void CodeGen::error(Token token, std::string_view message)
  {
    format_error(token, message);
    had_error = true;
  }

  void CodeGen::push_stack(uint16_t n) noexcept
  {
    current_stack_size += n;
  }
  void CodeGen::pop_stack(uint16_t n)
  {
    if (n > current_stack_size)
    {
      throw FatalError("Wrong stack size.");
    }
    current_stack_size -= n;
  }
  uint16_t CodeGen::idx_cast(uint16_t idx) noexcept
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
  gsl::index CodeGen::emit_jump(OP c)
  {
    emit(c);
    const auto ip = current_subroutine().get_code_num();
    emit(int16_t{});
    return ip;
  }
  void CodeGen::patch_jump(gsl::index ip, Token tk)
  {
    const auto jump_length = current_subroutine().get_code_num() - ip - 2;
    if (jump_length < 0)
    {
      throw FatalError("Wrong jump length.");
    }
    if (jump_length > std::numeric_limits<int16_t>::max())
    {
      error(tk, "Jump length is too long.");
    }
    current_subroutine().edit_code(ip, gsl::narrow_cast<int16_t>(jump_length));
  }
  void CodeGen::patch_jumps(std::vector<gsl::index>& ips, Token tk)
  {
    for (auto ip : ips)
    {
      patch_jump(ip, tk);
    }
    ips.clear();
  }
  gsl::index CodeGen::prepare_loop()
  {
    return current_subroutine().get_code_num();
  }
  void CodeGen::emit_loop(gsl::index ip, OP c, Token tk)
  {
    emit(c);
    const auto jump_length = ip - current_subroutine().get_code_num() - 2;
    if (jump_length > 0)
    {
      throw FatalError("Wrong jump length.");
    }
    if (jump_length < std::numeric_limits<int16_t>::min())
    {
      error(tk, "Jump length is too long");
    }
    emit(gsl::narrow_cast<int16_t>(jump_length));
  }
  void CodeGen::emit_pop_to(uint16_t stack_size_before)
  {
    if (current_stack_size < stack_size_before)
    {
      throw FatalError("Wrong stack size.");
    }
    const uint16_t new_stack_elem_num = current_stack_size - stack_size_before;
    if (new_stack_elem_num > 1)
    {
      emit(OP::POP_N, new_stack_elem_num);
    }
    else if (new_stack_elem_num == 1)
    {
      emit(OP::POP);
    }
  }
  void CodeGen::pop_stack_to(uint16_t stack_size_before)
  {
    if (current_stack_size < stack_size_before)
    {
      throw FatalError("Wrong stack size.");
    }
    const uint16_t new_stack_elem_num = current_stack_size - stack_size_before;
    pop_stack(new_stack_elem_num);
  }
  void CodeGen::visit_binary_expr(gsl::not_null<expr::Binary*> expr)
  {
    current_line = expr->op.line;

    compile(expr->left.get());
    compile(expr->right.get());
    switch (expr->op.type)
    {
    case TokenType::MINUS:
      emit(OP::SUBTRACT);
      break;
    case TokenType::SLASH:
      emit(OP::DIVIDE);
      break;
    case TokenType::STAR:
      emit(OP::MULTIPLY);
      break;
    case TokenType::PLUS:
      emit(OP::ADD);
      break;
    case TokenType::SLASH_SLASH:
      emit(OP::INTDIV);
      break;
    case TokenType::GREATER:
      emit(OP::GT);
      break;
    case TokenType::GREATER_EQUAL:
      emit(OP::GE);
      break;
    case TokenType::LESS:
      emit(OP::LT);
      break;
    case TokenType::LESS_EQUAL:
      emit(OP::LE);
      break;
    case TokenType::BANG_EQUAL:
      emit(OP::NE);
      break;
    case TokenType::EQUAL_EQUAL:
      emit(OP::EQ);
      break;
    default:
      throw FatalError("Unknown binary op.");
    }
    pop_stack();
  }
  void CodeGen::visit_tupleunpack_expr(gsl::not_null<expr::TupleUnpack*> expr)
  {
    compile(expr->tuple.get());
    const uint16_t tuple_size = gsl::narrow_cast<uint16_t>(expr->assignlist.size());
    emit(OP::UNPACK, tuple_size);
    push_stack(tuple_size);
    for (auto& e : expr->assignlist | std::views::reverse)
    {
      compile(e.get());
      emit(OP::POP);
      pop_stack();
    }
  }
  void CodeGen::visit_noop_expr(gsl::not_null<expr::NoOP*> /*expr*/) noexcept
  {
    // do nothing
  }
  void CodeGen::visit_grouping_expr(gsl::not_null<expr::Grouping*> expr)
  {
    compile(expr->expression.get());
  }
  void CodeGen::visit_tuple_expr(gsl::not_null<expr::Tuple*> expr)
  {
    for (auto& e : expr->exprs)
    {
      compile(e.get());
    }
    const uint16_t tuple_size = gsl::narrow_cast<uint16_t>(expr->exprs.size());
    emit(OP::TUPLE, tuple_size);
    pop_stack(tuple_size);
    push_stack();
  }
  void CodeGen::visit_literal_expr(gsl::not_null<expr::Literal*> expr)
  {
    auto& v = expr->value.v;
    try
    {
      if (std::holds_alternative<std::nullptr_t>(v))
      {
        emit(OP::NIL);
      }
      else if (std::holds_alternative<double>(v))
      {
        const uint16_t constant = chunk.add_constant(std::get<double>(v));
        emit(OP::CONSTANT, constant);
      }
      else if (std::holds_alternative<int64_t>(v))
      {
        const uint16_t constant = chunk.add_constant(std::get<int64_t>(v));
        emit(OP::CONSTANT, constant);
      }
      else if (std::holds_alternative<std::string>(v))
      {
        const uint16_t str_index = chunk.add_string(std::get<std::string>(v));
        emit(OP::STRING, str_index);
      }
      else if (std::holds_alternative<bool>(v))
      {
        emit(OP::BOOL, std::get<bool>(v));
      }
      else
      {
        throw FatalError("Unknown literal type.");
      }
    }
    catch (const ChunkOperationError& e)
    {
      error(expr->token, e.what());
    }
    push_stack();
  }
  void CodeGen::visit_unary_expr(gsl::not_null<expr::Unary*> expr)
  {
    current_line = expr->op.line;

    compile(expr->right.get());
    switch (expr->op.type)
    {
    case TokenType::MINUS:
      emit(OP::NEGATE);
      break;
    case TokenType::BANG:
      emit(OP::NOT);
      break;
    default:
      throw FatalError("Unknown unary op.");
    }
  }
  void CodeGen::visit_variable_expr(gsl::not_null<expr::Variable*> expr)
  {
    current_line = expr->name.line;

    const auto info = value_idxs.at(expr->declare);
    if (info.type == stmt::VarStoreType::Stack)
    {
      emit(OP::LOAD_STACK, idx_cast(info.idx));
    }
    else
    {
      current_subroutine().add_referenced_static_value(info.idx);
      emit(OP::LOAD_STATIC, info.idx);
    }
    push_stack();
  }
  void CodeGen::visit_assign_expr(gsl::not_null<expr::Assign*> expr)
  {
    current_line = expr->name.line;

    compile(expr->value.get());
    if (expr->name.type == TokenType::UNDERLINE)
    {
      // assign to a placeholder -> do nothing
      return;
    }
    const auto info = value_idxs.at(expr->declare);
    if (info.type == stmt::VarStoreType::Stack)
    {
      emit(OP::STORE_STACK, idx_cast(info.idx));
    }
    else
    {
      current_subroutine().add_referenced_static_value(info.idx);
      emit(OP::STORE_STATIC, info.idx);
    }
  }
  void CodeGen::visit_logical_expr(gsl::not_null<expr::Logical*> expr)
  {
    current_line = expr->op.line;
    compile(expr->left.get());
    if (expr->op.type == TokenType::OR)
    {
      const auto jump = emit_jump(OP::JUMP_IF_TRUE_NO_POP);
      pop_stack();
      emit(OP::POP);
      compile(expr->right.get());
      patch_jump(jump, expr->op);
    }
    else // TokenType::AND
    {
      const auto jump = emit_jump(OP::JUMP_IF_FALSE_NO_POP);
      pop_stack();
      emit(OP::POP);
      compile(expr->right.get());
      patch_jump(jump, expr->op);
    }
  }
  void CodeGen::visit_call_expr(gsl::not_null<expr::Call*> expr)
  {
    current_line = expr->paren.line;

    const auto enclosing_stack_size = current_stack_size;
    for (auto& e : expr->arguments)
    {
      compile(e.get());
    }
    compile(expr->callee.get());
    emit(OP::CALL, gsl::narrow_cast<uint16_t>(expr->arguments.size()));
    pop_stack_to(enclosing_stack_size + 1); // + 1 to store return value
  }
  void CodeGen::visit_get_expr(gsl::not_null<expr::Get*> expr)
  {
    compile(expr->obj.get());
    try
    {
      const uint16_t str_index = chunk.add_string(expr->name.lexeme);
      emit(OP::GET_PROPERTY, str_index);
    }
    catch (const ChunkOperationError& e)
    {
      error(expr->name, e.what());
    }
  }
  void CodeGen::visit_set_expr(gsl::not_null<expr::Set*> expr)
  {
    compile(expr->value.get());
    compile(expr->obj.get());
    try
    {
      const uint16_t str_index = chunk.add_string(expr->name.lexeme);
      emit(OP::SET_PROPERTY, str_index);
    }
    catch (const ChunkOperationError& e)
    {
      error(expr->name, e.what());
    }
    pop_stack();
  }
  void CodeGen::visit_this_expr(gsl::not_null<expr::This*> expr)
  {
    current_line = expr->keyword.line;

    const auto info = value_idxs.at(expr->declare);
    if (info.type != stmt::VarStoreType::Stack)
    {
      throw FatalError("Wrong store type for `this'.");
    }
    emit(OP::LOAD_STACK, idx_cast(info.idx));
    push_stack();
  }
  void CodeGen::visit_super_expr(gsl::not_null<expr::Super*> expr)
  {
    current_line = expr->keyword.line;

    const auto info = value_idxs.at(expr->declare);
    if (info.type != stmt::VarStoreType::Stack)
    {
      throw FatalError("Wrong store type for `super'.");
    }

    emit(OP::LOAD_STACK, idx_cast(info.idx));
    push_stack();
    try
    {
      const uint16_t str_index = chunk.add_string(expr->method.lexeme);
      emit(OP::GET_SUPER_METHOD, str_index);
    }
    catch (const ChunkOperationError& e)
    {
      error(expr->method, e.what());
    }
  }
  void CodeGen::visit_expression_stmt(gsl::not_null<stmt::Expression*> stmt)
  {
    compile(stmt->expression.get());
    pop_stack();
    emit(OP::POP);
  }
  void CodeGen::visit_var_stmt(gsl::not_null<stmt::Var*> stmt)
  {
    for (gsl::index i = 0; i < ssize(stmt->vars); i++)
    {
      current_line = stmt->vars.at(i).name.line;
      if (auto init = stmt->initializers.at(i).get(); init != nullptr)
      {
        compile(init);
      }
      else
      {
        emit(OP::NIL);
        push_stack();
      }
      if (stmt->vars.at(i).name.type == TokenType::UNDERLINE)
      {
        // assign to a placeholder -> drop the value
        pop_stack();
        emit(OP::POP);
      }
      else
      {
        declare_a_var_from_list(stmt, i);
      }
      if (auto e = stmt->tuple_unpacks.at(i).get(); e != nullptr)
      {
        compile(e);
      }
    }
  }
  void CodeGen::visit_block_stmt(gsl::not_null<stmt::Block*> stmt)
  {
    const uint16_t stack_size_before = current_stack_size;
    for (auto& s : stmt->statements)
    {
      compile(s.get());
    }
    emit_pop_to(stack_size_before);
    pop_stack_to(stack_size_before);
  }
  void CodeGen::visit_if_stmt(gsl::not_null<stmt::If*> stmt)
  {
    current_line = stmt->right_paren.line;

    compile(stmt->condition.get());
    const auto then_jump_ip = emit_jump(OP::JUMP_IF_FALSE);
    pop_stack();

    compile(stmt->then_branch.get());

    if (stmt->else_branch.get() != nullptr)
    {
      const auto else_jump_ip = emit_jump(OP::JUMP);

      patch_jump(then_jump_ip, stmt->right_paren);

      compile(stmt->else_branch.get());

      patch_jump(else_jump_ip, stmt->right_paren);
    }
    else
    {
      patch_jump(then_jump_ip, stmt->right_paren);
    }
  }
  void CodeGen::visit_while_stmt(gsl::not_null<stmt::While*> stmt)
  {
    current_line = stmt->right_paren.line;

    const auto start = prepare_loop();

    compile(stmt->condition.get());

    const auto jump_to_end = emit_jump(OP::JUMP_IF_FALSE);
    pop_stack();

    const auto enclosing_loop_start_stack_size = loop_start_stack_size;
    loop_start_stack_size = current_stack_size;

    compile(stmt->body.get());

    patch_jumps(continue_stmts, stmt->right_paren);
    emit_loop(start, OP::JUMP, stmt->right_paren);
    patch_jumps(break_stmts, stmt->right_paren);
    patch_jump(jump_to_end, stmt->right_paren);

    loop_start_stack_size = enclosing_loop_start_stack_size;
  }

  void CodeGen::declare_a_var_from_list(gsl::not_null<stmt::VarDeclareListBase*> stmt, gsl::index index)
  {
    current_line = stmt->vars.at(index).name.line;
    if (stmt->vars.at(index).store_type == stmt::VarStoreType::Stack)
    {
      // no code here, just let it stays in stack
      value_idxs.insert_or_assign(
        VarDeclareFromList{ stmt, index },
        ValueIdx{ stmt::VarStoreType::Stack, gsl::narrow_cast<uint16_t>(current_stack_size - 1) }
      );
    }
    else
    {
      try
      {
        const uint16_t alloc_idx = chunk.add_static_value();
        emit(OP::STORE_STATIC, alloc_idx);
        pop_stack();
        emit(OP::POP);
        value_idxs.insert_or_assign(
          VarDeclareFromList{ stmt, index },
          ValueIdx{ stmt::VarStoreType::Static, alloc_idx }
        );
        current_subroutine().add_referenced_static_value(alloc_idx);
      }
      catch (const ChunkOperationError& e)
      {
        error(stmt->vars.at(index).name, e.what());
      }
    }
  }

  uint16_t CodeGen::gen_subroutine(gsl::not_null<stmt::Function*> stmt, stmt::Class* klass)
  {
    current_line = stmt->vars.at(0).name.line;
    try
    {
      const uint16_t subroutine_idx =
        chunk.add_subroutine(std::format("{}:{}", source_name, stmt->vars.at(0).name.lexeme), gsl::narrow_cast<int>(ssize(stmt->vars) - 1));

      const auto stack_size_before = current_stack_size;

      for (gsl::index i = 1; i < ssize(stmt->vars); i++)
      {
        auto var = stmt->vars.at(i);
        push_stack();
        if (var.store_type == stmt::VarStoreType::Stack)
        {
          value_idxs.insert_or_assign(
            VarDeclareFromList{ stmt, gsl::narrow_cast<int>(i) },
            ValueIdx{ stmt::VarStoreType::Stack, gsl::narrow_cast<uint16_t>(current_stack_size - 1) }
          );
        }
        else
        {
          try
          {
            const uint16_t alloc_idx = chunk.add_static_value();
            value_idxs.insert_or_assign(
              VarDeclareFromList{ stmt, gsl::narrow_cast<int>(i) },
              ValueIdx{ stmt::VarStoreType::Static, alloc_idx }
            );
            current_subroutine().add_referenced_static_value(alloc_idx);
          }
          catch (const ChunkOperationError& e)
          {
            error(stmt->vars.at(i).name, e.what());
          }
        }
      }

      if (klass != nullptr) // this is a class method
      {
        // push `this' as a parameter to the stack
        push_stack();
        if (klass->this_store_type != stmt::VarStoreType::Stack)
        {
          throw FatalError("Wrong store type for `this'.");
        }
        value_idxs.insert_or_assign(
          ClassThisDeclare{ klass },
          ValueIdx{ stmt::VarStoreType::Stack, gsl::narrow_cast<uint16_t>(current_stack_size - 1) }
        );
      }

      const auto enclosing_subroutine_idx = current_subroutine_idx;
      current_subroutine_idx = subroutine_idx;

      // note: if one of the func args is a static value
      // we should do a store when the function is called
      for (gsl::index i = 1; i < ssize(stmt->vars); i++) // drop the func name
      {
        auto var = stmt->vars.at(i);
        if (var.store_type == stmt::VarStoreType::Static)
        {
          const uint16_t idx = value_idxs.at(VarDeclareFromList{ stmt, gsl::narrow_cast<int>(i) }).idx;
          const size_t param_num = (klass != nullptr) ? size(stmt->vars) : size(stmt->vars) - 1;
          emit(OP::LOAD_STACK, gsl::narrow_cast<uint16_t>(param_num - i));
          emit(OP::STORE_STATIC, idx);
          emit(OP::POP);
          current_subroutine().add_referenced_static_value(idx);
        }
      }
      for (auto& s : stmt->body)
      {
        compile(s.get());
      }
      // OP::RETURN will take charge of pop so we do not emit OP::POP here
      pop_stack_to(stack_size_before);
      current_subroutine_idx = enclosing_subroutine_idx;

      return subroutine_idx;
    }
    catch (const ChunkOperationError& e)
    {
      error(stmt->vars.at(0).name, e.what());
      return {};
    }
  }

  void CodeGen::visit_function_stmt(gsl::not_null<stmt::Function*> stmt)
  {
    // define earlier, store latter. for recursion

    current_line = stmt->vars.at(0).name.line;

    uint16_t alloc_idx{};
    if (stmt->vars.at(0).store_type == stmt::VarStoreType::Stack)
    {
      // no code here, just let it stays in stack
      push_stack();
      value_idxs.insert_or_assign(
        VarDeclareFromList{ stmt, 0 },
        ValueIdx{ stmt::VarStoreType::Stack, gsl::narrow_cast<uint16_t>(current_stack_size - 1) }
      );
    }
    else
    {
      try
      {
        alloc_idx = chunk.add_static_value();
      }
      catch (const ChunkOperationError& e)
      {
        error(stmt->vars.at(0).name, e.what());
      }
      value_idxs.insert_or_assign(
        VarDeclareFromList{ stmt, 0 },
        ValueIdx{ stmt::VarStoreType::Static, alloc_idx }
      );
      current_subroutine().add_referenced_static_value(alloc_idx);
    }

    const uint16_t subroutine_idx = gen_subroutine(stmt, nullptr);
    emit(OP::FUNC, subroutine_idx);

    current_line = stmt->vars.at(0).name.line;
    if (stmt->vars.at(0).store_type == stmt::VarStoreType::Stack)
    {
      // no code here, just let it stays in stack
    }
    else
    {
      emit(OP::STORE_STATIC, alloc_idx);
      emit(OP::POP);
    }
  }
  void CodeGen::visit_return_stmt(gsl::not_null<stmt::Return*> stmt)
  {
    current_line = stmt->keyword.line;
    if (stmt->value.get() != nullptr)
    {
      compile(stmt->value.get());
      emit(OP::RETURN_V);
      pop_stack();
    }
    else
    {
      emit(OP::RETURN);
    }
  }
  void CodeGen::visit_break_stmt(gsl::not_null<stmt::Break*> /*stmt*/)
  {
    // do not call pop_stack_to() here
    // as that should be done at the end of the block
    emit_pop_to(loop_start_stack_size);
    break_stmts.push_back(emit_jump(OP::JUMP));
  }
  void CodeGen::visit_continue_stmt(gsl::not_null<stmt::Continue*> /*stmt*/)
  {
    // do not call pop_stack_to() here
    // as that should be done at the end of the block
    emit_pop_to(loop_start_stack_size);
    continue_stmts.push_back(emit_jump(OP::JUMP));
  }
  void CodeGen::visit_for_stmt(gsl::not_null<stmt::For*> stmt)
  {
    current_line = stmt->right_paren.line;

    const auto stack_size_before_initializer = current_stack_size;

    compile(stmt->initializer.get());

    const auto start = prepare_loop();

    gsl::index jump_to_end{};
    if (stmt->condition.get() != nullptr)
    {
      compile(stmt->condition.get());
      jump_to_end = emit_jump(OP::JUMP_IF_FALSE);
      pop_stack();
    }

    const auto enclosing_loop_start_stack_size = loop_start_stack_size;
    loop_start_stack_size = current_stack_size;

    compile(stmt->body.get());

    patch_jumps(continue_stmts, stmt->right_paren);
    if (stmt->increment.get() != nullptr)
    {
      compile(stmt->increment.get());
      pop_stack();
      emit(OP::POP);
    }
    emit_loop(start, OP::JUMP, stmt->right_paren);
    patch_jumps(break_stmts, stmt->right_paren);
    if (stmt->condition.get() != nullptr)
    {
      patch_jump(jump_to_end, stmt->right_paren);
    }
    loop_start_stack_size = enclosing_loop_start_stack_size;

    emit_pop_to(stack_size_before_initializer);
    pop_stack_to(stack_size_before_initializer);
  }
  void CodeGen::visit_import_stmt(gsl::not_null<stmt::Import*> stmt)
  {
    for (const auto& elem : stmt->libpath)
    {
      current_line = elem.line;
      const uint16_t str_index = chunk.add_string(elem.lexeme);
      emit(OP::STRING, str_index);
    }
    emit(OP::IMPORT, gsl::narrow_cast<uint16_t>(stmt->libpath.size()));
    push_stack();

    declare_a_var_from_list(stmt, 0);
  }
  void CodeGen::visit_from_stmt(gsl::not_null<stmt::From*> stmt)
  {
    for (const auto& elem : stmt->libpath)
    {
      current_line = elem.line;
      const uint16_t str_index = chunk.add_string(elem.lexeme);
      emit(OP::STRING, str_index);
    }
    emit(OP::IMPORT, gsl::narrow_cast<uint16_t>(stmt->libpath.size()));
    const auto lib_stack_idx = current_stack_size;
    push_stack();

    for(gsl::index i = 0; i < ssize(stmt->vars); i++)
    {
      auto var = stmt->vars.at(i);
      emit(OP::LOAD_STACK, idx_cast(lib_stack_idx));
      push_stack();
      const uint16_t str_index = chunk.add_string(var.name.lexeme);
      emit(OP::GET_PROPERTY, str_index);
      declare_a_var_from_list(stmt, gsl::narrow_cast<int>(i));
    }

    // the imported lib dict is not useful anymore
    // but we can not pop it from stack since that would change the position of latter variables
    // so we set it to nil to empty it
    emit(OP::NIL);
    push_stack();
    emit(OP::STORE_STACK, idx_cast(lib_stack_idx));
    emit(OP::POP);
    pop_stack();
  }
  void CodeGen::visit_export_stmt(gsl::not_null<stmt::Export*> stmt)
  {
    compile(stmt->declare.get());
    const auto declare = dynamic_cast<stmt::VarDeclareListBase*>(stmt->declare.get());
    if (declare == nullptr)
    {
      throw FatalError("Not a valid declaration in the `export' statement.");
    }
    const auto value_id = value_idxs.at(foxlox::VarDeclareFromList{ declare, 0 });
    if (value_id.type != stmt::VarStoreType::Static)
    {
      throw FatalError("Exported value must be static storage.");
    }
    const uint16_t alloc_idx = value_id.idx;
    chunk.add_export(declare->vars.at(0).name.lexeme, alloc_idx);
  }
  void CodeGen::visit_class_stmt(gsl::not_null<stmt::Class*> stmt)
  {
    current_line = stmt->vars.at(0).name.line;

    uint16_t alloc_idx{};
    if (stmt->vars.at(0).store_type == stmt::VarStoreType::Stack)
    {
      value_idxs.insert_or_assign(
        VarDeclareFromList{ stmt, 0 },
        ValueIdx{ stmt::VarStoreType::Stack, current_stack_size }
      );
    }
    else
    {
      try
      {
        alloc_idx = chunk.add_static_value();
      }
      catch (const ChunkOperationError& e)
      {
        error(stmt->vars.at(0).name, e.what());
      }
      value_idxs.insert_or_assign(
        VarDeclareFromList{ stmt, 0 },
        ValueIdx{ stmt::VarStoreType::Static, alloc_idx }
      );
      current_subroutine().add_referenced_static_value(alloc_idx);
    }

    CompiletimeClass klass(stmt->vars.at(0).name.lexeme);
    for (auto& method : stmt->methods)
    {
      try
      {
        const uint16_t subroutine_idx = gen_subroutine(method.get(), stmt);
        const uint16_t str_idx = chunk.add_string(method->vars.at(0).name.lexeme);
        klass.add_method(str_idx, subroutine_idx);
      }
      catch (const ChunkOperationError& e)
      {
        error(method->vars.at(0).name, e.what());
      }
    }

    current_line = stmt->vars.at(0).name.line;
    try
    {
      const uint16_t klass_idx = chunk.add_class(std::move(klass));
      push_stack();
      emit(OP::CLASS, klass_idx);
    }
    catch (const ChunkOperationError& e)
    {
      error(stmt->vars.at(0).name, e.what());
    }

    if (stmt->superclass.get() != nullptr)
    {
      compile(stmt->superclass.get());
      emit(OP::INHERIT);
      pop_stack();
    }

    if (stmt->vars.at(0).store_type == stmt::VarStoreType::Stack)
    {
      // no code here, just let it stays in stack
    }
    else
    {
      emit(OP::STORE_STATIC, alloc_idx);
      emit(OP::POP);
      pop_stack();
    }
  }
}