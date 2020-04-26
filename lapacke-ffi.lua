local ffi = require("ffi")
local openblas = require("cblas-ffi")
local lapacke_file = io.open("lapacke_ffi.h", "rb")
local lapacke_decls = lapacke_file:read("*all")
ffi.cdef(lapacke_decls)
return openblas
