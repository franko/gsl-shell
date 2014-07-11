local lex_setup = require("lua-lexer")
local reader = require("reader")
local filename, dest_filename = ...

local ls = lex_setup(reader.file(filename), filename)

local dest = io.open(dest_filename, "w")

local function write_token(f, ls)
	if ls.token == "TK_string" then
		f:write(string.format("%q", ls.tokenval))
	elseif ls.token == "TK_name" then
		f:write(ls.tokenval)
	elseif ls.token == "TK_number" then
		f:write(ls.literal)
	elseif ls.token ~= "TK_eof" then
		f:write(ls.token2str(ls.token))
	end
end

repeat
	ls:next()
	dest:write(ls.space)
	if ls.token == ':' then ls.token = '::' end
	write_token(dest, ls)
until ls.token == "TK_eof"

dest:close()
