--This is a test script for all the sf. methods

--prints all available special functions in the name scope
--for k,v in pairs(t.) do print  = function(test) test{sf.) do print (k)} end  end

local t={}
t.Chi = function(test) test{sf.Chi(1)} end 
t.Ci = function(test) test{sf.Ci(1)} end 
t.Shi = function(test) test{sf.Shi(1)} end 
t.Si = function(test) test{sf.Si(1)} end 

t.airyAi = function(test) test{sf.airyAi(1)} end 
t.airyAi_deriv = function(test) test{sf.airyAi_deriv(1)} end 
t.airyAi_deriv_scaled = function(test) test{sf.airyAi_deriv_scaled(1)} end 
t.airyAi_scaled = function(test) test{sf.airyAi_scaled(1)} end 
t.airyAi_zero = function(test) test{sf.airyAi_zero(1)} end 
t.airyAi_deriv_zero = function(test) test{sf.airyAi_deriv_zero(1)} end 
t.airyBi = function(test) test{sf.airyBi(1)} end 
t.airyBi_deriv = function(test) test{sf.airyBi_deriv(1)} end 
t.airyBi_deriv_scaled = function(test) test{sf.airyBi_deriv_scaled(1)} end 
t.airyBi_scaled = function(test) test{sf.airyBi_scaled(1)} end 
t.airyBi_zero = function(test) test{sf.airyBi_zero(1)} end 
t.airyBi_deriv_zero = function(test) test{sf.airyBi_deriv_zero(1)} end 

t.atanint = function(test) test{sf.atanint(1)} end 

t.besselI = function(test) test{sf.besselI(1,1)} end 
t.besselI_scaled = function(test) test{sf.besselI_scaled(1,1)} end 
t.besselInu = function(test) test{sf.besselInu(1,1)} end 
t.besselInu_scaled = function(test) test{sf.besselInu_scaled(1,1)} end 
t.besselJ = function(test) test{sf.besselJ(1,1)} end 
t.besselJnu = function(test) test{sf.besselJnu(1,1)} end 
t.besselJ_zero = function(test) test{sf.besselJ_zero(1,1)} end 
t.besselK = function(test) test{sf.besselK(1,1)} end 
t.besselK_scaled = function(test) test{sf.besselK_scaled(1,1)} end 
t.besselKnu = function(test) test{sf.besselKnu(1,1)} end 
t.besselKnu_scaled = function(test) test{sf.besselKnu_scaled(1,1)} end 
t.besselY = function(test) test{sf.besselY(1,1)} end 
t.besselYnu = function(test) test{sf.besselYnu(1,1)} end 
t.besseli_scaled = function(test) test{sf.besseli_scaled(1,1)} end 
t.besselj = function(test) test{sf.besselj(1,1)} end 
t.besselk_scaled = function(test) test{sf.besselk_scaled(1,1)} end 
t.bessellnKnu = function(test) test{sf.bessellnKnu(1,1)} end 
t.bessely = function(test) test{sf.bessely(1,1)} end 

t.beta = function(test) test{sf.beta(1,1)} end 
t.beta_inc = function(test) test{sf.beta_inc(1,1,1)} end 

t.choose = function(test) test{sf.choose(1,1)} end 
t.doublefact = function(test) test{sf.doublefact(1)} end 
t.fact = function(test) test{sf.fact(1)} end 

t.clausen = function(test) test{sf.clausen(1)} end 

t.conicalP = function(test) test{sf.conicalP(1,1,1)} end 
t.conicalPcylreg = function(test) test{sf.conicalPcylreg(1,1,1)} end 
t.conicalPsphreg = function(test) test{sf.conicalPsphreg(1,1,1)} end 

t.coulomb_CL = function(test) test{sf.coulomb_CL(1,1)} end 
t.coulomb_wave_FG = function(test) test{sf.coulomb_wave_FG(1, 1, 1,1)} end 
--t.hydrogenicR = function(test) test{sf.hydrogenicR(1,1,1,1)} end 
--t.hydrogenicR_1 = function(test) test{sf.hydrogenicR_1(1,1)} end 

t.coupling_3j = function(test) test{sf.coupling_3j(1,1,1,1,1,1)} end 
t.coupling_6j = function(test) test{sf.coupling_6j(1,1,1,1,1,1)} end 
t.coupling_9j = function(test) test{sf.coupling_9j(1,1,1,1,1,1,1,1,1)} end 

t.dawson = function(test) test{sf.dawson(1)} end 

t.debye = function(test) test{sf.debye(1,1)} end 

t.dilog = function(test) test{sf.dilog(1)} end 
t.cdilog = function(test) test{sf.cdilog(1+1i)} end 

t.ellint_D = function(test) test{sf.ellint_D(1,1,1)} end 
t.ellint_E = function(test) test{sf.ellint_E(1,1)} end 
--t.ellint_Ecomp = function(test) test{sf.ellint_Ecomp(1)} end 
t.ellint_F = function(test) test{sf.ellint_F(1,1)} end 
--t.ellint_Kcomp = function(test) test{sf.ellint_Kcomp(1)} end 
t.ellint_P = function(test) test{sf.ellint_P(1,1,1)} end 
t.ellint_RC = function(test) test{sf.ellint_RC(1,1)} end 
t.ellint_RD = function(test) test{sf.ellint_RD(1,1,1)} end 
t.ellint_RF = function(test) test{sf.ellint_RF(1,1,1)} end 
t.ellint_RJ = function(test) test{sf.ellint_RJ(1,1,1,1)} end 
t.elljac = function(test) test{sf.elljac(1,1)} end 

