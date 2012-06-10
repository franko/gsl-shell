--This is a test script for all the sf. methods

--prints all available special functions in the name scope
--for k,v in pairs(t.) do print  = function() return sf.) do print (k) end  end

local t = {}
t.Chi = function() return sf.Chi(1) end
t.Ci = function() return sf.Ci(1) end
t.Shi = function() return sf.Shi(1) end
t.Si = function() return sf.Si(1) end

t.airyAi = function() return sf.airyAi(1) end
t.airyAi_deriv = function() return sf.airyAi_deriv(1) end
t.airyAi_deriv_scaled = function() return sf.airyAi_deriv_scaled(1) end
t.airyAi_scaled = function() return sf.airyAi_scaled(1) end
t.airyAi_zero = function() return sf.airyAi_zero(1) end
t.airyAi_deriv_zero = function() return sf.airyAi_deriv_zero(1) end
t.airyBi = function() return sf.airyBi(1) end
t.airyBi_deriv = function() return sf.airyBi_deriv(1) end
t.airyBi_deriv_scaled = function() return sf.airyBi_deriv_scaled(1) end
t.airyBi_scaled = function() return sf.airyBi_scaled(1) end
t.airyBi_zero = function() return sf.airyBi_zero(1) end
t.airyBi_deriv_zero = function() return sf.airyBi_deriv_zero(1) end

t.atanint = function() return sf.atanint(1) end

t.besselI = function() return sf.besselI(1,1) end
t.besselI_scaled = function() return sf.besselI_scaled(1,1) end
t.besselInu = function() return sf.besselInu(1,1) end
t.besselInu_scaled = function() return sf.besselInu_scaled(1,1) end
t.besselJ = function() return sf.besselJ(1,1) end
t.besselJnu = function() return sf.besselJnu(1,1) end
t.besselJ_zero = function() return sf.besselJ_zero(1,1) end
t.besselK = function() return sf.besselK(1,1) end
t.besselK_scaled = function() return sf.besselK_scaled(1,1) end
t.besselKnu = function() return sf.besselKnu(1,1) end
t.besselKnu_scaled = function() return sf.besselKnu_scaled(1,1) end
t.besselY = function() return sf.besselY(1,1) end
t.besselYnu = function() return sf.besselYnu(1,1) end
t.besseli_scaled = function() return sf.besseli_scaled(1,1) end
t.besselj = function() return sf.besselj(1,1) end
t.besselk_scaled = function() return sf.besselk_scaled(1,1) end
t.bessellnKnu = function() return sf.bessellnKnu(1,1) end
t.bessely = function() return sf.bessely(1,1) end

t.beta = function() return sf.beta(1,1) end
t.beta_inc = function() return sf.beta_inc(1,1,1) end

t.choose = function() return sf.choose(1,1) end
t.doublefact = function() return sf.doublefact(1) end
t.fact = function() return sf.fact(1) end

t.clausen = function() return sf.clausen(1) end

t.conicalP = function() return sf.conicalP(1,1,1) end
t.conicalPcylreg = function() return sf.conicalPcylreg(1,1,1) end
t.conicalPsphreg = function() return sf.conicalPsphreg(1,1,1) end

t.coulomb_CL = function() return sf.coulomb_CL(1,1) end
t.coulomb_wave_FG = function() return {sf.coulomb_wave_FG(1, 1, 1,1)} end
--t.hydrogenicR = function() return sf.hydrogenicR(1,1,1,1) end
--t.hydrogenicR_1 = function() return sf.hydrogenicR_1(1,1) end

t.coupling_3j = function() return sf.coupling_3j(1,1,1,1,1,1) end
t.coupling_6j = function() return sf.coupling_6j(1,1,1,1,1,1) end
t.coupling_9j = function() return sf.coupling_9j(1,1,1,1,1,1,1,1,1) end

t.dawson = function() return sf.dawson(1) end

t.debye = function() return sf.debye(1,1) end

t.dilog = function() return sf.dilog(1) end
t.cdilog = function() return sf.cdilog(1+1i) end

t.ellint_D = function() return sf.ellint_D(1,1,1) end
t.ellint_E = function() return sf.ellint_E(1,1) end
--t.ellint_Ecomp = function() return sf.ellint_Ecomp(1) end
t.ellint_F = function() return sf.ellint_F(1,1) end
--t.ellint_Kcomp = function() return sf.ellint_Kcomp(1) end
t.ellint_P = function() return sf.ellint_P(1,1,1) end
t.ellint_RC = function() return sf.ellint_RC(1,1) end
t.ellint_RD = function() return sf.ellint_RD(1,1,1) end
t.ellint_RF = function() return sf.ellint_RF(1,1,1) end
t.ellint_RJ = function() return sf.ellint_RJ(1,1,1,1) end
t.elljac = function() return {sf.elljac(1,1)} end

