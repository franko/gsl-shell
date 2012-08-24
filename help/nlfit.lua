
local REG = debug.getregistry()
NLFIT = REG['GSL.NLINFIT']

local M = {
   [num.nlinfit] = [[
num.nlinfit {n= <int>, p= <int>}

   Create a non-linear fit solver object for a system of dimension "n"
   with "p" fitting parameters.
]],

   [NLFIT.set] = [[
<nlfit>:set(fdf, x0)

    Associate the non-linear fit solver with the user-defined function
    fdf and set the initial condition for the fit parameters to x0.
    The function fdf will be called in the form

    fdf(x, f, J)

    where x is a column matrix with the fit parameters, f is either
    nil or a column matrix of dimension n to store the values f(x_i) -
    y_i. J is either nil or a matrix of dimension n x p to store the
    Jacobian of f versus the fit parameters:

    J[i, j] = d f_i / d x_j

    The function fdf should set all the elements of f and J if they
    are not nil.
]],

   [NLFIT.iterate] = [[
<nlfit>:iterate()

    Advance the solver of a single step. It does return "continue" if
    it did not reach the optimal point and "success" otherwise.
]],

   [NLFIT.test] = [[
<nlfit>:test(eps_abs, eps_err)

    Check if the the search converged for the given absolute error
    eps_abs and relative error eps_rel.
]],
}

return M
