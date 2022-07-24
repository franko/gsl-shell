local ffi = require 'ffi'

if ffi.os == "Windows" then
    ffi.cdef [[
        int _chdir(const char *path);
        char *_getcwd(char *buffer, int maxlen);
    ]]
else
    ffi.cdef [[
        int chdir(const char *path);
        char *getcwd(char *buf, size_t size);
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
        error("chdir() into directory " .. s .. " failed.")
    end
end

local function getcwd()
    local getcwd_fn = ffi.C[ffi.os == "Windows" and "_getcwd" or "getcwd"]
    local buf = ffi.gc(getcwd_fn(nil, 0), ffi.C.free)
    assert(buf, "error calling getcwd()")
    return ffi.string(buf)
end

local function run(filename)
    local cwd = getcwd()
    local ppath = package.path
    local path = filename:match("(.*[/\\])")
    if path then
        chdir(path)
        package.path = path .. "?.lua;" .. package.path
    end
    dofile(filename)
    package.path = ppath
    chdir(cwd)
end

return { chdir = chdir, getcwd = getcwd, run = run }

