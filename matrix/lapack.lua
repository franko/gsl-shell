local ffi = require("ffi")

local lapacke = require("lapacke")

local COL_MAJOR = lapacke.LAPACK_COL_MAJOR

local function workspace_new(n)
    return {data = ffi.new('double[?]', n), size = n}
end

local function check_info_singular(info)
    if info < 0 then
        error("internal error in solve function: invalid argument " .. tostring(-info))
    elseif info > 0 then
        error(string.format("singular matrix, U(%i,%i) = 0", info))
    end
end

-- Takes matrix in raw form. Do not check matrix size if they
-- match. Modify matrices in place.
local function gesv(Ar, P, Br)
    local n, nrhs = Ar.m, Br.n
    return lapacke.LAPACKE_dgesv(COL_MAJOR, n, nrhs, Ar.data, n, P.data, Br.data, n)
end

local function getrf(Ar, P)
    local m, n = Ar.m, Ar.n
    return lapacke.LAPACKE_dgetrf(COL_MAJOR, m, n, Ar.data, m, P.data)
end

local function getri(Ar, P, W)
    local m, n = Ar.m, Ar.n
    if not W then
        local work_query = ffi.new('double[1]')
        lapacke.LAPACKE_dgetri_work(COL_MAJOR, n, Ar.data, n, P.data, work_query, -1)
        W = workspace_new(work_query[0])
    end
    return lapacke.LAPACKE_dgetri_work(COL_MAJOR, n, Ar.data, n, P.data, W.data, W.size)
end

local function gels(trans, Ar, Br, W)
    local m, n = Ar.m, Ar.n
    local nrhs = Br.n
    local trans_char = string.byte(trans)
    if not W then
        local work_query = ffi.new('double[1]')
        lapacke.LAPACKE_dgels_work(COL_MAJOR, trans_char, m, n, nrhs, Ar.data, m, Br.data, m, work_query, -1)
        W = workspace_new(work_query[0])
    end
    return lapacke.LAPACKE_dgels_work(COL_MAJOR, trans_char, m, n, nrhs, Ar.data, m, Br.data, m, W.data, W.size)
end

return {
    gesv  = gesv,
    getrf = getrf,
    getri = getri,
    gels  = gels,

    check_info_singular = check_info_singular,
}
