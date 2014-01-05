local filename = assert(select(1, ...), "please give a file name to parse.")
local debug_file = assert(io.open(filename))

local function DEBUG_READ_FILE(ls)
    return debug_file:read(64)
end

local lex_setup = require('nyanga.lexer')
local ls = lex_setup(DEBUG_READ_FILE)
repeat
	ls:next()
	local line = string.format("%3i:", ls.linenumber)
	if ls.tokenval then
		print(line, ls.token, ls.tokenval)
	else
		print(line, ls.token)
	end
until ls.token == 'TK_eof'
