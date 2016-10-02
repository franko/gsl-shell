local ffi = require 'ffi'
local cgsl = require 'gsl'

local tobit, band, rshift, lshift = bit.tobit, bit.band, bit.rshift, bit.lshift
local tonumber = tonumber

local function is_two_power(n)
    if n > 0 then
        local k = tobit(n)
        while band(k, 1) == 0 do k = rshift(k, 1) end
        return (k == 1)
    end
end

local wavelet_type_lookup = {
    daubechies          = cgsl.gsl_wavelet_daubechies,
    daubechies_centered = cgsl.gsl_wavelet_daubechies_centered,
    haar                = cgsl.gsl_wavelet_haar,
    haar_centered       = cgsl.gsl_wavelet_haar_centered,
    bspline             = cgsl.gsl_wavelet_bspline,
    bspline_centered    = cgsl.gsl_wavelet_bspline_centered,
}

local wavelet_meta, wavelet_vector_meta

local function new_wavelet(name, number)
    local wt = wavelet_type_lookup[name]
    if not wt then error("unknown wavelet type: " .. tostring(name)) end
    local w = ffi.gc(cgsl.gsl_wavelet_alloc(wt, number), cgsl.gsl_wavelet_free)
    return setmetatable({ wavelet = w }, { __index = wavelet_meta })
end

local function wavelet_vector_get(vec, j, k)
    if j < 0 then
        return vec.data[0]
    else
        local index = 1
        for jx = 0, j - 1 do
            index = index + lshift(1, jx)
        end
        return vec.data[index + k]
    end
end

local function new_wavelet_vector(n)
    local data = ffi.new("double[?]", n)
    return setmetatable({ data = data }, { __index = wavelet_vector_meta })
end

local function wavelet_transform(w, x)
    local n = tonumber(x.size1)
    if not is_two_power(n) then error("size of vector should be a power of two") end
    local ws = ffi.gc(cgsl.gsl_wavelet_workspace_alloc(n), cgsl.gsl_wavelet_workspace_free)
    local wvec = new_wavelet_vector(n)
    for i = 0, n - 1 do
        wvec.data[i] = x.data[i]
    end
    cgsl.gsl_wavelet_transform_forward(w.wavelet, wvec.data, 1, n, ws)
    return wvec
end

wavelet_meta = {
    transform = wavelet_transform,
}

wavelet_vector_meta = {
    get = wavelet_vector_get,
}

return { new = new_wavelet }
