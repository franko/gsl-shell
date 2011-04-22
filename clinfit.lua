
require 'cmatrix'

local ffi    = require 'ffi'
local cgsl   = require 'cgsl'

local gsl_check = require 'gsl-check'

ffi.cdef[[
      typedef struct 
      {
	 size_t n; /* number of observations */
	 size_t p; /* number of parameters */
	 gsl_matrix * A;
	 gsl_matrix * Q;
	 gsl_matrix * QSI;
	 gsl_vector * S;
	 gsl_vector * t;
	 gsl_vector * xt;
	 gsl_vector * D;
      } gsl_multifit_linear_workspace;

      gsl_multifit_linear_workspace * gsl_multifit_linear_alloc (size_t n, size_t p);

      void gsl_multifit_linear_free (gsl_multifit_linear_workspace * work);

      int
      gsl_multifit_linear (const gsl_matrix * X,
			   const gsl_vector * y,
			   gsl_vector * c,
			   gsl_matrix * cov,
			   double * chisq,
			   gsl_multifit_linear_workspace * work);

      int
      gsl_multifit_wlinear (const gsl_matrix * X,
			    const gsl_vector * w,
			    const gsl_vector * y,
			    gsl_vector * c,
			    gsl_matrix * cov,
			    double * chisq,
			    gsl_multifit_linear_workspace * work);
]]

local workspace
local workspace_n
local workspace_p

local function get_workspace(n, p)
   local ws
   if n == workspace_n and p == workspace_p then
      ws = workspace
   else
      ws = ffi.gc(cgsl.gsl_multifit_linear_alloc(n, p),
		  cgsl.gsl_multifit_linear_free)

      workspace_n = n
      workspace_p = p
      workspace   = ws
   end
   return ws
end

local chisq = ffi.new('double[1]')

local function linfit(X, y, w)
   local n, p = matrix.dim(X)
   local ws = get_workspace(n, p)
   local c = matrix.alloc(p, 1)
   local cov = matrix.alloc(p, p)
   local yv = cgsl.gsl_matrix_column (y, 0)
   local cv = cgsl.gsl_matrix_column (c, 0)
   
   if w then
      local wv = cgsl.gsl_matrix_column (w, 0)
      gsl_check(cgsl.gsl_multifit_wlinear (X, wv, yv, cv, cov, chisq, ws))
   else
      gsl_check(cgsl.gsl_multifit_linear (X, yv, cv, cov, chisq, ws))
   end

   return c, chisq[0], cov
end

return linfit
