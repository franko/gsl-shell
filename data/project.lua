local ffi = require 'ffi'

if ffi.os == "Windows" then
    ffi.cdef [[
        int _chdir(const char *path);
    ]]
else
    ffi.cdef [[
        int chdir(const char *path);
    ]]
end

local function chdir(s)
    local err
    if ffi.os == "Windows" then
        err = ffi.C._chdir(s)
    else
        err = ffi.C.chdir(s)
    end
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
    local path = str:match("(.*[/\\])")
    if path then
        chdir(path)
        package.path = path .. "?.lua;" .. package.path
    else
        error("cannot locate project's directory")
    end
end

return { chdir = chdir, activate = activate }

