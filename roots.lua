
local abs, max, min = math.abs, math.max, math.min

local function is_between(x, a, b)
   if b < a then a, b = b, a end
   return (x > a and x < b)
end

-- BRENT algorithm
local function brent(f, a, fa, b, fb, eps, del)

   if abs(fa) < abs(fb) then
      a, b = b, a
      fa, fb = fb, fa 
   end

   local mflag = true
   local c, fc = a, fa
   local d

   local s, fs = b, fb

   while abs(fs) >= eps and abs(b-a) > del do
      if fa ~= fc and fb ~= fc then
	 s = a*fb*fc/((fa-fb)*(fa-fc)) + b*fa*fc/((fb-fa)*(fb-fc)) + c*fa*fb/((fc-fa)*(fc-fb))
      else
	 s = b - fb*(b-a)/(fb-fa)
      end

      if not is_between(s, (3*a+b)/4, b)
         or (    mflag and abs(s-b) >= abs(b-c)/2)
         or (not mflag and abs(s-b) >= abs(c-d)/2)
         or (    mflag and abs(b-c) < del)
         or (not mflag and abs(c-d) < del) then
	 s = (a+b)/2
	 mflag = true
      else
	 mflag = false
      end
      fs = f(s)
      d = c
      c, fc = b, fb
      if fa * fs < 0 then b, fb = s, fs else a, fa = s, fs end

      if abs(fa) < abs(fb) then
	 a, b = b, a
	 fa, fb = fb, fa 
      end
   end

   return s
end

local function segment_root_brent(f, a, b, eps, del)
   local fa, fb = f(a), f(b)
   return brent(f, a, fa, b, fb, eps, del)
end

local function lagrange_quad_est(x0, f0, x1, f1, x2, f2)
   local dx01, dx12, dx20 = x0 - x1, x1 - x2, x2 - x0
   local a0 = - f0 / (dx01 * dx20)
   local a1 = - f1 / (dx01 * dx12)
   local a2 = - f2 / (dx20 * dx12)
   return a0, a1, a2
end

local function lagrange_quad_eval(a0, a1, a2, x0, x1, x2, x)
   return a0 * (x-x1)*(x-x2) + a1 * (x-x0)*(x-x2) + a2 * (x-x0)*(x-x1)
end

local function solver_add_root(s, x)
   local rs = s.roots
   rs[#rs+1] = x
end

local function solver_get_random(s)
   return s.rng:get()
end

local function root_locate (s, xa, fa, xb, fb)
   if fa * fb < 0 then
      local f = s.f
      local eps, del = s.eps, s.del
      if s.scale_f then eps = eps * s.scale_f((xa+xb)/2) end
      local x = brent(f, xa, fa, xb, fb, eps, del)
      solver_add_root(s, x)
   end
end

local function f_quad_min (a0, a1, a2, x0, x1, x2)
   local a = 2*(a0+a1+a2)
   if a ~= 0 then
      return (a0*(x1+x2) + a1*(x0+x2) + a2*(x0+x1)) / a
   end
end

local function f_approx_test(s, fabsm, a0, a1, a2, x0, xm, x1)
   for i=1, 8 do
      local r = solver_get_random(s)
      local x = x0 + r * (x1 - x0)
      local fx = s.f(x)
      local fe = lagrange_quad_eval(a0, a1, a2, x0, xm, x1, x)
      if abs(fx - fe) > 0.01 * fabsm then return false end
   end
   return true
end

local function interval_roots (s, x0, f0, x1, f1)
   local f = s.f
   local xm = (x0+x1)/2
   local fm = f(xm)
   local a0, a1, a2 = lagrange_quad_est(x0, f0, xm, fm, x1, f1)

   local fabsm = max(abs(f0), abs(f1), abs(fm))

   if f_approx_test(s, fabsm, a0, a1, a2, x0, xm, x1) then
      local xi, fi
      local xmin = f_quad_min (a0, a1, a2, x0, xm, x1)
      if xmin and xmin < x1 and xmin > x0 then
	 xi, fi = xmin, f(xmin)
      else
	 xi, fi = xm, fm
      end

      if f0 == 0 then solver_add_root(s, x0) end
      if fi == 0 then solver_add_root(s, xi) end

      if f0 ~= 0 and fi ~= 0 then
	 root_locate(s, x0, f0, xi, fi)
      end

      if fi ~= 0 and f1 ~= 0 then
	 root_locate(s, xi, fi, x1, f1)
      end
   else
      interval_roots (s, x0, f0, xm, fm)
      interval_roots (s, xm, fm, x1, f1)
   end
end

local function solver_tolerance(s, eps, del)
   s.eps = eps
   s.del = del
end

local function solver_root(s, x0, x1)
   local f = s.f
   return brent(f, x0, f(x0), x1, f(x1), s.eps, s.del)
end

local function solver_interval_solve(s, x0, x1, roots)
   local f = s.f
   s.roots = roots or {}
   s.rng = s.rng or rng.new()
   interval_roots(s, x0, f(x0), x1, f(x1))
   if f(x1) == 0 then solver_add_root(s, x1) end
   return s.roots
end

local function root_solver_new (f, eps, del, scale_f)
   return {f= f, eps= eps, del= del,
	   scale_f = scale_f,
	   tolerance = solver_tolerance,
	   root = solver_root,
	   solve = solver_interval_solve
	}
end

return {solver = root_solver_new}
