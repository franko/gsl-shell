local LJ_52 = false

setmetatable(_G, {
    __index = function(t, x) error('undefined global ' .. x) end,
    __newindex = function(t, k, v) error('undefined global ' .. k) end
    }
)

--[[
err_syntax
err_token
]]

local function token2str(tok)
    return string.match(tok, "^TK_") and string.sub(tok, 4) or tok
end

local function lex_opt(ls, tok)
	if ls.token == tok then
		ls:next()
		return true
	end
	return false
end

local function lex_check(ls, tok)
	if ls.token ~= tok then err_token(ls, tok) end
	ls:next()
end

local function lex_match(ls, what, who, line)
    if not lex_opt(ls, what) then
        if line == ls.linenumber then
            err_token(ls, what)
        else
            lex_error(ls, ls.token, "%s expected (to close %s at line %d)", token2str(what), token2str(who), line)
        end
    end
end

local function lex_str(ls)
	if ls.token ~= 'TK_name' and (LJ_52 or ls.token ~= 'TK_goto') then
		err_token(ls, 'TK_name')
	end
	local s = ls.tokenval
	ls:next()
	return s
end

local expr, parse_body, parse_block

local function expr_field(ast, v, ls)
	ls:next() -- Skip dot or colon.
	local key = lex_str(ls)
	return ast:expr_index(v, key)
end

local function expr_bracket(ast, ls)
	ls:next() -- Skip '['.
	local v = expr(ast, ls)
	lex_check(ls, ']')
end

-- Priorities for each binary operator.
-- (left priority) * 256 + (right priority)
-- modulus is your friend
local BinOp = {
    ['+']  = 6 * 256 + 6, ['-']    = 6 * 256 + 6, ['*'] = 7 * 256 + 7, ['/'] = 7 * 256 + 7, ['%'] = 7 * 256 + 7,
    ['^']  = 10* 256 + 9, TK_concat= 5 * 256 + 4, -- POW CONCAT (right associative)
    TK_eq  = 3 * 256 + 3, TK_ne    = 3 * 256 + 3,
    ['<']  = 3 * 256 + 3, Tk_ge    = 3 * 256 + 3, ['>'] = 3 * 256 + 3, TK_le = 3 * 256 + 3,
    TK_and = 2 * 256 + 2, TK_or    = 1 * 256 + 1,
}

local UNARY_PRIORITY = 8

local function left_priority(op)
    return bit.rshift(op, 8)
end

local function right_priority(op)
    return bit.band(op, 0xff)
end

local function expr_table(ast, ls)
    error('NYI: expr_table')
end

local function expr_simple(ast, ls)
    local tk, val = ls.token, ls.tokenval
    local e
    if tk == 'TK_number' then
        e = ast:expr_number(val)
    elseif tk == 'TK_string' then
        e = ast:expr_string(val)
    elseif tk == 'TK_nil' then
        e = ast:expr_nil()
    elseif tk == 'TK_true' then
        e = ast:expr_boolean(true)
    elseif tk == 'TK_false' then
        e = ast:expr_boolean(false)
    elseif tk == 'TK_dots' then
        if not ls.fs.proto_varargs then
            err_syntax(ls, "cannot use \"...\" outside a vararg function")
        end
        e = ast:expr_vararg()
    elseif tk == '{' then
        e = expr_table(ast, ls)
    elseif tk == 'TK_function' then
        ls:next()
        return parse_body(ast, ls, ls.linenumber)
    else
        return expr_primary(ast, ls)
    end
    ls:next()
    return e
end

