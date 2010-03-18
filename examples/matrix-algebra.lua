
function fourierm(n)
   return cnew(n,n, |i,j| exp(2i*pi*(i-1)*(j-1)/n))
end

function bmatrix(n)
   return new(n, n, |i,j| j <= i and choose(i-1,j-1) or 0)
end


-- m = fourierm(8)
-- mi = cinverse(m)
