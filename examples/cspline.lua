
require 'cspline'

-- x = {581.8, 978.11, 3308}
-- y = {584, 1000, 3536}
x = {581.8, 978.11}
y = {584, 985}

f = cspline(x, y)

p = fxplot(f, 0, 3500)
p:addline(ipath(sequence(function(j) return x[j], y[j] end, #x)), 'blue', {{'marker', size= 6}})

x1 = {581.8, 978.11}
y1 = {596, 982}

f1 = cspline(x1, y1)

p:addline(fxline(f1, 0, 3500), 'green', {{'dash', a=7, b=3}})

p1 = fxplot(|x| f1(x) - f(x), 0, 3500)
