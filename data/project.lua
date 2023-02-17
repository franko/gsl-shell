local std_package_path, std_working_dir = package.path, filesystem.getcwd()

local function deactivate()
    package.path = std_package_path
    filesystem.chdir(std_working_dir)
end

local function run(path, filename)
    if not filename then
        filename = path
        path = filename:match("(.*[/\\])")
    end
    if path then
        filesystem.chdir(path)
        package.path = std_package_path .. "?.lua;" .. package.path
    end
    dofile(filename)
    deactivate()
end

local function activate(dirname)
    local path = dirname:match("(.*)[/\\]?$")
    if path then
        filesystem.chdir(path)
        local dirsep = package.config:sub(1, 1)
        package.path = std_package_path .. dirsep .. "?.lua;" .. package.path
    end
end

return {
    run = run,
    activate = activate,
    deactivate = deactivate,
}

