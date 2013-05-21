
local ffi = require 'ffi'
local gsl = require 'gsl'
local gsl_check = require 'gsl-check'
local gsl_const = require 'gsl-machine-consts'

local workspace
local workspace_n
local workspace_p

local dbl_epsilon = gsl_const.GSL_DBL_EPSILON

local function get_workspace(n, p)
   local ws
   if n == workspace_n and p == workspace_p then
      ws = workspace
   else
      ws = ffi.gc(gsl.gsl_multifit_linear_alloc(n, p),
                  gsl.gsl_multifit_linear_free)

      workspace_n = n
      workspace_p = p
      workspace   = ws
   end
   return ws
end

local function linfit_svd_tol(X, y, w, tol)
   local n, p = matrix.dim(X)
   local ws = get_workspace(n, p)
   local c = matrix.alloc(p, 1)
   local cov = matrix.alloc(p, p)
   local yv = gsl.gsl_matrix_column (y, 0)
   local cv = gsl.gsl_matrix_column (c, 0)

   local rank = ffi.new('size_t[1]')
   local chisq = ffi.new('double[1]')

   if w then
      local wv = gsl.gsl_matrix_column (w, 0)
      gsl_check(gsl.gsl_multifit_wlinear_svd (X, wv, yv, tol, rank, cv, cov, chisq, ws))
   else
      gsl_check(gsl.gsl_multifit_linear_svd (X, yv, tol, rank, cv, cov, chisq, ws))
   end

   return c, chisq[0], cov, tonumber(rank[0])
end

function num.linfit(X, y, w)
   local c, chisq, cov = linfit_svd_tol(X, y, w, dbl_epsilon)
   return c, chisq, cov
end

function num.linfit_svd(X, y, w, tol)
   return linfit_svd_tol(X, y, w, tol)
end
