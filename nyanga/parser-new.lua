local parse_block

local LJ_52 = false

setmetatable(_G, {
    __index = function(t, x) error('undefined global ' .. x) end,
    __newindex = function(t, k, v) error('undefined global ' .. k) end
    }
)

--[[
err_syntax
parse_args
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

local expr

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

local function expr_primary(ast, ls)
    local v
    if ls.token == '(' then
        local line = ls.linenumber
        ls:next()
        local _ = expr(ast, ls)
        lex_match(ls, ')', '(', line)
        -- discarge the resulting expression
    elseif ls.token == 'TK_name' or (not LJ_52 and ls.token == 'TK_goto') then
        v = ast:expr_var(ls.tokenval)
    else
        err_syntax(ls, "unexpected symbol")
    end
    while true do
        if ls.token == '.' then
            v = expr_field(ast, v, ls)
        elseif ls.token == '[' then
            local key = expr_bracket(ast, ls)
            v = ast:expr_index(v, key)
        elseif ls.token == ':' then
            ls:next()
            local key = lex_str(ls)
            local args = parse_args(ast, ls)
            v = ast:expr_method_call(v, key, args)
        elseif ls.token == '(' or ls.token == 'TK_string' or ls.token == '{' then
            local args = parse_args(ast, ls)
            v = ast:expr_function_call(v, args)
        else
            break
        end
    end
end

local function parse_assignment(ast, ls, vlist, var, vk)
    checkcond(ls, vk >= VLOCAL and vk <= VINDEXED, 'syntax error')
    ast:add_assign_lhs_var(vlist, var, ls.linenumber)
    if lex_opt(ls, ',') then
        local n_var, n_vk = expr_primary(ast, ls)
        parse_assignment(ast, ls, vlist, n_var, n_vk)
    else -- Parse RHS.
        ls:check('=')
        local els = expr_list(ast, ls)
        ast:add_assign_exprs(els, ls.linenumber)
    end
end

local function parse_call_assign(ast, ls)
    local var, vk = expr_primary(ast, ls)
    if vk == VCALL then
        return ast:new_statement_expr(var, ls.linenumber)
    else
        local vlist = ast:new_assignment(ls.linenumber)
        parse_assignment(ast, ls, vlist)
        return vlist
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

local function parse_then(ast, ls, if_stmt)
    ls:next()
    local cond = expr_cond(ast, ls)
    ls:check('TK_then')
    local b = parse_block(ast, ls)
    ast:add_if_then_block(if_stmt, cond, b, ls.linenumber)
end

local function parse_if(ast, ls, line)
    local if_stmt = ast:new_if_statement(ls.linenumber)
    parse_then(ast, ls, if_stmt)
    while ls.token == 'TK_elseif' do
        parse_then(ast, ls, if_stmt)
    end
    if ls.token == 'TK_else' then
        ls:next() -- Skip 'else'.
        local b = parse_block(ast, ls)
        ast:add_if_else_block(if_stmt, b, ls.linenumber)
    end
    lex_match(ls, 'TK_end', 'TK_if', line)
    return if_stmt
end

local StatementRule = {
    ['TK_if']    = parse_if,
    ['TK_while'] = parse_while,
}

local IsLastStatement = {
    ['TK_return'] = true,
    ['TK_break']  = true,
}

local EndOfBlock = { TK_else = true, TK_elseif = true, TK_end = true, TK_until = true, TK_eof = true }

local function parse_stmt(ast, ls)
    local line = ls.linenumber
    local parse_rule = StatementRule[ls.token]
    if parse_rule then
        local islast = IsLastStatement[ls.token]
        local stmt = parse_rule(ast, ls, line)
        return stmt, islast
    else
        local stmt = parse_call_assign(ast, ls)
        return stmt, false
    end
end

function parse_block(ast, ls)
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

local function parse(ast, ls)
    ls:next()
    local chunk = parse_block(ast, ls)
end
