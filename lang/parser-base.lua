
local function err_syntax(ls, em)
  ls:error(ls.token, em)
end

local function err_token(ls, token)
  ls:error(ls.token, "'%s' expected", ls.token2str(token))
end

local function checkcond(ls, cond, em)
    if not cond then err_syntax(ls, em) end
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
            local token2str = ls.token2str
            ls:error(ls.token, "%s expected (to close %s at line %d)", token2str(what), token2str(who), line)
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

return {
	err_syntax = err_syntax,
	err_token  = err_token,
	checkcond  = checkcond,
	lex_opt    = lex_opt,
	lex_str    = lex_str,
	lex_match  = lex_match,
	lex_check  = lex_check,
}
