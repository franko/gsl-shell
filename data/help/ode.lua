
local function get_ode()
    local REG = debug.getregistry()
    return REG['GSL.help_hook'].ODE
end

local ODE = get_ode()

local M = {
    [num.ode] = [[
num.ode {N= <int>, eps_abs= <num>, eps_rel= <num>}

   Return an ODE object to numerically integrate an ordinary
   differential equation. N is the dimension of the system and
   eps_abs, eps_rel are the requested absolute and relative 
   precision, respectively.
]]
}

if ODE then
    M[ODE.init] = [[
<ode>:init(t0, h0, f, y0_1, y0_2, ..., y0_N)

   Initialize the state of the solver to the time t0 with initial
   values y0_1, y0_2, ..., y0_N. The second argument h0 is the initial
   step size that the integrator will try. The function f is the
   function that defines the ODE system. It will be called as "f(t,
   y_1, y_2, ..., y_N)" where t is the time and y_1, y_2, ... are the
   values of the N independent values conventionally denoted here by
   y. The function f should return N values that correspond to values
   f_i(t, y_1, ..., y_N) for each component f_i of the ODE system
   function.
]]

    M[ODE.step] = [[
<ode>:step(t1)

   Advance the solution of the system by a step chosen adaptively
   based on the previous step size. The new values (t, y) are stored
   internally by the solver and can be retrieved as properties with
   the name "t" and "y" where the latter is a column matrix of size N.
   The new values of t will be less than or equal to the value given
   by t1. If the value <ode>.t is less than t1, then the function can be
   called again to further advance the ODE system.
]]

    M[ODE.evolve] = [[
 evolve(t1, t_step)

   Returns a Lua iterator that advances the ODE system at each
   iteration of a step t_step until the value t1 is reached. The
   iterator returns the value t itself and all the system variables
   y0, y1, ... up to y_N.
]]
end

return M