local function expr_list(ast, ls)
    local exps = { }
    exps[1] = expr(ast, ls)
    while lex_opt(ls, ',') do
        exps[#exps + 1] = exor(ast, ls)
    end
    return exps
end

local function expr_unop(ast, ls)
    local tk = ls.token
    if tk == 'TK_not' or tk == '-' or tk == '#' then
        ls:next()
        local v = expr_binop(ast, ls, UNARY_PRIORITY)
        return ast:expr_unop(tk, v)
    else
        return expr_simple(ast, ls)
    end
end

-- Parse binary expressions with priority higher than the limit.
local function expr_binop(ast, ls, limit)
	local v = expr_unop(ast, ls)
    local op = ls.token
    while BinOp[op] and left_priority(BinOp[op]) > limit do
        ls:next()
        local v2, nextop = expr_binop(ast, ls, right_priority(BinOp[op]))
        v = ast:expr_binop(op, v, v2)
        op = nextop
    end
    return v, op
end

function expr(ast, ls)
	return expr_binop(ast, ls, 0) -- Priority 0: parse whole expression.
end

-- Parse primary expression.
local function expr_primary(ast, ls)
    local v, vk
    -- Parse prefix expression.
    if ls.token == '(' then
        local line = ls.linenumber
        ls:next()
        vk, v = 'expr', expr(ast, ls)
        lex_match(ls, ')', '(', line)
    elseif ls.token == 'TK_name' or (not LJ_52 and ls.token == 'TK_goto') then
        vk, v = 'var', ast:expr_var(ls.tokenval)
    else
        err_syntax(ls, "unexpected symbol")
    end
    while true do -- Parse multiple expression suffixes.
        if ls.token == '.' then
            vk, v = 'indexed', expr_field(ast, v, ls)
        elseif ls.token == '[' then
            local key = expr_bracket(ast, ls)
            vk, v = 'indexed', ast:expr_index(v, key)
        elseif ls.token == ':' then
            ls:next()
            local key = lex_str(ls)
            local args = parse_args(ast, ls)
            vk, v = 'call', ast:expr_method_call(v, key, args)
        elseif ls.token == '(' or ls.token == 'TK_string' or ls.token == '{' then
            local args = parse_args(ast, ls)
            vk, v = 'call', ast:expr_function_call(v, args)
        else
            break
        end
    end
    return v, vk
end

-- Parse 'return' statement.
local function parse_return(ast, ls, line)
    ls:next() -- Skip 'return'.
    ls.fs.proto_has_return = true
    local exps
    if EndOfBlock(ls.token) or ls.token == ';' then -- Base return.
        exps = { }
    else -- Return with one or more values.
        exps = expr_list(ast, ls)
    end
    return ast:return_stmt(exps, line)
end

-- Parse function argument list.
local function parse_args(ast, ls, line)
    local line = ls.linenumber
    local args
    if ls.token == '(' then
        if not LJ_52 and line ~= ls.lastline then
            err_syntax(ls, "ambiguous syntax (function call x new statement)")
        end
        ls:next()
        if ls.token ~= ')' then -- Not f().
            args = expr_list(ast, ls)
        end
        lex_match(ls, ')', '(', line)
    elseif ls.token == '{' then
        local a = expr_table(ast, ls)
        args = { a }
    elseif ls.token == 'TK_string' then
        local a = lex_str(ls)
        args = { ast:expr_string(a) }
    else
        err_syntax(ls, "function arguments expected")
    end
    return args
end

local function parse_assignment(ast, ls, vlist, var, vk)
    checkcond(ls, vk == 'var' or vk == 'indexed', 'syntax error')
    vlist[#vlist+1] = var
    if lex_opt(ls, ',') then
        local n_var, n_vk = expr_primary(ast, ls)
        return parse_assignment(ast, ls, vlist, n_var, n_vk)
    else -- Parse RHS.
        ls:check('=')
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
        return parse_assignment(ast, ls, vlist)
    end
end

-- Parse 'local' statement.
local function parse_local(ast, ls, line)
    if lex_opt(ls, 'TK_function') then -- Local function declaration.
        return parse_body(ast, ls, line)
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

local function parse_while(ast, ls, line)
    ls:next() -- Skip 'while'.
    local cond = expr_cond(ast, ls)
    ls:check()
    local b = parse_block(ast, ls)
    lex_match(ls, 'TK_end', 'TK_while', line)
    return ast:new_while_statement(cond, b, ls.linenumber)
end

local function parse_then(ast, ls, branches)
    ls:next()
    local cond = expr_cond(ast, ls)
    ls:check('TK_then')
    local b = parse_block(ast, ls)
    branches[#branches + 1] = {cond, b}
end

local function parse_if(ast, ls, line)
    local branches = { }
    parse_then(ast, ls, branches)
    while ls.token == 'TK_elseif' do
        parse_then(ast, ls, branches)
    end
    local else_branch
    if ls.token == 'TK_else' then
        ls:next() -- Skip 'else'.
        else_branch = parse_block(ast, ls)
    end
    lex_match(ls, 'TK_end', 'TK_if', line)
    return ast:if_stmt(branches, else_branch, line)
end

local IsLastStatement = {
    ['TK_return'] = true,
    ['TK_break']  = true,
}

local EndOfBlock = { TK_else = true, TK_elseif = true, TK_end = true, TK_until = true, TK_eof = true }

local function parse_stmt(ast, ls)
    local line = ls.linenumber
    local stmt
    if ls.token == 'TK_if' then
        stmt = parse_if(ast, ls, line)
    elseif ls.token == 'TK_while' then
        stmt = parse_while(ast, ls, line)
    elseif ls.token == 'TK_return' then
        stmt = parse_return(ast, ls, line)
        return stmt, true
    elseif ls.token == 'TK_local' then
        ls:next()
        stmt = parse_local(ast, ls, line)
    else
        stmt = parse_call_assign(ast, ls)
    end
    return stmt, false
end

local function parse_params(ast, ls, needself)
    lex_check(ls, '(')
    local args = { }
    if needself then
        args[1] = ast:expr_var("self")
    end
    if ls.token ~= ')' then
        repeat
            if ls.token == 'TK_name' || (not LJ_52 and ls.token == 'TK_goto') then
                args[#args+1] = ast:expr_var(lex_str(ls))
            elseif ls.token == 'TK_dots' then
                ls:next()
                ls.fs.proto_varargs = true
            else
                err_syntax(ls, "<name> or \"...\" expected")
            end
        until not lex_opt(ls, ',')
    end
    lex_check(ls, ')')
    return args
end

local function new_proto(ls, varargs)
    return { proto_varargs = varargs }
end

-- Parse body of a function.
function parse_body(ast, ls, line, needself)
    local pfs = ls.fs
    ls.fs = new_proto(ls, false)
    local args = parse_params(ls, needself)
    local body = parse_chunk(ls)
    if ls.token ~= 'TK_end' then
        lex_match(ls, 'TK_end', 'TK_function', line)
    end
    ls:next()
    ls.fs = pfs
    return ast:new_function(args, body, line)
end

local function parse_chunk(ast, ls)
    local islast = false
    local chunk = ast:new_block(ls.linenumber)
    while not islast and not EndOfBlock(ls.token) do
        local stmt
        stmt, islast = parse_stmt(ast, ls)
        ast:add_block_stmt(stmt, islast, ls.linenumber)
        lex_opt(ls, ';')
    end
    return chunk
end

function parse_block(ast, ls)
    return parse_chunk(ast, ls)
end

local function parse(ast, ls)
    ls.fs = new_proto(ls, true)
    ls:next()
    local chunk = parse_chunk(ast, ls)
    return chunk
end
