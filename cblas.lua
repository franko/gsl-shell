local ffi = require("ffi")
local cblas_file = io.open("cblas_ffi.h", "rb")
local cblas_decls = cblas_file:read("*all")
ffi.cdef(cblas_decls)
local openblas_lib = ffi.load("openblas")
return openblas_lib
