local operator = require("operator")
local parser_base = require("parser-base")

local err_syntax, err_token = parser_base.err_syntax, parser_base.err_token
local lex_opt, lex_check, lex_str, lex_match = parser_base.lex_opt, parser_base.lex_check, parser_base.lex_str, parser_base.lex_match
local checkcond = parser_base.checkcond

local LJ_52 = false

local IsLastStatement = { TK_return = true, TK_break  = true }
local EndOfBlock = { TK_else = true, TK_elseif = true, TK_end = true, TK_until = true, TK_eof = true }

local expr_primary, expr, expr_unop, expr_binop, expr_simple
local expr_list, expr_table
local parse_body, parse_simple_body, parse_block, parse_chunk, parse_args

local RANGE_TOKEN, LABEL_TOKEN, RESOLVE_TOKEN = ':', ':', 'TK_resolve'

local function var_lookup(ast, ls)
    local name = lex_str(ls)
    return ast:identifier(name)
end

local function expr_field(ast, ls, v)
	ls:next() -- Skip dot or colon.
	local key = lex_str(ls)
	return ast:expr_property(v, key)
end

local function expr_bracket(ast, ls)
	ls:next() -- Skip '['.
	local v = expr(ast, ls)
	lex_check(ls, ']')
    return v
end

function expr_table(ast, ls)
    local line = ls.linenumber
    local hkeys, hvals = { }, { }
    local avals = { }
    lex_check(ls, '{')
    while ls.token ~= '}' do
        local key
        if ls.token == '[' then
            key = expr_bracket(ast, ls)
            lex_check(ls, '=')
        elseif (ls.token == 'TK_name' or (not LJ_52 and ls.token == 'TK_goto')) and ls:lookahead() == '=' then
            local name = lex_str(ls)
            key = ast:literal(name)
            lex_check(ls, '=')
        end
        local val = expr(ast, ls)
        if key then
            hkeys[#hkeys + 1] = key
            hvals[#hvals + 1] = val
        else
            avals[#avals + 1] = val
        end
        if not lex_opt(ls, ',') and not lex_opt(ls, ';') then break end
    end
    lex_match(ls, '}', '{', line)
    return ast:expr_table(avals, hkeys, hvals, line)
end

local function expr_matrix(ast, ls)
    local line = ls.linenumber
    lex_check(ls, '[')
    local ncol
    local t = { }
    while ls.token ~= ']' do
        local n = 0
        while true do
            t[#t + 1], n = expr(ast, ls), n + 1
            if not lex_opt(ls, ',') then break end
        end
        lex_opt(ls, ';')
        ncol = ncol or n
        if ncol ~= n then err_syntax(ls, "mismatching number of columns") end
    end
    lex_match(ls, ']', '[', line)
    return ast:expr_matrix(ncol, t, line)
end

function expr_simple(ast, ls)
    local tk, val = ls.token, ls.tokenval
    local e
    if tk == 'TK_number' then
        e = ast:literal(val)
    elseif tk == 'TK_string' then
        e = ast:literal(val)
    elseif tk == 'TK_nil' then
        e = ast:literal(nil)
    elseif tk == 'TK_true' then
        e = ast:literal(true)
    elseif tk == 'TK_false' then
        e = ast:literal(false)
    elseif tk == 'TK_dots' then
        if not ls.fs.varargs then
            err_syntax(ls, "cannot use \"...\" outside a vararg function")
        end
        e = ast:expr_vararg()
    elseif tk == '{' then
        return expr_table(ast, ls)
    elseif tk == '[' then
        return expr_matrix(ast, ls)
    elseif tk == 'TK_function' then
        ls:next()
        local args, body, proto = parse_body(ast, ls, ls.linenumber, false, false)
        return ast:expr_function(args, body, proto)
    elseif tk == '|' then
        local args, body, proto = parse_simple_body(ast, ls, ls.linenumber)
        return ast:expr_function(args, body, proto)
    else
        return expr_primary(ast, ls)
    end
    ls:next()
    return e
end

function expr_list(ast, ls)
    local exps = { }
    exps[1] = expr(ast, ls)
    while lex_opt(ls, ',') do
        exps[#exps + 1] = expr(ast, ls)
    end
    return exps
end

function expr_unop(ast, ls)
    local tk = ls.token
    if tk == 'TK_not' or tk == '-' or tk == '#' then
        ls:next()
        local v = expr_binop(ast, ls, operator.unary_priority)
        return ast:expr_unop(ls.token2str(tk), v)
    else
        local exp = expr_simple(ast, ls)
        if lex_opt(ls, "'") then
            exp = ast:expr_unop("'", exp)
        end
        return exp
    end
end

-- Parse binary expressions with priority higher than the limit.
function expr_binop(ast, ls, limit)
	local v = expr_unop(ast, ls)
    local op = ls.token2str(ls.token)
    while operator.is_binop(op) and operator.left_priority(op) > limit do
        ls:next()
        local v2, nextop = expr_binop(ast, ls, operator.right_priority(op))
        v = ast:expr_binop(op, v, v2)
        op = nextop
    end
    return v, op
end

function expr(ast, ls)
	return expr_binop(ast, ls, 0) -- Priority 0: parse whole expression.
end

-- Parse primary expression.
function expr_primary(ast, ls)
    local v, vk
    -- Parse prefix expression.
    if ls.token == '(' then
        local line = ls.linenumber
        ls:next()
        vk, v = 'expr', expr(ast, ls)
        lex_match(ls, ')', '(', line)
    elseif ls.token == 'TK_name' or (not LJ_52 and ls.token == 'TK_goto') then
        vk, v = 'var', var_lookup(ast, ls)
    else
        err_syntax(ls, "unexpected symbol")
    end
    while true do -- Parse multiple expression suffixes.
        if ls.token == '.' then
            vk, v = 'indexed', expr_field(ast, ls, v)
        elseif ls.token == '[' then
            local row_slice, col_slice = false, false
            local dual_index = false
            ls:next()
            local key, upto
            if ls.token ~= RANGE_TOKEN then
                key = expr(ast, ls)
            end
            if lex_opt(ls, RANGE_TOKEN) then
                row_slice = true
                if ls.token ~= ',' and ls.token ~= ']' then
                    upto = expr(ast, ls)
                end
            end
            local upto_col, key_col
            if row_slice or ls.token == ',' then
                lex_check(ls, ',')
                dual_index = true
                if ls.token ~= RANGE_TOKEN then
                    key_col = expr(ast, ls)
                end
                if lex_opt(ls, RANGE_TOKEN) then
                    col_slice = true
                    if ls.token ~= ']' then
                        upto_col = expr(ast, ls)
                    end
                end
            end
            lex_check(ls, ']')
            if dual_index then
                if row_slice or col_slice then
                    if not row_slice then upto = key end
                    if not col_slice then upto_col = key_col end
                    vk, v = 'slice', ast:expr_slice(v, key, upto, key_col, upto_col)
                else
                    vk, v = 'indexed', ast:expr_index_dual(v, key, key_col)
                end
            else
                if upto then err_syntax(ls, "slicing with non-matrix indexing") end
                vk, v = 'indexed', ast:expr_index(v, key)
            end
        elseif ls.token == RESOLVE_TOKEN then
            ls:next()
            local key = lex_str(ls)
            local args = parse_args(ast, ls, false)
            vk, v = 'call', ast:expr_method_call(v, key, args)
        elseif ls.token == '(' or ls.token == 'TK_string' or ls.token == '{' then
            local args, kws, kvs = parse_args(ast, ls, true)
            vk, v = 'call', ast:expr_function_call(v, args, kws, kvs)
        else
            break
        end
    end
    return v, vk
end

-- Parse statements ----------------------------------------------------


-- Parse 'return' statement.
local function parse_return(ast, ls, line)
    ls:next() -- Skip 'return'.
    ls.fs.has_return = true
    local exps
    if EndOfBlock[ls.token] or ls.token == ';' then -- Base return.
        exps = { }
    else -- Return with one or more values.
        exps = expr_list(ast, ls)
    end
    return ast:return_stmt(exps, line)
end

-- Parse numeric 'for'.
local function parse_for_num(ast, ls, varname, line)
    lex_check(ls, '=')
    local init = expr(ast, ls)
    lex_check(ls, ',')
    local last = expr(ast, ls)
    local step
    if lex_opt(ls, ',') then
        step = expr(ast, ls)
    else
        step = ast:literal(1)
    end
    lex_check(ls, 'TK_do')
    ast:fscope_begin()
    local var = ast:var_declare(varname)
    local body = parse_chunk(ast, ls, false)
    local stmt = ast:for_stmt(var, init, last, step, body, line)
    ast:fscope_end()
    return stmt
end

-- Parse 'for' iterator.
local function parse_for_iter(ast, ls, indexname)
    ast:fscope_begin()
    local vars = { ast:var_declare(indexname) }
    while lex_opt(ls, ',') do
        vars[#vars + 1] = ast:var_declare(lex_str(ls))
    end
    lex_check(ls, 'TK_in')
    local line = ls.linenumber
    local exps = expr_list(ast, ls)
    lex_check(ls, 'TK_do')
    local body = parse_block(ast, ls)
    local stmt = ast:for_iter_stmt(vars, exps, body, line)
    ast:fscope_end()
    return stmt
end

-- Parse 'for' statement.
local function parse_for(ast, ls, line)
    ls:next()  -- Skip 'for'.
    local varname = lex_str(ls)  -- Get first variable name.
    local stmt
    if ls.token == '=' then
        stmt = parse_for_num(ast, ls, varname, line)
    elseif ls.token == ',' or ls.token == 'TK_in' then
        stmt = parse_for_iter(ast, ls, varname)
    else
        err_syntax(ls, "'=' or 'in' expected")
    end
    lex_match(ls, 'TK_end', 'TK_for', line)
    return stmt
end

local function parse_repeat(ast, ls, line)
    ast:fscope_begin()
    ls:next() -- Skip 'repeat'.
    local body = parse_block(ast, ls)
    lex_match(ls, 'TK_until', 'TK_repeat', line)
    local cond = expr(ast, ls) -- Parse condition.
    ast:fscope_end()
    return ast:repeat_stmt(cond, body, line)
end

local function expr_keyword(ast, ls, accept_keywords)
    local kw, val
    if ls.token == "TK_name" and accept_keywords and ls:lookahead() == '=' then
        local name = lex_str(ls)
        kw = ast:literal(name)
        lex_check(ls, '=')
    end
    val = expr(ast, ls)
    return val, kw
end

-- Parse function argument list.
function parse_args(ast, ls, accept_keywords)
    local line = ls.linenumber
    local args = { }
    local kws, kvs
    if ls.token == '(' then
        if not LJ_52 and line ~= ls.lastline then
            err_syntax(ls, "ambiguous syntax (function call x new statement)")
        end
        ls:next()
        while ls.token ~= ')' do
            local val, kw = expr_keyword(ast, ls, accept_keywords)
            if kw then
                if not kws then kws, kvs = {}, {} end
                kws[#kws+1] = kw
                kvs[#kvs+1] = val
            else
                args[#args+1] = val
            end
            if not lex_opt(ls, ',') then break end
        end
        lex_match(ls, ')', '(', line)
    elseif ls.token == '{' then
        local a = expr_table(ast, ls)
        args = { a }
    elseif ls.token == 'TK_string' then
        local a = ls.tokenval
        ls:next()
        args = { ast:literal(a) }
    else
        err_syntax(ls, "function arguments expected")
    end
    return args, kws, kvs
end

local function parse_assignment(ast, ls, vlist, var, vk)
    checkcond(ls, vk == 'var' or vk == 'indexed' or vk == 'slice', 'syntax error')
    vlist[#vlist+1] = var
    if lex_opt(ls, ',') then
        local n_var, n_vk = expr_primary(ast, ls)
        return parse_assignment(ast, ls, vlist, n_var, n_vk)
    else -- Parse RHS.
        lex_check(ls, '=')
        local exps = expr_list(ast, ls)
        return ast:assignment_expr(vlist, exps, ls.linenumber)
    end
end

local function parse_call_assign(ast, ls)
    local var, vk = expr_primary(ast, ls)
    if vk == 'call' then
        return ast:new_statement_expr(var, ls.linenumber)
    else
        local vlist = { }
        return parse_assignment(ast, ls, vlist, var, vk)
    end
end

-- Parse 'local' statement.
local function parse_local(ast, ls)
    local line = ls.linenumber
    if lex_opt(ls, 'TK_function') then -- Local function declaration.
        local name = lex_str(ls)
        local args, body, proto = parse_body(ast, ls, ls.linenumber, false, true)
        return ast:local_function_decl(name, args, body, proto)
    else -- Local variable declaration.
        local vl = { }
        repeat -- Collect LHS.
            vl[#vl+1] = lex_str(ls)
        until not lex_opt(ls, ',')
        local exps
        if lex_opt(ls, '=') then -- Optional RHS.
            exps = expr_list(ast, ls)
        else
            exps = { }
        end
        return ast:local_decl(vl, exps, line)
    end
end

local function parse_func(ast, ls, line)
    local needself = false
    ls:next() -- Skip 'function'.
    -- Parse function name.
    local v = var_lookup(ast, ls)
    while ls.token == '.' do -- Multiple dot-separated fields.
        v = expr_field(ast, ls, v)
    end
    if ls.token == RESOLVE_TOKEN then -- Optional colon to signify method call.
        needself = true
        v = expr_field(ast, ls, v)
    end
    local args, body, proto = parse_body(ast, ls, line, needself, not needself)
    return ast:function_decl(v, args, body, proto)
end

local function parse_while(ast, ls, line)
    ls:next() -- Skip 'while'.
    local cond = expr(ast, ls)
    ast:fscope_begin()
    lex_check(ls, 'TK_do')
    local body = parse_block(ast, ls)
    lex_match(ls, 'TK_end', 'TK_while', line)
    ast:fscope_end()
    return ast:while_stmt(cond, body, ls.linenumber)
end

local function parse_then(ast, ls, tests, blocks)
    ls:next()
    tests[#tests+1] = expr(ast, ls)
    lex_check(ls, 'TK_then')
    blocks[#blocks+1] = parse_block(ast, ls)
end

local function parse_if(ast, ls, line)
    local tests, blocks = { }, { }
    parse_then(ast, ls, tests, blocks)
    while ls.token == 'TK_elseif' do
        parse_then(ast, ls, tests, blocks)
    end
    local else_branch
    if ls.token == 'TK_else' then
        ls:next() -- Skip 'else'.
        else_branch = parse_block(ast, ls)
    end
    lex_match(ls, 'TK_end', 'TK_if', line)
    return ast:if_stmt(tests, blocks, else_branch, line)
end

local function parse_label(ast, ls)
    ls:next() -- Skip '::'.
    local name = lex_str(ls)
    lex_check(ls, LABEL_TOKEN)
    -- Recursively parse trailing statements: labels and ';' (Lua 5.2 only).
    while true do
        if ls.token == LABEL_TOKEN then
            parse_label(ast, ls)
        elseif LJ_52 and ls.token == ';' then
            ls:next()
        else
            break
        end
    end
    return ast:label_stmt(name, ls.linenumber)
end

local function parse_goto(ast, ls)
    local line = ls.linenumber
    local name = lex_str(ls)
    return ast:goto_stmt(name, line)
end

local function parse_use(ast, ls, line)
    ls:next()
    if ls.token ~= 'TK_string' then err_token(ls, 'TK_string') end
    local name = ls.tokenval
    ls:next()
    return ast:use_stmt(name, line)
end

-- Parse a statement. Returns the statement itself and a boolean that tells if it
-- must be the last one in a chunk.
local function parse_stmt(ast, ls)
    local line = ls.linenumber
    local stmt
    if ls.token == 'TK_if' then
        stmt = parse_if(ast, ls, line)
    elseif ls.token == 'TK_while' then
        stmt = parse_while(ast, ls, line)
    elseif ls.token == 'TK_do' then
        ls:next()
        local body = parse_block(ast, ls)
        lex_match(ls, 'TK_end', 'TK_do', line)
        stmt = ast:do_stmt(body, line)
    elseif ls.token == 'TK_for' then
        stmt = parse_for(ast, ls, line)
    elseif ls.token == 'TK_repeat' then
        stmt = parse_repeat(ast, ls, line)
    elseif ls.token == 'TK_function' then
        stmt = parse_func(ast, ls, line)
    elseif ls.token == 'TK_local' then
        ls:next()
        stmt = parse_local(ast, ls, line)
    elseif ls.token == 'TK_return' then
        stmt = parse_return(ast, ls, line)
        return stmt, true -- Must be last.
    elseif ls.token == 'TK_break' then
        ls:next()
        stmt = ast:break_stmt(line)
        return stmt, not LJ_52 -- Must be last in Lua 5.1.
    elseif ls.token == 'TK_use' then
        stmt = parse_use(ast, ls, line)
    elseif LJ_52 and ls.token == ';' then
        ls:next()
        return parse_stmt(ast, ls)
    elseif ls.token == LABEL_TOKEN then
        stmt = parse_label(ast, ls)
    elseif ls.token == 'TK_goto' then
        if LJ_52 or ls:lookahead() == 'TK_name' then
            ls:next()
            stmt = parse_goto(ast, ls)
        end
    end
    -- If here 'stmt' is "nil" then ls.token didn't match any of the previous rules.
    -- Fall back to call/assign rule.
    if not stmt then
        stmt = parse_call_assign(ast, ls)
    end
    return stmt, false
end

local function parse_params_delim(ast, ls, needself, accept_keywords, start_token, end_token)
    lex_check(ls, start_token)
    local args = { }
    local kargs
    if needself then
        args[1] = ast:var_declare("self")
    end
    if ls.token ~= end_token then
        repeat
            if ls.token == 'TK_name' or (not LJ_52 and ls.token == 'TK_goto') then
                local name = lex_str(ls)
                if accept_keywords and lex_opt(ls, '=') then
                    local arg_expr = expr(ast, ls)
                    if not kargs then kargs = {} end
                    kargs[#kargs+1] = { parameter = name, default = arg_expr }
                else
                    args[#args+1] = ast:var_declare(name)
                end
            elseif ls.token == 'TK_dots' then
                ls:next()
                ls.fs.varargs = true
                args[#args + 1] = ast:expr_vararg()
                break
            else
                err_syntax(ls, "<name> or \"...\" expected")
            end
        until not lex_opt(ls, ',')
    end
    lex_check(ls, end_token)
    args.keyargs = kargs
    return args
end

local function parse_params(ast, ls, needself, accept_keywords)
    return parse_params_delim(ast, ls, needself, accept_keywords, '(', ')')
end

local function new_proto(ls, varargs)
    return { varargs = varargs }
end

function parse_chunk(ast, ls, top_level, use_stmts)
    local firstline = ls.linenumber
    local islast = false
    local body = { }
    if use_stmts then
        for i = 1, #use_stmts do
            body[i] = use_stmts[i]
        end
    end
    while not islast and not EndOfBlock[ls.token] do
        local stmt
        stmt, islast = parse_stmt(ast, ls)
        body[#body + 1] = stmt
        lex_opt(ls, ';')
    end
    local lastline = ls.linenumber
    if top_level then
        return ast:chunk(body, ls.chunkname, 0, lastline)
    else
        return ast:block_stmt(body, firstline, lastline)
    end
end

-- Parse body of a function.
function parse_body(ast, ls, line, needself, accept_keywords)
    local pfs = ls.fs
    ls.fs = new_proto(ls, false)
    ast:fscope_begin()
    ls.fs.firstline = line
    local args = parse_params(ast, ls, needself, accept_keywords)
    local body = parse_block(ast, ls)
    ast:fscope_end()
    local proto = ls.fs
    if ls.token ~= 'TK_end' then
        lex_match(ls, 'TK_end', 'TK_function', line)
    end
    ls.fs.lastline = ls.linenumber
    ls:next()
    ls.fs = pfs
    return args, body, proto
end

function parse_simple_body(ast, ls, line)
    local pfs = ls.fs
    ls.fs = new_proto(ls, false)
    ast:fscope_begin()
    ls.fs.firstline = line
    local args = parse_params_delim(ast, ls, false, false, '|', '|')
    local exp = expr(ast, ls)
    local retstmt = ast:return_stmt({ exp }, line)
    local body = ast:block_stmt({ retstmt }, line)
    ast:fscope_end()
    ls.fs.lastline = ls.linenumber
    local proto = ls.fs
    ls.fs = pfs
    return args, body, proto
end

function parse_block(ast, ls)
    ast:fscope_begin()
    local block = parse_chunk(ast, ls, false)
    ast:fscope_end()
    return block
end

local function load_use_list(ast, use_list)
    local use_stmts = {}
    for i = 1, #use_list do
        local name = use_list[i]
        use_stmts[i] = ast:use_stmt(name)
    end
    return use_stmts
end

local function store_use_list(imports, use_list)
    for k = 1, #imports do
        local mod_name = imports[k].name
        for j = 1, #use_list do
            if mod_name == use_list[j] then
                mod_name = nil
                break
            end
        end
        if mod_name then
            use_list[#use_list+1] = mod_name
        end
    end
end

-- Parse the code chunk referenced from the lexer, "ls" and returns the
-- AST tree. The last argument can be nil or a list of module's names.
-- If the "use_list" is actually given the referenced modules will be
-- "in use". In addition the "use_list" will be completed with any other
-- module explicitely used in the chunk.
local function parse(ast, ls, use_list)
    ls:next()
    ls.fs = new_proto(ls, true)
    ast:fscope_begin()
    local args = { ast:expr_vararg(ast) }
    local use_stmts = use_list and load_use_list(ast, use_list)
    local chunk = parse_chunk(ast, ls, true, use_stmts)
    if use_list and ast.current.imports then
        store_use_list(ast.current.imports, use_list)
    end
    ast:fscope_end()
    if ls.token ~= 'TK_eof' then
        err_token(ls, 'TK_eof')
    end
    return chunk
end

return parse