t.erf = function(test) test{sf.erf(1)} end 
t.erf_Q = function(test) test{sf.erf_Q(1)} end 
t.erf_Z = function(test) test{sf.erf_Z(1)} end 
t.erfc = function(test) test{sf.erfc(1)} end 
t.log_erfc = function(test) test{sf.log_erfc(1)} end 

t.eta = function(test) test{sf.eta(1)} end 

t.exp = function(test) test{sf.exp(1)} end 
t.exp_err = function(test) test{sf.exp_err(1, 1)} end 
t.exp_mult = function(test) test{sf.exp_mult(1,1)} end 
t.exp_mult_err = function(test) test{sf.exp_mult_err(1,1,1,1)} end 

t.expint3 = function(test) test{sf.expint3(1)} end 
t.expint_Ei = function(test) test{sf.expint_Ei(1)} end 
t.expint_E = function(test) test{sf.expint_E(1,1)} end 

t.expm1 = function(test) test{sf.expm1(1)} end 
t.exprel = function(test) test{sf.exprel(1)} end 
t.exprel_2 = function(test) test{sf.exprel_2(1)} end 
t.exprel_n = function(test) test{sf.exprel_n(1,1)} end 


t.fermi_dirac = function(test) test{sf.fermi_dirac(1,1)} end 
t.fermi_dirac_inc = function(test) test{sf.fermi_dirac_inc(1,1)} end 

t.gamma = function(test) test{sf.gamma(1)} end 
t.gamma_inc = function(test) test{sf.gamma_inc(1,1)} end 
t.gamma_inc_P = function(test) test{sf.gamma_inc_P(1,1)} end 
t.gamma_inc_Q = function(test) test{sf.gamma_inc_Q(1,1)} end 
t.lngammac = function(test) test{sf.lngammac(1+1i)} end 
t.gammainv = function(test) test{sf.gammainv(1)} end 
t.gammastar = function(test) test{sf.gammastar(1)} end 

--t.gegenpoly = function(test) test{sf.gegenpoly(1,1,1)} end 

t.hyperg0F1 = function(test) test{sf.hyperg0F1(1,1)} end 
t.hyperg1F1 = function(test) test{sf.hyperg1F1(1,1,1)} end 
--t.hyperg2F0 = function(test) test{sf.hyperg2F0(1,1,1)} end 
--t.hyperg2F1 = function(test) test{sf.hyperg2F1(1,1,1,1)} end 
--t.hyperg2F1_renorm = function(test) test{sf.hyperg2F1_renorm(1,1,1,1)} end 
--t.hyperg2F1conj = function(test) test{sf.hyperg2F1conj(1+1i, 1,1)} end 
--t.hyperg2F1conj_renorm = function(test) test{sf.hyperg2F1conj_renorm(1+1i, 1,1)} end 
t.hypergU = function(test) test{sf.hypergU(1,1,1)} end 


t.laguerre = function(test) test{sf.laguerre(1,1,1)} end 

t.lambertW0 = function(test) test{sf.lambertW0(1)} end 
t.lambertWm1 = function(test) test{sf.lambertWm1(1)} end 

t.legendreH3d = function(test) test{sf.legendreH3d(1,1,1)} end 
t.legendreP = function(test) test{sf.legendreP(1,1)} end 
t.legendrePlm = function(test) test{sf.legendrePlm(1,1,1)} end 
--t.legendreQ = function(test) test{sf.legendreQ(1,1)} end 
t.legendresphPlm = function(test) test{sf.legendresphPlm(1,1,1)} end 

t.lnbeta = function(test) test{sf.lnbeta(1,1)} end 
t.lnchoose = function(test) test{sf.lnchoose(1,1)} end 
t.lndoublefact = function(test) test{sf.lndoublefact(1)} end 
t.lnfact = function(test) test{sf.lnfact(1)} end 
t.lngamma = function(test) test{sf.lngamma(1)} end 
t.lnpoch = function(test) test{sf.lnpoch(1,1)} end 

t.log = function(test) test{sf.log(1)} end 
t.log_1plusx = function(test) test{sf.log_1plusx(1)} end 
t.log_1plusx_mx = function(test) test{sf.log_1plusx_mx(1)} end 
t.log_abs = function(test) test{sf.log_abs(1)} end 

t.poch = function(test) test{sf.poch(1,1)} end 
t.pochrel = function(test) test{sf.pochrel(1,1)} end 

t.pow_int = function(test) test{sf.pow_int(1,1)} end 

t.psi = function(test) test{sf.psi(1)} end 
t.psi_1 = function(test) test{sf.psi_1(1)} end 
t.psi_1piy = function(test) test{sf.psi_1piy(1)} end 
t.psi_n = function(test) test{sf.psi_n(1,1)} end 

t.synchrotron1 = function(test) test{sf.synchrotron1(1)} end 
t.synchrotron2 = function(test) test{sf.synchrotron2(1)} end 

t.taylorcoeff = function(test) test{sf.taylorcoeff(1,1)} end 

t.transport = function(test) test{sf.transport(2,1)} end 

t.zeta = function(test) test{sf.zeta(0.9)} end 
t.zetam1 = function(test) test{sf.zetam1(0.9)} end 
t.hzeta = function(test) test{sf.hzeta(2,0.9)} end 
return t
