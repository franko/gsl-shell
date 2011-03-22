
local abs = math.abs

local function is_between(x, a, b)
   if b < a then a, b = b, a end
   return (x > a and x < b)
end

-- BRENT algorithm
local function segment_root_brent(f, a, b, eps, del)

   local fa, fb = f(a), f(b)

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

return segment_root_brent
