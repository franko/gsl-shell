local b = require 'nyanga.builder'

local AST = { }

function AST.expr_function(ast, args, body, proto, line)
    return b.functionExpression(args, body, proto.varargs, line)
end

function AST.local_function_decl(ast, name, args, body, proto, line)
    local id = ast:expr_var(name)
    return b.functionDeclaration(id, args, body, proto.varargs, true, line)
end

function AST.function_decl(ast, path, args, body, proto, line)
    if path.kind == 'Identifier' then
        return b.functionDeclaration(path, args, body, proto.varargs, false, line)
    else
        local fn = b.functionExpression(args, body, proto.varargs, line)
        return b.assignmentExpression({ path }, { fn }, line)
    end
end

function AST.chunk(ast, body, line)
    return b.chunk(body, line)
end

function AST.block_stmt(ast, body, line)
    return b.blockStatement(body, line)
end

function AST.local_decl(ast, vlist, exps, line)
    local ids = {}
    for k = 1, #vlist do ids[k] = ast:expr_var(vlist[k]) end
    return b.localDeclaration(ids, exps, line)
end

function AST.assignment_expr(ast, vlist, exps, line)
    return b.assignmentExpression(vlist, exps, line)
end

function AST.expr_index(ast, v, index, line)
    return b.memberExpression(v, index, true, line)
end

function AST.expr_property(ast, v, prop, line)
    local index = ast:expr_var(prop)
    return b.memberExpression(v, index, false, line)
end

function AST.expr_number(ast, n) return b.literal(n) end
function AST.expr_string(ast, s) return b.literal(s) end
function AST.expr_nil(ast) return b.literal(nil) end
function AST.expr_boolean(ast, v) return b.literal(v) end
function AST.expr_vararg(ast) return b.vararg() end

function AST.expr_table(ast, avals, hkeys, hvals, line)
    return b.table(avals, hkeys, hvals, line)
end

function AST.expr_unop(ast, op, v)
    return b.unaryExpression(op == 'TK_not' and 'not' or op, v)
end

function AST.expr_binop(ast, op, expa, expb)
    return b.binaryExpression(op, expa, expb)
end

function AST.expr_var(ast, s)
    return b.identifier(s)
end

function AST.expr_method_call(ast, v, key, args)
    error('NYI')
end

function AST.expr_function_call(ast, v, args)
    return b.callExpression(v, args)
end

function AST.return_stmt(ast, exps, line)
    return b.returnStatement(exps, line)
end

function AST.break_stmt(ast, line)
    return b.breakStatement(line)
end

function AST.label_stmt(ast, name, line)
    local label = ast:expr_var(name)
    return labelStatement(label, line)
end

function AST.new_statement_expr(ast, var, line)
    return b.expressionStatement(var, line)
end

function AST.if_stmt(ast, branches, else_branch, line)
    if #branches > 1 then error('NYI') end
    local test, cons = branches[1][1], branches[1][2]
    return b.ifStatement(test, cons, else_branch, line)
end

function AST.while_stmt(ast, cond, body, line)
    return b.whileStatement(cond, body, line)
end

function AST.repeat_stmt(ast, cond, body, line)
    return b.repeatStatement(cond, body, line)
end

function AST.for_stmt(ast, var, init, last, step, body, line)
    local for_init = b.forInit(var, init, line)
    return b.forStatement(for_init, last, step, body, line)
end

function AST.for_iter_stmt(ast, vars, exps, body, line)
    local init = b.forNames(vars, line)
    if #exps > 1 then error('NYI: iter with multiple expression list') end
    local iter = exps[1]
    return b.forInStatement(init, iter, body, line)
end

local ASTClass = { __index = AST }

local function new_ast()
    return setmetatable({ }, ASTClass)
end

return {New = new_ast}
