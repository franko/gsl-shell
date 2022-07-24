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
    local buf_size = 256
    while buf_size <= 8192 do
        local buf = ffi.new("char[?]", buf_size)
        local r
        if ffi.os == "Windows" then
            r = ffi.C._getcwd(buf, buf_size)
        else
            r = ffi.C.getcwd(buf, buf_size)
        end
        if r then
            return ffi.string(buf)
        else
            buf_size = buf_size * 4
        end
    end
    error("error calling getcwd()")
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

