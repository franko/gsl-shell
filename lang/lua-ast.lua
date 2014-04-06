local build = require('syntax').build

local function build_stmt(ast, kind, prop)
    local stmt = build(kind, prop)
    if #ast.gen_stmts > 0 then
        stmt.pre_stms = ast.gen_stmts
        ast.gen_stmts = { }
    end
    return stmt
end

local CONCAT_OP = '~'

local function ident(name, line)
    return build("Identifier", { name = name, line = line })
end

local function literal(value)
    return build("Literal", { value = value })
end

local function binop(op, left, right)
    return build("BinaryExpression", { operator = op, left = left, right = right })
end

local function logical(op, left, right)
    return build("LogicalExpression", { operator = op, left = left, right = right })
end

local function field(obj, name)
    return build("MemberExpression", { object = obj, property = ident(name), computed = false })
end

local function tget(obj, index)
    return build("MemberExpression", { object = obj, property = index, computed = true })
end

local function error_stmt(msg)
    local expr = build("CallExpression", { callee = ident("error"), arguments = { literal(msg) } })
    return build("ExpressionStatement", { expression = expr })
end

local function recollect_stmts(stmts)
    local ls = { }
    for i = 1, #stmts do
        local s = stmts[i]
        if s.pre_stms then
            for k = 1, #s.pre_stms do
                ls[#ls + 1] = s.pre_stms[k]
            end
            s.pre_stms = nil
        end
        ls[#ls + 1] = s
    end
    return ls
end

local AST = { }

local function func_decl(id, body, params, vararg, locald, firstline, lastline)
    return build("FunctionDeclaration", {
        id         = id,
        body       = body,
        params     = params,
        vararg     = vararg,
        locald     = locald,
        firstline  = firstline,
        lastline   = lastline,
    })
end

local function func_expr(body, params, vararg, firstline, lastline)
    return build("FunctionExpression", { body = body, params = params, vararg = vararg, firstline = firstline, lastline = lastline })
end

function AST.use_stmt(ast, name, line)
    local defs = ast.probe(name)
    if defs then
        local node = build("LocalDeclaration", { names = { }, expressions = { }, line = line })
        local mod = { vars = defs, id = ident(name), vids = node.names, exps = node.expressions }
        ast:fscope_register_use(mod)
        return node
    else
        error(string.format("invalid module name: \"%s\" in line %d", name, line))
    end
end

function AST.expr_function(ast, args, body, proto)
   return func_expr(body, args, proto.varargs, proto.firstline, proto.lastline)
end

function AST.local_function_decl(ast, name, args, body, proto)
    local id = ast:var_declare(name)
    return func_decl(id, body, args, proto.varargs, true, proto.firstline, proto.lastline)
end

function AST.function_decl(ast, path, args, body, proto)
    local fn = func_expr(body, args, proto.varargs, proto.firstline, proto.lastline)
    return build_stmt(ast, "AssignmentExpression", { left = { path }, right = { fn }, line = line })
end

function AST.chunk(ast, body, chunkname, firstline, lastline)
    local body_stmts = recollect_stmts(body)
    return build("Chunk", { body = body_stmts, chunkname = chunkname, firstline = firstline, lastline = lastline })
end

function AST.block_stmt(ast, body, firstline, lastline)
    local body_stmts = recollect_stmts(body)
    return build("BlockStatement", { body = body_stmts, firstline = firstline, lastline = lastline })
end

function AST.local_decl(ast, vlist, exps, line)
    local ids = {}
    for k = 1, #vlist do
        ids[k] = ast:var_declare(vlist[k])
    end
    return build_stmt(ast, "LocalDeclaration", { names = ids, expressions = exps, line = line })
end

function AST.assignment_expr(ast, vars, exps, line)
    ast:mark_mutable(vars)
    return build_stmt(ast, "AssignmentExpression", { left = vars, right = exps, line = line })
end

function AST.expr_index(ast, v, index, line)
    return build("MemberExpression", { object = v, property = index, computed = true, line = line })
end

local function bound_check(index, inf, sup, line)
    local clow = binop("<", index, inf)
    local cupp = binop(">", index, sup)
    local cond = logical("or", clow, cupp)
    local err_stmt = error_stmt("index out of bounds")
    local err_stmt_block = build("BlockStatement", { body = { err_stmt } })
    return build("IfStatement", { tests = { cond }, cons = { err_stmt_block }, line = line })
end

function AST:add_generated_stmt(stmt)
    self.gen_stmts[#self.gen_stmts+1] = stmt
end

function AST.expr_index_dual(ast, v, row, col, line)
    local one = literal(1)
    local rowcheck = bound_check(row, one, field(v, "size1"), line)
    local colcheck = bound_check(col, one, field(v, "size2"), line)
    ast:add_generated_stmt(rowcheck)
    ast:add_generated_stmt(colcheck)
    local index = binop("*", field(v, "tda"), binop("-", row, one))
    index = binop("+", index, binop("-", col, one))
    return tget(field(v, "data"), index)
end

function AST.expr_slice(ast, v, row_start, row_end, col_start, col_end)
    row_end = row_end or row_start
    col_end = col_end or col_start
    return build("MatrixSliceExpression", { object = v, row_start = row_start, row_end = row_end, col_start = col_start, col_end = col_end, line = line })
end

function AST.expr_property(ast, v, prop, line)
    local index = ident(prop, line)
    return build("MemberExpression", { object = v, property = index, computed = false, line = line })
end

function AST.literal(ast, val)
    return literal(val)
end

function AST.expr_vararg(ast)
    return build("Vararg", { })
end

function AST.expr_table(ast, avals, hkeys, hvals, line)
    return build("Table", { array_entries = avals, hash_keys = hkeys, hash_values = hvals, line = line })
end

function AST.expr_matrix(ast, ncols, terms, line)
    local nrows = #terms / ncols
    return build("Matrix", { terms = terms, ncols = ncols, nrows = nrows, line = line })
end

function AST.expr_unop(ast, op, v)
    return build("UnaryExpression", { operator = op, argument = v, line = line })
end

local function concat_append(ts, node)
    local n = #ts
    if node.kind == "ConcatenateExpression" then
        for k = 1, #node.terms do ts[n + k] = node.terms[k] end
    else
        ts[n + 1] = node
    end
end

function AST.expr_binop(ast, op, expa, expb)
    local binop_body = (op ~= CONCAT_OP and { operator = op, left = expa, right = expb, line = line })
    if binop_body then
        if op == 'and' or op == 'or' then
            return build("LogicalExpression", binop_body)
        else
            return build("BinaryExpression", binop_body)
        end
    else
        local terms = { }
        concat_append(terms, expa)
        concat_append(terms, expb)
        return build("ConcatenateExpression", { terms = terms, line = expa.line })
    end
end

function AST.identifier(ast, name)
    local found, mod = ast:lookup(name)
    if found == 'use' then
        ast:add_used_var(mod, name)
    end
    return ident(name)
end

function AST.expr_method_call(ast, v, key, args)
    local m = ident(key)
    return build("SendExpression", { receiver = v, method = m, arguments = args })
end

function AST.expr_function_call(ast, v, args)
    return build("CallExpression", { callee = v, arguments = args, line = line })
end

function AST.return_stmt(ast, exps, line)
    return build_stmt(ast, "ReturnStatement", { arguments = exps, line = line })
end

function AST.break_stmt(ast, line)
    return build_stmt(ast, "BreakStatement", { line = line })
end

function AST.label_stmt(ast, name, line)
    return build_stmt(ast, "LabelStatement", { label = name, line = line })
end

function AST.new_statement_expr(ast, expr, line)
    return build_stmt(ast, "ExpressionStatement", { expression = expr, line = line })
end

function AST.if_stmt(ast, tests, cons, else_branch, line)
    return build_stmt(ast, "IfStatement", { tests = tests, cons = cons, alternate = else_branch, line = line })
end

function AST.do_stmt(ast, body, line)
    return build_stmt(ast, "DoStatement", { body = body, line = line })
end

function AST.while_stmt(ast, test, body, line)
    return build_stmt(ast, "WhileStatement", { test = test, body = body, line = line })
end

function AST.repeat_stmt(ast, test, body, line)
    return build_stmt(ast, "RepeatStatement", { test = test, body = body, line = line })
end

function AST.for_stmt(ast, var, init, last, step, body, line)
    local for_init = build("ForInit", { id = var, value = init, line = line })
    return build_stmt(ast, "ForStatement", { init = for_init, last = last, step = step, body = body, line = line })
end

function AST.for_iter_stmt(ast, vars, exps, body, line)
    local names = build("ForNames", { names = vars, line = line })
    return build_stmt(ast, "ForInStatement", { namelist = names, explist = exps, body = body, line = line })
end

function AST.goto_stmt(ast, name, line)
    return build_stmt(ast, "GotoStatement", { label = name, line = line })
end

local function new_scope(parent_scope)
    return {
        vars = { },
        parent = parent_scope,
    }
end

function AST.var_declare(ast, name)
    local id = ident(name)
    local vinfo = { id = id, mutable = false }
    ast.current.vars[name] = vinfo
    return id
end

function AST.fscope_begin(ast)
    ast.current = new_scope(ast.current)
end

function AST.fscope_end(ast)
    ast.current = ast.current.parent
end

function AST.lookup(ast, name)
    local current = ast.current
    while current do
        if current.vars[name] then return 'local', current end
        if current.imports then
            for k = 1, #current.imports do
                local mod = current.imports[k]
                if mod.vars[name] then return 'use', mod end
            end
        end
        current = current.parent
    end
    return false
end

function AST.add_used_var(ast, mod, name)
    local vids = mod.vids
    for k = 1, #vids do
        if vids[k].name == name then return end
    end
    local var_id = ident(name)
    mod.vids[#mod.vids + 1] = var_id
    mod.exps[#mod.exps + 1] = build("MemberExpression", { object = mod.id, property = var_id, computed = false })
end

function AST.mark_mutable(ast, vars)
    for i = 1, #vars do
        local v = vars[i]
        if v.kind == "Identifier" then
            local usage, scope = ast:lookup(v.name)
            if usage == "local" then
                local vinfo = scope.vars[v.name]
                vinfo.mutable = true
            end
        end
    end
end

function AST.fscope_register_use(ast, mod)
    if not ast.current.imports then ast.current.imports = { } end
    ast.current.imports[#ast.current.imports + 1] = mod
end

local ASTClass = { __index = AST }

local function new_ast()
    local ast = { gen_stmts = { } }
    return setmetatable(ast, ASTClass)
end

return { New = new_ast }
