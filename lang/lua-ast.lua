local syntax = require('syntax')
local libexpr = require('expr-utils')
local lua_interface = require('lua-interface')

local build, ident, literal, logical, binop, field, tget, empty_table = syntax.build, syntax.ident, syntax.literal, syntax.logical, syntax.binop, syntax.field, syntax.tget, syntax.empty_table

local function add_pre_stmts(stmt, pre_stmts)
    if not stmt.pre_stmts then stmt.pre_stmts = { } end
    local n = #stmt.pre_stmts
    for i = 1, #pre_stmts do
        stmt.pre_stmts[n + i] = pre_stmts[i]
    end
end

local function build_stmt(ast, kind, prop)
    local stmt = build(kind, prop)
    if #ast.gen_stmts > 0 then
        stmt.pre_stmts = ast:commit_generated_stmts()
    end
    return stmt
end

local function error_stmt(msg)
    local expr = build("CallExpression", { callee = ident("error"), arguments = { literal(msg) } })
    return build("ExpressionStatement", { expression = expr })
end

local function recollect_stmts(stmts)
    local ls = { }
    for i = 1, #stmts do
        local s = stmts[i]
        if s.pre_stmts then
            for k = 1, #s.pre_stmts do
                ls[#ls + 1] = s.pre_stmts[k]
            end
            s.pre_stmts = nil
        end
        ls[#ls + 1] = s
    end
    return ls
end

local function var_context(var, context)
    local ast, for_scope = context.ast, context.for_scope
    local name = var.name
    local result, scope = ast:lookup(name)
    local vinfo = result == "local" and scope.vars[name]
    if result == "local" and vinfo.mutable == false then
        if scope == for_scope then
            return true, false, vinfo.value
        else
            return false, true
        end
    end
    return false, false
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

local function interface_symbols(inodes)
    local defs = { }
    for i = 1, #inodes do
        local name = inodes[i].name
        defs[name] = true
    end
    return defs
end

function AST.use_stmt(ast, name, line)
    local inodes = lua_interface.read(name)
    local defs = interface_symbols(inodes)
    if defs then
        local node = build("LocalDeclaration", { names = { }, expressions = { }, line = line })
        local mod = { vars = defs, id = ast:genid(), vids = node.names, exps = node.expressions, name = name }
        local req = build("CallExpression", { callee = ident("require"), arguments = { literal(name) } })
        local mod_local = build("LocalDeclaration", { names = { mod.id }, expressions = { req } })
        add_pre_stmts(node, { mod_local })
        ast:fscope_register_use(mod)
        return node
    else
        error(string.format("invalid module name: \"%s\" in line %d", name, line))
    end
end

local function kargs_fetch_stmt(ast, keyargs, options_id)
    local knames, kfetch_args = {}, { options_id }
    for k = 1, #keyargs do
        local name = keyargs[k].parameter
        knames[k] = ast:var_declare(name)
        kfetch_args[2*k] = literal(name)
        kfetch_args[2*k + 1] = keyargs[k].default
    end
    local kfetch_call = build("CallExpression", { callee = field(ident("lang"), "__keyargs_options"), arguments = kfetch_args })
    return build("LocalDeclaration", { names = knames, expressions = { kfetch_call } })
end

local function func_decl_keyargs(ast, path, args, body, proto, assign_decl)
    local path_keyargs = field(path, "__keyargs")
    local options_id = ast:genid()
    table.insert(args, 1, options_id)
    local decl = func_decl(path_keyargs, body, args, proto.varargs, false, proto.firstline, proto.lastline)

    add_pre_stmts(decl, { assign_decl })

    local kargs_decl = kargs_fetch_stmt(ast, args.keyargs, options_id)
    table.insert(body, 1, kargs_decl)

    return decl
end

function AST.expr_function(ast, args, body, proto)
    if args.keyargs then
        local options_id = ast:genid()
        table.insert(args, 1, options_id)
        local kargs_decl = kargs_fetch_stmt(ast, args.keyargs, options_id)
        table.insert(body, 1, kargs_decl)
        local fn = func_expr(body, args, proto.varargs, proto.firstline, proto.lastline)
        local t = build("Table", { array_entries = {}, hash_keys = { literal("__keyargs") }, hash_values = { fn } })
        return build("CallExpression", { callee = ident("setmetatable"), arguments = { t, field(ident("lang"), "__keyargs_class") } })
    else
        return func_expr(body, args, proto.varargs, proto.firstline, proto.lastline)
    end
end

function AST.local_function_decl(ast, name, args, body, proto)
    local id = ast:var_declare(name)
    if args.keyargs then
        local call = build("CallExpression", { callee = ident("setmetatable"), arguments = { empty_table, field(ident("lang"), "__keyargs_class") } })
        local ldecl = build("LocalDeclaration", { names = { id }, expressions = { call } })
        return func_decl_keyargs(ast, id, args, body, proto, ldecl)
    else
        return func_decl(id, body, args, proto.varargs, true, proto.firstline, proto.lastline)
    end
end

-- object.foo = setmetatable({}, lang.__keyargs_class)
-- function object.foo.__keyargs(__options, x, y)
--     local linewidth, color = lang.__keyargs_options(__options, "linewidth", 1, "color", "red")
--     ...
-- end
function AST.function_decl(ast, path, args, body, proto)
    if args.keyargs then
        local call = build("CallExpression", { callee = ident("setmetatable"), arguments = { empty_table, field(ident("lang"), "__keyargs_class") } })
        local assign = build("AssignmentExpression", { left = { path }, right = { call } })
        return func_decl_keyargs(ast, path, args, body, proto, assign)
    else
        return func_decl(path, body, args, proto.varargs, false, proto.firstline, proto.lastline)
    end
end

function AST.chunk(ast, body, chunkname, firstline, lastline)
    local body_stmts = recollect_stmts(body)
    return build("Chunk", { body = body_stmts, chunkname = chunkname, firstline = firstline, lastline = lastline })
end

function AST.block_stmt(ast, body, firstline, lastline)
    return recollect_stmts(body)
end

function AST.local_decl(ast, vlist, exps, line)
    local ids = {}
    for k = 1, #vlist do
        ids[k] = ast:var_declare(vlist[k], exps[k])
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

local function bound_check(node, index, inf, sup, line)
    return build("CheckIndex", { index = index, inf = inf, sup = sup, reference = node, line = line })
end

function AST:add_generated_stmt(stmt)
    self.gen_stmts[#self.gen_stmts+1] = stmt
end

function AST:void_generated_stmts()
    self.gen_stmts = {}
end

-- Mark as "safe" the corresponding "MatrixIndex" reference node of each
-- generated statement and return the generated statements list. The gen_stmts
-- field of the AST object ("self") is set to the emty list.
-- We assume here that each generated statement is of type "CheckIndex" or
-- at least, have a "reference" node that accept a boolean "safe" attribute.
-- The logic is that the CheckIndex nodes are going to be taken into account
-- and so the MatrixIndex nodes can be considered safe to access the matrix
-- without checks.
function AST:commit_generated_stmts()
    local ls = self.gen_stmts
    for i = 1, #ls do
        local check = ls[i]
        check.reference.safe = true
    end
    self.gen_stmts = {}
    return ls
end

function AST.expr_index_dual(ast, v, row, col, line)
    local ctx_data = { ast = ast }
    local node = build("MatrixIndex", { object = v, row = row, col = col, safe = false, line = line})
    local one = literal(1)
    if libexpr.context_free(row, var_context, ctx_data) and libexpr.context_free(col, var_context, ctx_data) then
        local rowcheck = bound_check(node, row, one, field(v, "size1"), line)
        local colcheck = bound_check(node, col, one, field(v, "size2"), line)
        ast:add_generated_stmt(colcheck)
        ast:add_generated_stmt(rowcheck)
    end
    return node
end

-- The arguments row_start, row_end, etc can be nil to indicate implicitly
-- to take the slice from the beginning or up to its end.
function AST.expr_slice(ast, v, row_start, row_end, col_start, col_end)
    local ra, rb = row_start or literal(1), row_end or field(v, "size1")
    local ca, cb = col_start or literal(1), col_end or field(v, "size2")
    return build("MatrixSliceExpression", { object = v, row_start = ra, row_end = rb, col_start = ca, col_end = cb, line = line })
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
    local binop_body = (op ~= '..' and { operator = op, left = expa, right = expb, line = line })
    if binop_body then
        if op == 'and' or op == 'or' then
            -- Logical operators voids the generated statements because they
            -- cannot be hoisted to the statement level. The reason is that
            -- logical expressions are conditionally executed.
            ast:void_generated_stmts()
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

-- Only identifier that appears in expressions should be declared here. Variable
-- declarations should use "var_declare" instead.
function AST.identifier(ast, name)
    ast:ident_report(name)
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

function AST.expr_function_call(ast, v, args, kw_keys, kw_vals)
    local callee = v
    if kw_keys then
        local t = build("Table", { array_entries = {}, hash_keys = kw_keys, hash_values = kw_vals })
        table.insert(args, 1, t)
        callee = field(callee, "__keyargs")
    end
    return build("CallExpression", { callee = callee, arguments = args })
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

local function check_index(index, inf, sup, line)
    local index_const = libexpr.is_const(index)
    if index_const then
        local inf_const = inf and libexpr.is_const(inf)
        if inf_const and index_const >= inf_const then
            inf = nil
        end
        local sup_const = sup and libexpr.is_const(sup)
        if sup_const and index_const <= sup_const then
            sup = nil
        end
    end
    if inf or sup then
        return build("CheckIndex", { index = index, inf = inf, sup = sup, line = line })
    end
end

function AST.for_post_process(ast, body, var, init, last, step)
    local for_scope = ast.current
    local rstmts = { }
    local ctx_data = { ast = ast, for_scope = for_scope }
    local i = 1
    while body[i] do
        local stmt = body[i]
        if stmt.kind == "CheckIndex" then
            local index, lin, coeff = libexpr.linear_ctxfree(stmt.index, var, var_context, ctx_data)
            local inf_cf = lin and (not stmt.inf or libexpr.context_free(stmt.inf, var_context, ctx_data))
            local sup_cf = lin and (not stmt.sup or libexpr.context_free(stmt.sup, var_context, ctx_data))
            if inf_cf and sup_cf then
                local index_inf = libexpr.eval(index, var, init)
                local index_sup = libexpr.eval(index, var, last)
                if coeff < 0 then index_inf, index_sup = index_sup, index_inf end
                local icheck = check_index(index_inf, stmt.inf, nil, stmt.line)
                if icheck then rstmts[#rstmts+1] = icheck end
                local scheck = check_index(index_sup, nil, stmt.sup, stmt.line)
                if scheck then rstmts[#rstmts+1] = scheck end
                table.remove(body, i)
                i = i - 1
            end
        end
        i = i + 1
    end
    return rstmts
end

function AST.for_stmt(ast, var, init, last, step, body, line)
    local pre_stmts = ast:for_post_process(body, var, init, last, step)
    local for_init = build("ForInit", { id = var, value = init, line = line })
    local stmt = build_stmt(ast, "ForStatement", { init = for_init, last = last, step = step, body = body, line = line })
    add_pre_stmts(stmt, pre_stmts)
    return stmt
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

-- All the lexical variable declaration should pass from this function.
-- The functions fscope_start/end will be called appropriately before and after
-- to ensure that variables are declared in the correct lexical scope.
function AST.var_declare(ast, name, value)
    ast:ident_report(name)
    local id = ident(name)
    local vinfo = { id = id, mutable = false, value = value }
    ast.current.vars[name] = vinfo
    return id
end

function AST.ident_report(ast, name)
    -- Declare the identifier to the lexical genid to avoid possible
    -- conflicts.
    ast.lex_genid.var_declare(name)
end

function AST.genid(ast)
    local id = ast.lex_genid.new_ident()
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
                vinfo.value = false
            end
        end
    end
end

function AST.fscope_register_use(ast, mod)
    if not ast.current.imports then ast.current.imports = { } end
    ast.current.imports[#ast.current.imports + 1] = mod
end

local ASTClass = { __index = AST }

local function new_ast(lex_genid)
    local ast = { gen_stmts = { }, for_stack = { } , lex_genid = lex_genid }
    return setmetatable(ast, ASTClass)
end

return { New = new_ast }
