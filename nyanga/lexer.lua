local band = bit.band

local ASCII_0, ASCII_9 = 48, 57
local ASCII_a, ASCII_z = 97, 122
local ASCII_A, ASCII_Z = 65, 90

local END_OF_STREAM = -1

local ReservedKeyword = {['and'] = 1, ['break'] = 2, ['do'] = 3, ['else'] = 4, ['elseif'] = 5, ['end'] = 6, ['false'] = 7, ['for'] = 8, ['function'] = 9, ['goto'] = 10, ['if'] = 11, ['in'] = 12, ['local'] = 13, ['nil'] = 14, ['not'] = 15, ['or'] = 16, ['repeat'] = 17, ['return'] = 18, ['then'] = 19, ['true'] = 20, ['until'] = 21, ['while'] = 22 }

setmetatable(_G, {
    __index = function(t, x) error('undefined global ' .. x) end,
    __newindex = function(t, k, v) error('undefined global ' .. k) end
    }
)

local function lex_error(ls, token, msg)
    error("boum:" .. msg)
end

local function char_isident(c)
    if type(c) == 'string' then
        -- print('char:'.. c .. '.')
        local b = string.byte(c)
        if b >= ASCII_0 and b <= ASCII_9 then
            return true
        elseif b >= ASCII_a and b <= ASCII_z then
            return true
        elseif b >= ASCII_A and b <= ASCII_Z then
            return true
        else
            return (c == '_')
        end
    end
    return false
end

local function char_isdigit(c)
    if type(c) == 'string' then
        local b = string.byte(c)
        return b >= ASCII_0 and b <= ASCII_9
    end
    return false
end

local function char_isspace(c)
    local b = string.byte(c)
    return b >= 9 and b <= 13 or b == 32
end

local function byte(ls, n)
    local k = ls.p + n
    return string.sub(ls.data, k, k)
end

local function skip(ls, n)
    ls.n = ls.n - n
    ls.p = ls.p + n
end

local function pop(ls)
    local k = ls.p
    local c = string.sub(ls.data, k, k)
    ls.p = k + 1
    ls.n = ls.n - 1
    return c
end

local function fillbuf(ls)
    local data = ls:read_func()
    -- print('+++', data)
    if not data then
        return END_OF_STREAM
    end
    ls.data, ls.n, ls.p = data, #data, 1
    return pop(ls)
end

local function next(ls)
    local c = ls.n > 0 and pop(ls) or fillbuf(ls)
    ls.current = c
    return c
end

local function curr_is_newline(ls)
    local c = ls.current
    return (c == '\n' or c == '\r')
end

local function lex_setup(read_func)
    local header = false
    local ls = {
        n = 0,
        lookahead = 'TK_eof', -- No look-ahead token. 
        linenumber = 1,
        lastline = 1,
        read_func = read_func,
    }
    next(ls)
    if ls.current == '\xef' and ls.n >= 2 and
        byte(ls, 0) == '\xbb' and byte(ls, 1) == '\xbf' then -- Skip UTF-8 BOM (if buffered).
        ls.n = ls.n - 2
        ls.p = ls.p + 2
        next(ls)
        header = true
    end
    if ls.current == '#' then
        repeat
            next(ls)
            if ls.current == END_OF_STREAM then return ls end
        until curr_is_newline(ls)
        inclinenumber(ls)
        header = true
    end
    return ls
end

local function resetbuf(ls)
    ls.save_buf = ''
end

local function save(ls, c)
    ls.save_buf = ls.save_buf .. c
end

local function save_and_next(ls)
    ls.save_buf = ls.save_buf .. ls.current
    next(ls)
end    

local function get_string(ls, init_skip, end_skip)
    return string.sub(ls.save_buf, init_skip + 1, - (end_skip + 1))
end

