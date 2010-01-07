
f = function(x, g)
       local xc = vector {4.45, -1.2}
       local y = x - xc
       if g then set(g, 2*y) end
       return prod(y, y)[1]
    end

x0 = vector {-1, 0.9}
m = minimizer(f, x0:dims())
m:set(x0, 0.5)
