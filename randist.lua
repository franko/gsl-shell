
local gsl = require 'gsl'

local M = {
   bernoulli_pdf = gsl.gsl_ran_bernoulli_pdf,
   beta_pdf = gsl.gsl_ran_beta_pdf,
   binomial_pdf = gsl.gsl_ran_binomial_pdf,
   exponential_pdf = gsl.gsl_ran_exponential_pdf,
   exppow_pdf = gsl.gsl_ran_exppow_pdf,
   cauchy_pdf = gsl.gsl_ran_cauchy_pdf,
   chisq_pdf = gsl.gsl_ran_chisq_pdf,
   erlang_pdf = gsl.gsl_ran_erlang_pdf,
   fdist_pdf = gsl.gsl_ran_fdist_pdf,
   flat_pdf = gsl.gsl_ran_flat_pdf,
   gamma_pdf = gsl.gsl_ran_gamma_pdf,
   gaussian_pdf = gsl.gsl_ran_gaussian_pdf,
   ugaussian_pdf = gsl.gsl_ran_ugaussian_pdf,
   gaussian_tail_pdf = gsl.gsl_ran_gaussian_tail_pdf,
   ugaussian_tail_pdf = gsl.gsl_ran_ugaussian_tail_pdf,
   bivariate_gaussian_pdf = gsl.gsl_ran_bivariate_gaussian_pdf,
   landau_pdf = gsl.gsl_ran_landau_pdf,
   geometric_pdf = gsl.gsl_ran_geometric_pdf,
   hypergeometric_pdf = gsl.gsl_ran_hypergeometric_pdf,
   gumbel1_pdf = gsl.gsl_ran_gumbel1_pdf,
   gumbel2_pdf = gsl.gsl_ran_gumbel2_pdf,
   logistic_pdf = gsl.gsl_ran_logistic_pdf,
   lognormal_pdf = gsl.gsl_ran_lognormal_pdf,
   logarithmic_pdf = gsl.gsl_ran_logarithmic_pdf,
   pascal_pdf = gsl.gsl_ran_pascal_pdf,
   pareto_pdf = gsl.gsl_ran_pareto_pdf,
   poisson_pdf = gsl.gsl_ran_poisson_pdf,
   rayleigh_pdf = gsl.gsl_ran_rayleigh_pdf,
   rayleigh_tail_pdf = gsl.gsl_ran_rayleigh_tail_pdf,
   tdist_pdf = gsl.gsl_ran_tdist_pdf,
   laplace_pdf = gsl.gsl_ran_laplace_pdf,
   weibull_pdf = gsl.gsl_ran_weibull_pdf,

   ugaussian_P = gsl.gsl_cdf_ugaussian_P,
   ugaussian_Q = gsl.gsl_cdf_ugaussian_Q,

   ugaussian_Pinv = gsl.gsl_cdf_ugaussian_Pinv,
   ugaussian_Qinv = gsl.gsl_cdf_ugaussian_Qinv,

   gaussian_P = gsl.gsl_cdf_gaussian_P,
   gaussian_Q = gsl.gsl_cdf_gaussian_Q,

   gaussian_Pinv = gsl.gsl_cdf_gaussian_Pinv,
   gaussian_Qinv = gsl.gsl_cdf_gaussian_Qinv,

   gamma_P = gsl.gsl_cdf_gamma_P,
   gamma_Q = gsl.gsl_cdf_gamma_Q,

   gamma_Pinv = gsl.gsl_cdf_gamma_Pinv,
   gamma_Qinv = gsl.gsl_cdf_gamma_Qinv,

   cauchy_P = gsl.gsl_cdf_cauchy_P,
   cauchy_Q = gsl.gsl_cdf_cauchy_Q,

   cauchy_Pinv = gsl.gsl_cdf_cauchy_Pinv,
   cauchy_Qinv = gsl.gsl_cdf_cauchy_Qinv,

   laplace_P = gsl.gsl_cdf_laplace_P,
   laplace_Q = gsl.gsl_cdf_laplace_Q,

   laplace_Pinv = gsl.gsl_cdf_laplace_Pinv,
   laplace_Qinv = gsl.gsl_cdf_laplace_Qinv,

   rayleigh_P = gsl.gsl_cdf_rayleigh_P,
   rayleigh_Q = gsl.gsl_cdf_rayleigh_Q,

   rayleigh_Pinv = gsl.gsl_cdf_rayleigh_Pinv,
   rayleigh_Qinv = gsl.gsl_cdf_rayleigh_Qinv,

   chisq_P = gsl.gsl_cdf_chisq_P,
   chisq_Q = gsl.gsl_cdf_chisq_Q,

   chisq_Pinv = gsl.gsl_cdf_chisq_Pinv,
   chisq_Qinv = gsl.gsl_cdf_chisq_Qinv,

   exponential_P = gsl.gsl_cdf_exponential_P,
   exponential_Q = gsl.gsl_cdf_exponential_Q,

   exponential_Pinv = gsl.gsl_cdf_exponential_Pinv,
   exponential_Qinv = gsl.gsl_cdf_exponential_Qinv,

   exppow_P = gsl.gsl_cdf_exppow_P,
   exppow_Q = gsl.gsl_cdf_exppow_Q,

   tdist_P = gsl.gsl_cdf_tdist_P,
   tdist_Q = gsl.gsl_cdf_tdist_Q,

   tdist_Pinv = gsl.gsl_cdf_tdist_Pinv,
   tdist_Qinv = gsl.gsl_cdf_tdist_Qinv,

   fdist_P = gsl.gsl_cdf_fdist_P,
   fdist_Q = gsl.gsl_cdf_fdist_Q,

   fdist_Pinv = gsl.gsl_cdf_fdist_Pinv,
   fdist_Qinv = gsl.gsl_cdf_fdist_Qinv,

   beta_P = gsl.gsl_cdf_beta_P,
   beta_Q = gsl.gsl_cdf_beta_Q,

   beta_Pinv = gsl.gsl_cdf_beta_Pinv,
   beta_Qinv = gsl.gsl_cdf_beta_Qinv,

   flat_P = gsl.gsl_cdf_flat_P,
   flat_Q = gsl.gsl_cdf_flat_Q,

   flat_Pinv = gsl.gsl_cdf_flat_Pinv,
   flat_Qinv = gsl.gsl_cdf_flat_Qinv,

   lognormal_P = gsl.gsl_cdf_lognormal_P,
   lognormal_Q = gsl.gsl_cdf_lognormal_Q,

   lognormal_Pinv = gsl.gsl_cdf_lognormal_Pinv,
   lognormal_Qinv = gsl.gsl_cdf_lognormal_Qinv,

   gumbel1_P = gsl.gsl_cdf_gumbel1_P,
   gumbel1_Q = gsl.gsl_cdf_gumbel1_Q,

   gumbel1_Pinv = gsl.gsl_cdf_gumbel1_Pinv,
   gumbel1_Qinv = gsl.gsl_cdf_gumbel1_Qinv,

   gumbel2_P = gsl.gsl_cdf_gumbel2_P,
   gumbel2_Q = gsl.gsl_cdf_gumbel2_Q,

   gumbel2_Pinv = gsl.gsl_cdf_gumbel2_Pinv,
   gumbel2_Qinv = gsl.gsl_cdf_gumbel2_Qinv,

   weibull_P = gsl.gsl_cdf_weibull_P,
   weibull_Q = gsl.gsl_cdf_weibull_Q,

   weibull_Pinv = gsl.gsl_cdf_weibull_Pinv,
   weibull_Qinv = gsl.gsl_cdf_weibull_Qinv,

   pareto_P = gsl.gsl_cdf_pareto_P,
   pareto_Q = gsl.gsl_cdf_pareto_Q,

   pareto_Pinv = gsl.gsl_cdf_pareto_Pinv,
   pareto_Qinv = gsl.gsl_cdf_pareto_Qinv,

   logistic_P = gsl.gsl_cdf_logistic_P,
   logistic_Q = gsl.gsl_cdf_logistic_Q,

   logistic_Pinv = gsl.gsl_cdf_logistic_Pinv,
   logistic_Qinv = gsl.gsl_cdf_logistic_Qinv,

   binomial_P = gsl.gsl_cdf_binomial_P,
   binomial_Q = gsl.gsl_cdf_binomial_Q,

   poisson_P = gsl.gsl_cdf_poisson_P,
   poisson_Q = gsl.gsl_cdf_poisson_Q,

   geometric_P = gsl.gsl_cdf_geometric_P,
   geometric_Q = gsl.gsl_cdf_geometric_Q,

   negative_binomial_P = gsl.gsl_cdf_negative_binomial_P,
   negative_binomial_Q = gsl.gsl_cdf_negative_binomial_Q,

   pascal_P = gsl.gsl_cdf_pascal_P,
   pascal_Q = gsl.gsl_cdf_pascal_Q,

   hypergeometric_P = gsl.gsl_cdf_hypergeometric_P,
   hypergeometric_Q = gsl.gsl_cdf_hypergeometric_Q,
}

randist = M

return M
