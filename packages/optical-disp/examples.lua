
require 'disp'

local function dispfit(ref, m, ps, p0)
   local nb = #ref
   local function fithook(x, f, J)
      for j = 1, #ps do m:apply(ps[j], x[j]) end
      for k = 1, nb do
	 local lam, n = ref:sample(k)
	 if f then 
	    local z = m:get_n(lam) - n
	    f:set(k, 1, z)
	 end
	 if J then
	    for j = 1, #ps do
	       local z = m:get_n_deriv(ps[j], lam)
	       J:set(k, j, z)
	    end
	 end
      end
   end
   return cnlfsolver {fdf= fithook, n= nb, p0= vector(p0)}
end

local function plotcfr(ref, m, title, get)
   get = get and get or real
   local lnr, lnm
   for lam, nref in ref:samples() do
      if not lnr then
	 lnr, lnm = path(lam, get(nref)), path(lam, get(m:get_n(lam)))
      else
	 lnr:line_to(lam, get(nref))
	 lnm:line_to(lam, get(m:get_n(lam)))
      end
   end
   local pn = plot(title)
   pn:addline(lnr, 'blue', {{'marker', size=5}})
   pn:addline(lnm, 'red', {{'dash', a=7, b=3}})
   pn:show()
   return pn
end

function demo1()
   -- ho fit for silicon oxyde
   local oxho = disp.ho {{nosc= 145, en= 15.78}}

   local ps, p0 = {'nosc:0', 'en:0'}, {170, 15.78}

   local sio2 = disp.load_nk 'packages/optical-disp/thermal-sio2.nk'

   local fit = dispfit(sio2, oxho, ps, p0)

   fit:run()

   print('Resulting parameters:', tr(fit.p))
   print('Chi square:', fit.f:norm() / #sio2)

   plotcfr(sio2, oxho, 'Fit result, n vs wavelngth')
end

function demo2first()
   -- ho fit for crystalline silicon

   local siho = disp.ho {{nosc= 186.5385, en= 10.4963, eg= 1.1442, phi= 0.0532},
			 {nosc= 0.1788, en= 3.3766, eg= 0.5510, phi= -8.1241},
			 {nosc= 0.6134, en= 4.7829, eg= 0.6031, phi= -6.5940},
			 {nosc= 1.0210, en= 4.9724, eg= 0.6961, phi= -1.9735},
			 {nosc= 0.5096, en= 3.3572, eg= 2.2388, phi= 4.7374}}

   local ps = {'nosc:0', 'nosc:1', 'nosc:2', 'nosc:3', 'nosc:4'}
   local p0 = {186, 0.18, 0.6, 1.0, 0.5}

   local siho_ref = disp.load_nk 'packages/optical-disp/si-soi-1c.nk'

   local fit = dispfit(siho_ref, siho, ps, p0)

   fit:run()

   print('Resulting parameters:', tr(fit.p))
   print('Chi square:', fit.f:norm() / #siho_ref)

   plotcfr(siho_ref, siho, 'Fit result, n vs wavelngth')
   plotcfr(siho_ref, siho, 'Fit result, n vs wavelngth', |z| -imag(z))
end

function demo2()
   -- ho fit for crystalline silicon

   local siho = disp.ho {{nosc= 186.5385, en= 10.4963, eg= 1.1442, phi= 0.0532},
			 {nosc= 0.1788, en= 3.3766, eg= 0.5510, phi= -8.1241},
			 {nosc= 0.6134, en= 4.7829, eg= 0.6031, phi= -6.5940},
			 {nosc= 1.0210, en= 4.9724, eg= 0.6961, phi= -1.9735},
			 {nosc= 0.5096, en= 3.3572, eg= 2.2388, phi= 4.7374},
			 {nosc= 0.05,   en= 5.45,   eg= 0.17,   phi= 4.2}
		      }

   local ps = {'nosc:0', 
	       'nosc:1', 'en:1', 'eg:1', 'phi:1',
	       'nosc:2', 'en:2', 'eg:2', 'phi:2',
	       'nosc:3', 'en:3', 'eg:3', 'phi:3',
	       'nosc:4', 'en:4', 'eg:4', 'phi:4',
	       'nosc:5', 'en:5', 'eg:5', 'phi:5'}
   local p0 = {186, 
	       0.18, 3.37, 0.5, -8.12, 
	       0.6, 4.78, 0.55, -6.59,
	       1.0, 4.972, 0.69, -1.97,
	       0.5, 3.357, 2.23, 4.73,
	       0.05, 5.45, 0.07, 4.2}

   local si_w = disp.load_nk 'packages/optical-disp/si-woollam-b.nk'

   local fit = dispfit(si_w, siho, ps, p0)

   print('Initial Chi square:', fit.f:norm() / #si_w)

   fit:run(200)

   print('Resulting parameters:', tr(fit.p))
   print('Final Chi square:', fit.f:norm() / #si_w)

   plotcfr(si_w, siho, 'Fit result, n vs wavelngth')
   plotcfr(si_w, siho, 'Fit result, k vs wavelngth', |z| -imag(z))
end

