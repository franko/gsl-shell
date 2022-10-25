local ffi = require 'ffi'

local prev_package_path, prev_working_dir

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

local function deactivate()
    if prev_package_path then
        package.path = prev_package_path
        chdir(prev_working_dir)
        prev_package_path = nil
        prev_working_dir = nil
    end
end

local function run(filename)
    local cwd = getcwd()
    local ppath = package.path
    local path = filename:match("(.*[/\\])")
    if path then
        chdir(path)
        package.path = path .. "?.lua;" .. package.path
    end
    prev_package_path = ppath
    prev_working_dir = cwd
    local ok, err = pcall(dofile, filename)
    deactivate()
    if not ok then error(err) end
end

local function activate(dirname)
    deactivate()
    local cwd = getcwd()
    local prev_path = package.path
    local path = dirname:match("(.*)[/\\]?$")
    chdir(path)
    local dirsep = package.config:sub(1, 1)
    package.path = path .. dirsep .. "?.lua;" .. package.path
    prev_package_path = prev_path
    prev_working_dir = cwd
end

return {
    chdir = chdir,
    getcwd = getcwd,
    run = run,
    activate = activate,
    deactivate = deactivate,
}