local function inclinenumber(ls)
    local old = ls.current
    next(ls) -- skip `\n' or `\r'
    if curr_is_newline(ls) and ls.current ~= old then
        next(ls) -- skip `\n\r' or `\r\n'
    end
    ls.linenumber = ls.linenumber + 1
end

local function skip_sep(ls)
    local count = 0
    local s = ls.current
    assert(s == '[' or s == ']')
    save_and_next(ls)
    while ls.current == '=' do
        save_and_next(ls)
        count = count + 1
    end
    return ls.current == s and count or (-count - 1)
end

local function lex_number(ls)
    local xp = 'e'
    local c = ls.current
    if c == '0' then
        save_and_next(ls)
        local xc = ls.current
        if xc == 'x' or xc == 'X' then xp = 'p' end
    end
    while char_isident(ls.current) or ls.current == '.' or
        ((ls.current == '-' or ls.current == '+') and string.lower(c) == xp) do
        c = ls.current
        save_and_next(ls)
    end
    local eval, err = loadstring('return '.. ls.save_buf)
    if not eval then
        lex_error(ls, 'TK_number', "malformed number")
    end
    return eval()
end

local function read_long_string(ls, sep, ret_value)
    save_and_next(ls) -- skip 2nd `['
    if curr_is_newline(ls) then -- string starts with a newline?
        inclinenumber(ls) -- skip it
    end
    while true do
        local c = ls.current
        if c == END_OF_STREAM then
            lex_error(ls, 'TK_eof', ret_value and "unfinished long string" or "unfinished long comment")
        elseif c == ']' then
            if skip_sep(ls) == sep then
                save_and_next(ls) -- skip 2nd `['
                break
            end
        elseif c == '\n' or c == '\r' then
            save(ls, '\n')
            inclinenumber(ls)
            if not ret_value then
                resetbuf(ls) -- avoid wasting space
            end
        else
            if ret_value then save_and_next(ls)
            else next(ls) end
        end
    end
    if ret_value then
        return get_string(ls, 2 + sep, 2 + sep)
    end
end

local Escapes = {
    a = '\a', b = '\b', f = '\f', n = '\n', r = '\r', t = '\t',
    v = '\v',
}

local function hex_char(c)
    if string.match(c, '^%x') then
        local b = band(string.byte(c), 15)
        if not char_isdigit(c) then b = b + 9 end
        return b
    end
end

local function read_string(ls, delim)
    save_and_next(ls)
    while ls.current ~= delim do
        local c = ls.current
        if c == END_OF_STREAM then
            lex_error(ls, 'TK_eof', "unfinished string")
        elseif c == '\n' or c == '\r' then
            lex_error(ls, 'TK_string', "unfinished string")
        elseif c == '\\' then
            c = next(ls) -- Skip the '\\'.
            local esc = Escapes[c]
            if esc then
                c = esc
            elseif c == 'x' then -- Hexadecimal escape '\xXX'.
                local ch1 = hex_char(next(ls))
                c = nil
                if ch1 then
                    local ch2 = hex_char(next(ls))
                    if ch2 then
                        c = string.char(ch1 * 16 + ch2)
                    end
                end
                if not c then
                    lex_error(ls, 'TK_string', "invalid escape sequence")
                end
            elseif c == 'z' then -- Skip whitespace.
                next(ls)
                while char_isspace(ls.current) do
                    if curr_is_newline(ls) then inclinenumber(ls) else next(ls) end
                end
            elseif c == '\n' or c == '\r' then
                save(ls, '\n')
                inclinenumber(ls)
            elseif c == '\\' or c == '\"' or c == '\''  or c == END_OF_STREAM then
            else
                if not char_isdigit(c) then
                    lex_error(ls, 'TK_string', "invalid escape sequence")
                end
                local bc = band(string.byte(c), 15) -- Decimal escape '\ddd'.
                if char_isdigit(next(ls)) then
                    bc = bc * 10 + band(string.byte(ls.current), 15)
                    if char_isdigit(next(ls)) then
                        bc = bc * 10 + band(string.byte(ls.current), 15)
                        if bc > 255 then
                            lex_error(ls, 'TK_string', "invalid escape sequence")
                        end
                    end
                end
                c = string.char(bc)
            end
            save(ls, c)
            next(ls)
        else
            save_and_next(ls)
        end
    end
    save_and_next(ls) -- skip delimiter
    return get_string(ls, 1, 1)
end

local function llex(ls)
    resetbuf(ls)
    while true do
        local current = ls.current
        if char_isident(current) then
            if char_isdigit(current) then -- Numeric literal.
                return 'TK_number', lex_number(ls)
            end
            repeat
                save_and_next(ls)
            until not char_isident(ls.current)
            local s = get_string(ls, 0, 0)
            local reserved = ReservedKeyword[s]
            if reserved then
                return 'TK_' .. s
            else
                return 'TK_name', s
            end
        end
        if current == '\n' or current == '\r' then
            inclinenumber(ls)
        elseif current == ' ' or current == '\t' or current == '\b' or current == '\f' then
            next(ls)
        elseif current == '-' then
            next(ls)
            if ls.current ~= '-' then return '-' end
            -- else is a comment
            next(ls)
            if ls.current == '[' then
                local sep = skip_sep(ls)
                resetbuf(ls) -- `skip_sep' may dirty the buffer
                if sep >= 0 then
                    read_long_string(ls, sep, false) -- long comment
                    resetbuf(ls)
                end
            end
            while not curr_is_newline(ls) and ls.current ~= END_OF_STREAM do
                next(ls)
            end
        elseif current == '[' then
            local sep = skip_sep(ls)
            if sep >= 0 then
                local str = read_long_string(ls, sep, true)
                return 'TK_string', str
            elseif sep == -1 then
                return '['
            else
                lex_error(ls, 'TK_string', "delimiter error")
            end
        elseif current == '=' then
            next(ls)
            if ls.current ~= '=' then return '=' else next(ls); return 'TK_eq' end
        elseif current == '<' then
            next(ls)
            if ls.current ~= '=' then return '<' else next(ls); return 'TK_le' end
        elseif current == '>' then
            next(ls)
            if ls.current ~= '=' then return '>' else next(ls); return 'TK_ge' end
        elseif current == '~' then
            next(ls)
            if ls.current ~= '=' then return '~' else next(ls); return 'TK_ne' end
        elseif current == ':' then
            next(ls)
            if ls.current ~= ':' then return ':' else next(ls); return 'TK_label' end
        elseif current == '"' or current == "'" then
            local str = read_string(ls, current)
            return 'TK_string', str
        elseif current == '.' then
            save_and_next(ls)
            if ls.current == '.' then
                next(ls)
                if ls.current == '.' then
                    next(ls)
                    return 'TK_dots' -- ...
                end
                return 'TK_concat' -- ..
            elseif not char_isdigit(ls.current) then
                return '.'
            else
                return 'TK_number', lex_number(ls)
            end
        elseif current == END_OF_STREAM then
            return 'TK_eof'
        else
            next(ls)
            return current -- Single-char tokens (+ - / ...).
        end
    end
end

local Lexer = { }

function Lexer.next(ls)
    ls.lastline = ls.linenumber
    if ls.lookahead == 'TK_eof' then -- No lookahead token?
        ls.token, ls.tokenval = llex(ls) -- Get next token.
    else
        ls.token, ls.tokenval = ls.lookahead, ls.lookaheadval
        ls.lookahead = 'TK_eof'
    end
end

local debug_file = io.open('lex-test.lua')

local function DEBUG_READ_FILE(ls)
    return debug_file:read(64)
end

local ls = lex_setup(DEBUG_READ_FILE)
local token, value
repeat
    token, value = llex(ls)
    if value then
        print(string.format('* %s:', token), value)
    else
        print(string.format('* %s', token))
    end
until token == 'TK_eof'