t.erf = function() return sf.erf(1) end
t.erf_Q = function() return sf.erf_Q(1) end
t.erf_Z = function() return sf.erf_Z(1) end
t.erfc = function() return sf.erfc(1) end
t.log_erfc = function() return sf.log_erfc(1) end

t.eta = function() return sf.eta(1) end

t.exp = function() return sf.exp(1) end
t.exp_err = function() return sf.exp_err(1, 1) end
t.exp_mult = function() return sf.exp_mult(1,1) end
t.exp_mult_err = function() return sf.exp_mult_err(1,1,1,1) end

t.expint3 = function() return sf.expint3(1) end
t.expint_Ei = function() return sf.expint_Ei(1) end
t.expint_E = function() return sf.expint_E(1,1) end

t.expm1 = function() return sf.expm1(1) end
t.exprel = function() return sf.exprel(1) end
t.exprel_2 = function() return sf.exprel_2(1) end
t.exprel_n = function() return sf.exprel_n(1,1) end


t.fermi_dirac = function() return sf.fermi_dirac(1,1) end
t.fermi_dirac_inc = function() return sf.fermi_dirac_inc(1,1) end

t.gamma = function() return sf.gamma(1) end
t.gamma_inc = function() return sf.gamma_inc(1,1) end
t.gamma_inc_P = function() return sf.gamma_inc_P(1,1) end
t.gamma_inc_Q = function() return sf.gamma_inc_Q(1,1) end
t.lngammac = function() return {sf.lngammac(1+1i)} end
t.gammainv = function() return sf.gammainv(1) end
t.gammastar = function() return sf.gammastar(1) end

--t.gegenpoly = function() return sf.gegenpoly(1,1,1) end

t.hyperg0F1 = function() return sf.hyperg0F1(1,1) end
t.hyperg1F1 = function() return sf.hyperg1F1(1,1,1) end
--t.hyperg2F0 = function() return sf.hyperg2F0(1,1,1) end
--t.hyperg2F1 = function() return sf.hyperg2F1(1,1,1,1) end
--t.hyperg2F1_renorm = function() return sf.hyperg2F1_renorm(1,1,1,1) end
--t.hyperg2F1conj = function() return sf.hyperg2F1conj(1+1i, 1,1) end
--t.hyperg2F1conj_renorm = function() return sf.hyperg2F1conj_renorm(1+1i, 1,1) end
t.hypergU = function() return sf.hypergU(1,1,1) end


t.laguerre = function() return sf.laguerre(1,1,1) end

t.lambertW0 = function() return sf.lambertW0(1) end
t.lambertWm1 = function() return sf.lambertWm1(1) end

t.legendreH3d = function() return sf.legendreH3d(1,1,1) end
t.legendreP = function() return sf.legendreP(1,1) end
t.legendrePlm = function() return sf.legendrePlm(1,1,1) end
--t.legendreQ = function() return sf.legendreQ(1,1) end
t.legendresphPlm = function() return sf.legendresphPlm(1,1,1) end

t.lnbeta = function() return sf.lnbeta(1,1) end
t.lnchoose = function() return sf.lnchoose(1,1) end
t.lndoublefact = function() return sf.lndoublefact(1) end
t.lnfact = function() return sf.lnfact(1) end
t.lngamma = function() return sf.lngamma(1) end
t.lnpoch = function() return sf.lnpoch(1,1) end

t.log = function() return sf.log(1) end
t.log_1plusx = function() return sf.log_1plusx(1) end
t.log_1plusx_mx = function() return sf.log_1plusx_mx(1) end
t.log_abs = function() return sf.log_abs(1) end

t.poch = function() return sf.poch(1,1) end
t.pochrel = function() return sf.pochrel(1,1) end

t.pow_int = function() return sf.pow_int(1,1) end

t.psi = function() return sf.psi(1) end
t.psi_1 = function() return sf.psi_1(1) end
t.psi_1piy = function() return sf.psi_1piy(1) end
t.psi_n = function() return sf.psi_n(1,1) end

t.synchrotron1 = function() return sf.synchrotron1(1) end
t.synchrotron2 = function() return sf.synchrotron2(1) end

t.taylorcoeff = function() return sf.taylorcoeff(1,1) end

t.transport = function() return sf.transport(2,1) end

t.zeta = function() return sf.zeta(0.9) end
t.zetam1 = function() return sf.zetam1(0.9) end
t.hzeta = function() return sf.hzeta(2,0.9) end
return t
