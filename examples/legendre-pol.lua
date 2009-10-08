
function array_imap(ls, f)
   local n = {}
   for i, v in ipairs(ls) do 
      local nv = f(i, v)
      if nv then n[#n+1] = nv end
   end
   return n
end

function array_ireduce(ls, f, accu)
   for i, v in ipairs(ls) do accu = f(accu, i, v) end
   return accu
end

function bin_coeff(n, k)
   local r = 1
   for j=1, k do r = r * (n-j+1) end
   for j=1, k do r = r / j end
   return r
end

function poly_scalar_mul(p, s)
   if s == 0 then return {} end
   return array_imap(p, function(i, x) return x*s end)
end

function poly_degree(p) 
   return #p - 1
end

function poly_term(p, i)
   local v = p[i+1]
   return v and v or 0
end
 
function poly_mul(p, q)
   local pdeg, qdeg = poly_degree(p), poly_degree(q)
   local rdeg = pdeg + qdeg
   local r = {}
   for nu=0, rdeg do
      local x = 0
      for k=0, nu do 
	 if k <= pdeg and nu-k <= qdeg then x = x + p[k+1] * q[nu-k+1] end
      end
      r[nu+1] = x
   end
   return r
end

function poly_add(p, q)
   local pdeg, qdeg = poly_degree(p), poly_degree(q)
   local rdeg = pdeg > qdeg and pdeg or qdeg
   for k=0, rdeg do
      local v = poly_term(p, k) + poly_term(q, k) 
      p[k+1] = (v ~= 0 and v or nil) 
   end
end

function poly_x_power_mul(p, coeff, xdeg)
   if coeff == 0 then return {} end
   local pn = {}
   for k=1, xdeg do table.insert(pn, 0) end
   for k=1, poly_degree(p)+1 do table.insert(pn, coeff * p[k]) end
   return pn
end

function print_poly(p)
   local function fclean(i, x)
      if x ~= 0 then
	 local l = {}
	 local sign = x > 0 and '+' or '-'
	 if x < 0 then x = -x end
	 if x ~= 1 or i == 1 then 
	    l[#l+1] = string.format('%g', x)
	 end
	 if i > 1 then
	    l[#l+1] = i == 2 and 'x' or string.format('x^%i', i-1)
	 end
	 return {sign= sign, text= table.concat(l, ' ')}
      end
   end
   local function fsign(t, k, nd)
      if nd.sign == '-' or k > 1 then t[#t+1] = nd.sign end
      t[#t+1] = nd.text
      return t
   end
   local s = table.concat(array_ireduce(array_imap(p, fclean), fsign, {}), ' ')
   print(s ~= '' and s or '0')
end

function poly_eval(p, x)
   local r = 0
   for k = #p, 1, -1 do r = r * x + p[k] end
   return r
end

--[[
function poly_derive(p)
   local nu = poly_degree(p)
   local r = {}
   for k=1, nu do r[k] = k * p[k+1] end
   return r
end

function poly_integ(p, c0)
   local nu = poly_degree(p)
   table.insert(p, 1, c0)
   for k=1, nu+1 do p[k+1] = p[k+1] / k end
   return p
end
--]]

function legendre_pol(n)
   local p = {}
   local nfact = 1 / pow(2,n)
   for k=0, n/2 do
      local s = 1 - 2*(k % 2)
      p[n-2*k+1] = s * bin_coeff(n,k) * bin_coeff(2*(n-k),n) * nfact
      if n > 2*k then p[n-2*k] = 0 end
   end
   return p
end
