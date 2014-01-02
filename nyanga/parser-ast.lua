
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
