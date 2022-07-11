local ffi = require 'ffi'

ffi.cdef [[
    int chdir(const char *path);
]]

local function chdir(s)
    local err = ffi.C.chdir(s)
    if err ~= 0 then
        error("chdir() into directory %q failed.")
    end
end

function activate()
    local info = debug.getinfo(2, "S")
    if not info or not info.source then
        print("not a script: no project directory to activate")
        return
    end
    local str = info.source:sub(2)
    local path = str:match("(.*/)")
    if path then
        chdir(path)
        package.path = path .. "?.lua;" .. package.path
    else
        error("cannot locate project's directory")
    end
end

return { chdir = chdir, activate = activate }

