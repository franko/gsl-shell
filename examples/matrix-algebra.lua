
function fourierm(n)
   return cnew(n,n, |i,j| exp(2i*pi*(i-1)*(j-1)/n))
end

function bmatrix(n)
   return new(n, n, |i,j| j <= i and choose(i-1,j-1) or 0)
end

function hilbert(n)
   return new(n, n, |i,j| 1/(i+j-1))
end

function vandermonde(x)
   local n = #x
   return new(n, n, |i,j| x[i]^(n-j))
end

function randhmat(n)
  local r = rng()
  local m = cnew(n, n)
  for i=1,n do
    for j=1,i do
      if i == j then m:set(i, j, r:getint(12))
      else
        local v = r:getint(12) + 1i * r:getint(12)
        m:set(i, j, v)
        m:set(j, i, conj(v))
     end
    end
  end
  return m
end

-- m = fourierm(8)
-- mi = cinverse(m)
