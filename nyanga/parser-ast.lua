
local AST = { }

function AST.new_block(ast, line)
    print('New block', line)
end

function AST.add_block_stmt(ast, stmt, islast, line)
    print('Add block statement', stmt, islast, line)
end

function AST.local_decl(ast, vlist, exps, line)
    print('Local decraratiions: ', vlist, exps, line)
end

function AST.assignment_expr(ast, vlist, exps, line)
    print('Assignment ', vlist, exps, line)
end

function AST.expr_index(ast, v, index)
	print('Expression Index', v, index)
end

function AST.expr_number(ast, n) print('Number', n) end
function AST.expr_string(ast, s) print('String', s) end
function AST.expr_nil(ast, ) print('Nil') end
function AST.expr_boolean(ast, b) print('Boolean', b) end
function AST.expr_vararg(ast, ) print('Var Arg') end

function AST.expr_unop(ast, op, v)
	print('Unary Expression ' .. op, v)
end

function AST.expr_binop(ast, op, a, b)
	print('Binary Expression ' .. op, a, b)
end

function AST.expr_var(ast, s) print('Variable', s) end

function AST.expr_method_call(ast, v, key, args)
	print('Method Call', v, key, args)
end

function AST.expr_function_call(ast, v, args)
	print('Function Call', v, key, args)
end

function AST.return_stmt(ast, exps, line)
	print('Return ', exps)
end

function AST.new_statement_expr(ast, var, line)
	print('Statement Expression', var)
end

function AST.if_stmt(ast, branches, else_branch, line)
	print('If statement', branches, else_branch)
end

function AST.while_stmt(cond, body, line)
	print('While statement', cond, body)
end
