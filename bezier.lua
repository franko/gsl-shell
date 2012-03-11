
local matrix = matrix.new
local linfit = num.linfit
local divmod = math.divmod

local function bernstein_cubic_fit(xf, yf, i0, i1)
   local x0, x1 = xf(i0), xf(i1)
   local y0, y1 = yf(i0), yf(i1)

   local function model(i, j)
      local t = (xf(i+i0) - x0) / (x1 - x0)
      return j == 1 and 3*t*(1-t)^2 or 3*t^2*(1-t)
   end

   local function y_resid(i)
      local t, y = (xf(i+i0) - x0) / (x1 - x0), yf(i+i0)
      local y1st = (1-t)^3 * y0 + t^3 * y1
      return y - y1st
   end

   local N = i1 - i0 - 1

   local X = matrix(N, 2, model)
   local Y = matrix(N, 1, y_resid)

   return linfit(X, Y)
end

local function bezier_approx(xf, yf, N, tol, debug)

   local x1, y1 = xf(1), yf(1)

   local ln = graph.curve(x1, y1)
   local pt, mp, sg

   if debug then
      pt, sg, mp = graph.path(x1, y1), graph.path(x1, y1), graph.path()
   end

   local function interval_approx(i0, i1)
      local n = i1 - i0 + 1

      -- Number of additional points required for bezier fit.
      -- Using the bare minimum of zero can lead to artifacts.
      local n_excess = 2

      if n < 4 + n_excess then
	 for k = i0+1, i1 do
	    ln:line_to(xf(k), yf(k))
	 end
	 if debug then
	    sg:move_to(xf(i1), yf(i1))
	 end
      else

	 local x0, y0 = xf(i0), yf(i0)
	 local c, chisq = bernstein_cubic_fit(xf, yf, i0, i1)
	 if chisq / (n - 2) < tol then
	    local x1, y1 = xf(i1), yf(i1)
	    local xi1, xi2 = x0 + (x1 - x0)/3, x0 + 2*(x1 - x0)/3
	    ln:curve4(xi1, c[1], xi2, c[2], x1, y1)
	    if debug then
	       pt:line_to(x1, y1)
	       mp:line_to(xi1, c[1])
	       mp:line_to(xi2, c[2])
	       sg:line_to(xi1, c[1])
	       sg:move_to(xi2, c[2])
	       sg:line_to(x1, y1)
	    end
	 else
	    local im = i0 + divmod(i1-i0, 2)
	    interval_approx(i0, im)
	    interval_approx(im, i1)
	 end
      end
   end

   interval_approx(1, N)

   return ln, pt, mp, sg
end

return {approx= bezier_approx}
