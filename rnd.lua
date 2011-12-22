
local gsl = require 'gsl'
local ran = {}

ran.bernoulli = gsl.gsl_ran_bernoulli
ran.beta = gsl.gsl_ran_beta
ran.binomial = gsl.gsl_ran_binomial
ran.binomial_knuth = gsl.gsl_ran_binomial_knuth
ran.binomial_tpe = gsl.gsl_ran_binomial_tpe
ran.exponential = gsl.gsl_ran_exponential
ran.exppow = gsl.gsl_ran_exppow
ran.cauchy = gsl.gsl_ran_cauchy
ran.chisq = gsl.gsl_ran_chisq
ran.erlang = gsl.gsl_ran_erlang
ran.fdist = gsl.gsl_ran_fdist
ran.flat = gsl.gsl_ran_flat
ran.gamma = gsl.gsl_ran_gamma
ran.gamma_int = gsl.gsl_ran_gamma_int
ran.gamma_mt = gsl.gsl_ran_gamma_mt
ran.gamma_knuth = gsl.gsl_ran_gamma_knuth
ran.gaussian = gsl.gsl_ran_gaussian
ran.gaussian_ratio_method = gsl.gsl_ran_gaussian_ratio_method
ran.gaussian_ziggurat = gsl.gsl_ran_gaussian_ziggurat
ran.ugaussian = gsl.gsl_ran_ugaussian
ran.ugaussian_ratio_method = gsl.gsl_ran_ugaussian_ratio_method
ran.gaussian_tail = gsl.gsl_ran_gaussian_tail
ran.ugaussian_tail = gsl.gsl_ran_ugaussian_tail
ran.bivariate_gaussian = gsl.gsl_ran_bivariate_gaussian
ran.landau = gsl.gsl_ran_landau
ran.geometric = gsl.gsl_ran_geometric
ran.hypergeometric = gsl.gsl_ran_hypergeometric
ran.gumbel1 = gsl.gsl_ran_gumbel1
ran.gumbel2 = gsl.gsl_ran_gumbel2
ran.logistic = gsl.gsl_ran_logistic
ran.lognormal = gsl.gsl_ran_lognormal
ran.logarithmic = gsl.gsl_ran_logarithmic
ran.pascal = gsl.gsl_ran_pascal
ran.pareto = gsl.gsl_ran_pareto
ran.poisson = gsl.gsl_ran_poisson
ran.rayleigh = gsl.gsl_ran_rayleigh
ran.rayleigh_tail = gsl.gsl_ran_rayleigh_tail
ran.tdist = gsl.gsl_ran_tdist
ran.laplace = gsl.gsl_ran_laplace
ran.levy = gsl.gsl_ran_levy
ran.levy_skew = gsl.gsl_ran_levy_skew
ran.weibull = gsl.gsl_ran_weibull

-- set global variable
rnd = ran

return ran
