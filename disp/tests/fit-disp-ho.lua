
require 'disp'

d = disp.ho {{nosc= 145, en= 15.78}}

ps = {'nosc:0', 'en:0'}
x0 = vector {170, 15.78}

ref = disp.load_nk 'disp/tests/thermal-sio2.nk'

nb = ref:length()

function ho_fit(x, f, J)
   for p = 1, #ps do d:apply(ps[p], x[p]) end
   for k = 1, nb do
      local t = ref:sample(k)
      if f then 
	 local z = d:get_n(t.lambda) - t.n
	 f:set(k, 1, z)
      end
      if J then
	 for p = 1, #ps do
	    local z = d:get_n_deriv(ps[p], t.lambda)
	    J:set(k, p, z)
	 end
      end
   end
end

s = csolver {fdf= ho_fit, n= nb, p= #ps, x0= x0}

function print_cfr(ref, m)
   for k = 0, ref:length()-1 do
      local t = ref:sample(k)
      print(t.lambda, t.n, m:get_n(t.lambda))
   end
end
