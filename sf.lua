--Spatial function sf.module of gsl-shell

local ffi = require 'ffi'
local gsl = require 'gsl'

local gsl_check = require 'gsl-check'
local check     = require 'check'
local is_integer = check.is_integer

--Definition of result file
ffi.cdef[[
struct gsl_sf_result_struct {
  double val;
  double err;
};
typedef struct gsl_sf_result_struct gsl_sf_result;


struct gsl_sf_result_e10_struct {
  double val;
  double err;
  int    e10;
};
typedef struct gsl_sf_result_e10_struct gsl_sf_result_e10;

typedef int gsl_mode_t;


int gsl_sf_result_smash_e(const gsl_sf_result_e10 * re, gsl_sf_result * r);
]]

--Definition of fermi_dirac file
ffi.cdef[[
int     gsl_sf_fermi_dirac_m1_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_m1(const double x);
int     gsl_sf_fermi_dirac_0_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_0(const double x);
int     gsl_sf_fermi_dirac_1_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_1(const double x);
int     gsl_sf_fermi_dirac_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_2(const double x);
int     gsl_sf_fermi_dirac_int_e(const int j, const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_int(const int j, const double x);
int     gsl_sf_fermi_dirac_mhalf_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_mhalf(const double x);
int     gsl_sf_fermi_dirac_half_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_half(const double x);
int     gsl_sf_fermi_dirac_3half_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_3half(const double x);
int     gsl_sf_fermi_dirac_inc_0_e(const double x, const double b, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_inc_0(const double x, const double b);
]]

--Definition of elementary file
ffi.cdef[[
int gsl_sf_multiply_e(const double x, const double y, gsl_sf_result * result);
double gsl_sf_multiply(const double x, const double y);
int gsl_sf_multiply_err_e(const double x, const double dx, const double y, const double dy, gsl_sf_result * result);
]]

--Definition of gegenbauer file
ffi.cdef[[
int gsl_sf_gegenpoly_1_e(double lambda, double x, gsl_sf_result * result);
int gsl_sf_gegenpoly_2_e(double lambda, double x, gsl_sf_result * result);
int gsl_sf_gegenpoly_3_e(double lambda, double x, gsl_sf_result * result);
double gsl_sf_gegenpoly_1(double lambda, double x);
double gsl_sf_gegenpoly_2(double lambda, double x);
double gsl_sf_gegenpoly_3(double lambda, double x);
int gsl_sf_gegenpoly_n_e(int n, double lambda, double x, gsl_sf_result * result);
double gsl_sf_gegenpoly_n(int n, double lambda, double x);
int gsl_sf_gegenpoly_array(int nmax, double lambda, double x, double * result_array);
]]

--Definition of dawson file
ffi.cdef[[
int     gsl_sf_dawson_e(double x, gsl_sf_result * result);
double     gsl_sf_dawson(double x);
]]

--Definition of expint file
ffi.cdef[[
int     gsl_sf_expint_E1_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_E1(const double x);
int     gsl_sf_expint_E2_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_E2(const double x);
int     gsl_sf_expint_En_e(const int n, const double x, gsl_sf_result * result);
double  gsl_sf_expint_En(const int n, const double x);
int     gsl_sf_expint_E1_scaled_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_E1_scaled(const double x);
int     gsl_sf_expint_E2_scaled_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_E2_scaled(const double x);
int     gsl_sf_expint_En_scaled_e(const int n, const double x, gsl_sf_result * result);
double  gsl_sf_expint_En_scaled(const int n, const double x);
int     gsl_sf_expint_Ei_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_Ei(const double x);
int     gsl_sf_expint_Ei_scaled_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_Ei_scaled(const double x);
int     gsl_sf_Shi_e(const double x, gsl_sf_result * result);
double  gsl_sf_Shi(const double x);
int     gsl_sf_Chi_e(const double x, gsl_sf_result * result);
double  gsl_sf_Chi(const double x);
int     gsl_sf_expint_3_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_3(double x);
int     gsl_sf_Si_e(const double x, gsl_sf_result * result);
double  gsl_sf_Si(const double x);
int     gsl_sf_Ci_e(const double x, gsl_sf_result * result);
double  gsl_sf_Ci(const double x);
int     gsl_sf_atanint_e(const double x, gsl_sf_result * result);
double  gsl_sf_atanint(const double x);
]]

--Definition of dilog file
ffi.cdef[[
int     gsl_sf_dilog_e(const double x, gsl_sf_result * result);
double  gsl_sf_dilog(const double x);
int gsl_sf_complex_dilog_xy_e(
  const double x,
  const double y,
  gsl_sf_result * result_re,
  gsl_sf_result * result_im
  );

int	gsl_sf_complex_dilog_e(
  const double r,
  const double theta,
  gsl_sf_result * result_re,
  gsl_sf_result * result_im
  );

int	gsl_sf_complex_spence_xy_e(
  const double x,
  const double y,
  gsl_sf_result * real_sp,
  gsl_sf_result * imag_sp
  );
]]

--Definition of pow_int file
ffi.cdef[[
int     gsl_sf_pow_int_e(double x, int n, gsl_sf_result * result);
double  gsl_sf_pow_int(const double x, const int n);
]]

--Definition of coulomb file
ffi.cdef[[
int gsl_sf_hydrogenicR_1_e(const double Z, const double r, gsl_sf_result * result);
double gsl_sf_hydrogenicR_1(const double Z, const double r);
int gsl_sf_hydrogenicR_e(const int n, const int l, const double Z, const double r, gsl_sf_result * result);
double gsl_sf_hydrogenicR(const int n, const int l, const double Z, const double r);
int gsl_sf_coulomb_wave_FG_e(const double eta, const double x,
                    const double lam_F,
                    const int  k_lam_G,
                    gsl_sf_result * F, gsl_sf_result * Fp,
                    gsl_sf_result * G, gsl_sf_result * Gp,
                    double * exp_F, double * exp_G);
int gsl_sf_coulomb_wave_F_array(
  double lam_min, int kmax,
  double eta, double x,
  double * fc_array,
  double * F_exponent
  );
int gsl_sf_coulomb_wave_FG_array(double lam_min, int kmax,
	                        double eta, double x,
	                        double * fc_array, double * gc_array,
	                        double * F_exponent,
	                        double * G_exponent
	                        );
int gsl_sf_coulomb_wave_FGp_array(double lam_min, int kmax,
	                        double eta, double x,
	                        double * fc_array, double * fcp_array,
	                        double * gc_array, double * gcp_array,
	                        double * F_exponent,
	                        double * G_exponent
	                        );
int gsl_sf_coulomb_wave_sphF_array(double lam_min, int kmax,
	                                double eta, double x,
	                                double * fc_array,
	                                double * F_exponent
	                                );
int gsl_sf_coulomb_CL_e(double L, double eta, gsl_sf_result * result);
int gsl_sf_coulomb_CL_array(double Lmin, int kmax, double eta, double * cl);
]]

--Definition of bessel file
ffi.cdef[[
int gsl_sf_bessel_J0_e(const double x,  gsl_sf_result * result);
double gsl_sf_bessel_J0(const double x);
int gsl_sf_bessel_J1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_J1(const double x);
int gsl_sf_bessel_Jn_e(int n, double x, gsl_sf_result * result);
double gsl_sf_bessel_Jn(const int n, const double x);
int gsl_sf_bessel_Jn_array(int nmin, int nmax, double x, double * result_array);
int gsl_sf_bessel_Y0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_Y0(const double x);
int gsl_sf_bessel_Y1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_Y1(const double x);
int gsl_sf_bessel_Yn_e(int n,const double x, gsl_sf_result * result);
double gsl_sf_bessel_Yn(const int n,const double x);
int gsl_sf_bessel_Yn_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_I0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_I0(const double x);
int gsl_sf_bessel_I1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_I1(const double x);
int gsl_sf_bessel_In_e(const int n, const double x, gsl_sf_result * result);
double gsl_sf_bessel_In(const int n, const double x);
int gsl_sf_bessel_In_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_I0_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_I0_scaled(const double x);
int gsl_sf_bessel_I1_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_I1_scaled(const double x);
int gsl_sf_bessel_In_scaled_e(int n, const double x, gsl_sf_result * result);
double gsl_sf_bessel_In_scaled(const int n, const double x);
int gsl_sf_bessel_In_scaled_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_K0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_K0(const double x);
int gsl_sf_bessel_K1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_K1(const double x);
int gsl_sf_bessel_Kn_e(const int n, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Kn(const int n, const double x);
int gsl_sf_bessel_Kn_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_K0_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_K0_scaled(const double x);
int gsl_sf_bessel_K1_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_K1_scaled(const double x);
int gsl_sf_bessel_Kn_scaled_e(int n, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Kn_scaled(const int n, const double x);
int gsl_sf_bessel_Kn_scaled_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_j0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_j0(const double x);
int gsl_sf_bessel_j1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_j1(const double x);
int gsl_sf_bessel_j2_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_j2(const double x);
int gsl_sf_bessel_jl_e(const int l, const double x, gsl_sf_result * result);
double gsl_sf_bessel_jl(const int l, const double x);
int gsl_sf_bessel_jl_array(const int lmax, const double x, double * result_array);
int gsl_sf_bessel_jl_steed_array(const int lmax, const double x, double * jl_x_array);
int gsl_sf_bessel_y0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_y0(const double x);
int gsl_sf_bessel_y1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_y1(const double x);
int gsl_sf_bessel_y2_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_y2(const double x);
int gsl_sf_bessel_yl_e(int l, const double x, gsl_sf_result * result);
double gsl_sf_bessel_yl(const int l, const double x);
int gsl_sf_bessel_yl_array(const int lmax, const double x, double * result_array);
int gsl_sf_bessel_i0_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_i0_scaled(const double x);
int gsl_sf_bessel_i1_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_i1_scaled(const double x);
int gsl_sf_bessel_i2_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_i2_scaled(const double x);
int gsl_sf_bessel_il_scaled_e(const int l, double x, gsl_sf_result * result);
double gsl_sf_bessel_il_scaled(const int l, const double x);
int gsl_sf_bessel_il_scaled_array(const int lmax, const double x, double * result_array);
int gsl_sf_bessel_k0_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_k0_scaled(const double x);
int gsl_sf_bessel_k1_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_k1_scaled(const double x);
int gsl_sf_bessel_k2_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_k2_scaled(const double x);
int gsl_sf_bessel_kl_scaled_e(int l, const double x, gsl_sf_result * result);
double gsl_sf_bessel_kl_scaled(const int l, const double x);
int gsl_sf_bessel_kl_scaled_array(const int lmax, const double x, double * result_array);
int gsl_sf_bessel_Jnu_e(const double nu, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Jnu(const double nu, const double x);
int gsl_sf_bessel_Ynu_e(double nu, double x, gsl_sf_result * result);
double gsl_sf_bessel_Ynu(const double nu, const double x);
int gsl_sf_bessel_sequence_Jnu_e(double nu, gsl_mode_t mode, size_t size, double * v);
int gsl_sf_bessel_Inu_scaled_e(double nu, double x, gsl_sf_result * result);
double gsl_sf_bessel_Inu_scaled(double nu, double x);
int gsl_sf_bessel_Inu_e(double nu, double x, gsl_sf_result * result);
double gsl_sf_bessel_Inu(double nu, double x);
int gsl_sf_bessel_Knu_scaled_e(const double nu, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Knu_scaled(const double nu, const double x);
int gsl_sf_bessel_Knu_scaled_e10_e(const double nu, const double x, gsl_sf_result_e10 * result);
int gsl_sf_bessel_Knu_e(const double nu, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Knu(const double nu, const double x);
int gsl_sf_bessel_lnKnu_e(const double nu, const double x, gsl_sf_result * result);
double gsl_sf_bessel_lnKnu(const double nu, const double x);
int gsl_sf_bessel_zero_J0_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_bessel_zero_J0(unsigned int s);
int gsl_sf_bessel_zero_J1_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_bessel_zero_J1(unsigned int s);
int gsl_sf_bessel_zero_Jnu_e(double nu, unsigned int s, gsl_sf_result * result);
double gsl_sf_bessel_zero_Jnu(double nu, unsigned int s);
]]

--Definition of laguerre file
ffi.cdef[[
int gsl_sf_laguerre_1_e(const double a, const double x, gsl_sf_result * result);
int gsl_sf_laguerre_2_e(const double a, const double x, gsl_sf_result * result);
int gsl_sf_laguerre_3_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_laguerre_1(double a, double x);
double gsl_sf_laguerre_2(double a, double x);
double gsl_sf_laguerre_3(double a, double x);
int     gsl_sf_laguerre_n_e(const int n, const double a, const double x, gsl_sf_result * result);
double     gsl_sf_laguerre_n(int n, double a, double x);
]]

--Definition of airy file
ffi.cdef[[
int gsl_sf_airy_Ai_e(const double x, const gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Ai(const double x, gsl_mode_t mode);
int gsl_sf_airy_Bi_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Bi(const double x, gsl_mode_t mode);
int gsl_sf_airy_Ai_scaled_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Ai_scaled(const double x, gsl_mode_t mode);
int gsl_sf_airy_Bi_scaled_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Bi_scaled(const double x, gsl_mode_t mode);
int gsl_sf_airy_Ai_deriv_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Ai_deriv(const double x, gsl_mode_t mode);
int gsl_sf_airy_Bi_deriv_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Bi_deriv(const double x, gsl_mode_t mode);
int gsl_sf_airy_Ai_deriv_scaled_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Ai_deriv_scaled(const double x, gsl_mode_t mode);
int gsl_sf_airy_Bi_deriv_scaled_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Bi_deriv_scaled(const double x, gsl_mode_t mode);
int gsl_sf_airy_zero_Ai_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_airy_zero_Ai(unsigned int s);
int gsl_sf_airy_zero_Bi_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_airy_zero_Bi(unsigned int s);
int gsl_sf_airy_zero_Ai_deriv_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_airy_zero_Ai_deriv(unsigned int s);
int gsl_sf_airy_zero_Bi_deriv_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_airy_zero_Bi_deriv(unsigned int s);
]]

--Definition of lambert file
ffi.cdef[[
int     gsl_sf_lambert_W0_e(double x, gsl_sf_result * result);
double  gsl_sf_lambert_W0(double x);
int     gsl_sf_lambert_Wm1_e(double x, gsl_sf_result * result);
double  gsl_sf_lambert_Wm1(double x);
]]

--Definition of exp file
ffi.cdef[[
int gsl_sf_exp_e(const double x, gsl_sf_result * result);
double gsl_sf_exp(const double x);
int gsl_sf_exp_e10_e(const double x, gsl_sf_result_e10 * result);
int gsl_sf_exp_mult_e(const double x, const double y, gsl_sf_result * result);
double gsl_sf_exp_mult(const double x, const double y);
int gsl_sf_exp_mult_e10_e(const double x, const double y, gsl_sf_result_e10 * result);
int gsl_sf_expm1_e(const double x, gsl_sf_result * result);
double gsl_sf_expm1(const double x);
int gsl_sf_exprel_e(const double x, gsl_sf_result * result);
double gsl_sf_exprel(const double x);
int gsl_sf_exprel_2_e(double x, gsl_sf_result * result);
double gsl_sf_exprel_2(const double x);
int gsl_sf_exprel_n_e(const int n, const double x, gsl_sf_result * result);
double gsl_sf_exprel_n(const int n, const double x);
int gsl_sf_exprel_n_CF_e(const double n, const double x, gsl_sf_result * result);
int gsl_sf_exp_err_e(const double x, const double dx, gsl_sf_result * result);
int gsl_sf_exp_err_e10_e(const double x, const double dx, gsl_sf_result_e10 * result);
int gsl_sf_exp_mult_err_e(const double x, const double dx, const double y, const double dy, gsl_sf_result * result);
int gsl_sf_exp_mult_err_e10_e(const double x, const double dx, const double y, const double dy, gsl_sf_result_e10 * result);
]]

--Definition of psi file
ffi.cdef[[
int     gsl_sf_psi_int_e(const int n, gsl_sf_result * result);
double  gsl_sf_psi_int(const int n);
int     gsl_sf_psi_e(const double x, gsl_sf_result * result);
double  gsl_sf_psi(const double x);
int     gsl_sf_psi_1piy_e(const double y, gsl_sf_result * result);
double  gsl_sf_psi_1piy(const double y);
int gsl_sf_complex_psi_e(
  const double x,
  const double y,
  gsl_sf_result * result_re,
  gsl_sf_result * result_im
  );
int     gsl_sf_psi_1_int_e(const int n, gsl_sf_result * result);
double  gsl_sf_psi_1_int(const int n);
int     gsl_sf_psi_1_e(const double x, gsl_sf_result * result);
double  gsl_sf_psi_1(const double x);
int     gsl_sf_psi_n_e(const int n, const double x, gsl_sf_result * result);
double  gsl_sf_psi_n(const int n, const double x);
]]

--Definition of clausen file
ffi.cdef[[
int gsl_sf_clausen_e(double x, gsl_sf_result * result);
double gsl_sf_clausen(const double x);
]]

--Definition of erf file
ffi.cdef[[
int gsl_sf_erfc_e(double x, gsl_sf_result * result);
double gsl_sf_erfc(double x);
int gsl_sf_log_erfc_e(double x, gsl_sf_result * result);
double gsl_sf_log_erfc(double x);
int gsl_sf_erf_e(double x, gsl_sf_result * result);
double gsl_sf_erf(double x);
int gsl_sf_erf_Z_e(double x, gsl_sf_result * result);
int gsl_sf_erf_Q_e(double x, gsl_sf_result * result);
double gsl_sf_erf_Z(double x);
double gsl_sf_erf_Q(double x);
int gsl_sf_hazard_e(double x, gsl_sf_result * result);
double gsl_sf_hazard(double x);
]]

--Definition of coupling file
ffi.cdef[[
int gsl_sf_coupling_3j_e(int two_ja, int two_jb, int two_jc,
	                    int two_ma, int two_mb, int two_mc,
	                    gsl_sf_result * result
	                    );
double gsl_sf_coupling_3j(int two_ja, int two_jb, int two_jc,
	                  int two_ma, int two_mb, int two_mc
	                  );
int gsl_sf_coupling_6j_e(int two_ja, int two_jb, int two_jc,
	                 int two_jd, int two_je, int two_jf,
	                 gsl_sf_result * result
	                 );
double gsl_sf_coupling_6j(int two_ja, int two_jb, int two_jc,
	                  int two_jd, int two_je, int two_jf
	                  );
int gsl_sf_coupling_RacahW_e(int two_ja, int two_jb, int two_jc,
	                     int two_jd, int two_je, int two_jf,
	                     gsl_sf_result * result
	                     );
double gsl_sf_coupling_RacahW(int two_ja, int two_jb, int two_jc,
	                      int two_jd, int two_je, int two_jf
	                      );
int gsl_sf_coupling_9j_e(int two_ja, int two_jb, int two_jc,
	                 int two_jd, int two_je, int two_jf,
	                 int two_jg, int two_jh, int two_ji,
	                 gsl_sf_result * result
	                 );
double gsl_sf_coupling_9j(int two_ja, int two_jb, int two_jc,
	                  int two_jd, int two_je, int two_jf,
	                  int two_jg, int two_jh, int two_ji
	                  );
int gsl_sf_coupling_6j_INCORRECT_e(int two_ja, int two_jb, int two_jc,
	                           int two_jd, int two_je, int two_jf,
	                           gsl_sf_result * result
	                           );
double gsl_sf_coupling_6j_INCORRECT(int two_ja, int two_jb, int two_jc,
	                            int two_jd, int two_je, int two_jf
	                            );
]]

--Definition of ma file
ffi.cdef[[
typedef struct {
  size_t size;
  double * d;
  double * sd;
} gsl_eigen_symm_workspace;

typedef struct {
  size_t size;
  double * d;
  double * sd;
  double * gc;
  double * gs;
} gsl_eigen_symmv_workspace;

typedef struct
{
  size_t size;
  size_t even_order;
  size_t odd_order;
  int extra_values;
  double qa;   /* allow for caching of results: not implemented yet */
  double qb;   /* allow for caching of results: not implemented yet */
  double *aa;
  double *bb;
  double *dd;
  double *ee;
  double *tt;
  double *e2;
  double *zz;
  gsl_vector *eval;
  gsl_matrix *evec;
  gsl_eigen_symmv_workspace *wmat;
} gsl_sf_mathieu_workspace;

int gsl_sf_mathieu_a_array(int order_min, int order_max, double qq, gsl_sf_mathieu_workspace *work, double result_array[]);
int gsl_sf_mathieu_b_array(int order_min, int order_max, double qq,  gsl_sf_mathieu_workspace *work, double result_array[]);
int gsl_sf_mathieu_a(int order, double qq, gsl_sf_result *result);
int gsl_sf_mathieu_b(int order, double qq, gsl_sf_result *result);
int gsl_sf_mathieu_a_coeff(int order, double qq, double aa, double coeff[]);
int gsl_sf_mathieu_b_coeff(int order, double qq, double aa, double coeff[]);
void gsl_sf_mathieu_free(gsl_sf_mathieu_workspace *workspace);
int gsl_sf_mathieu_ce(int order, double qq, double zz, gsl_sf_result *result);
int gsl_sf_mathieu_se(int order, double qq, double zz, gsl_sf_result *result);
int gsl_sf_mathieu_ce_array(int nmin, int nmax, double qq, double zz,
	                    gsl_sf_mathieu_workspace *work,
	                    double result_array[]);
int gsl_sf_mathieu_se_array(int nmin, int nmax, double qq, double zz,
	                    gsl_sf_mathieu_workspace *work,
	                    double result_array[]);
int gsl_sf_mathieu_Mc(int kind, int order, double qq, double zz,
	              gsl_sf_result *result);
int gsl_sf_mathieu_Ms(int kind, int order, double qq, double zz,
	              gsl_sf_result *result);
int gsl_sf_mathieu_Mc_array(int kind, int nmin, int nmax, double qq,
	                    double zz, gsl_sf_mathieu_workspace *work,
	                    double result_array[]);
int gsl_sf_mathieu_Ms_array(int kind, int nmin, int nmax, double qq,
	                    double zz, gsl_sf_mathieu_workspace *work,
	                    double result_array[]);
]]


--Definition of syn file
ffi.cdef[[
int     gsl_sf_synchrotron_1_e(const double x, gsl_sf_result * result);
double     gsl_sf_synchrotron_1(const double x);
int     gsl_sf_synchrotron_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_synchrotron_2(const double x);
]]

--Definition of hyperg file
ffi.cdef[[
int gsl_sf_hyperg_0F1_e(double c, double x, gsl_sf_result * result);
double gsl_sf_hyperg_0F1(const double c, const double x);
int gsl_sf_hyperg_1F1_int_e(const int m, const int n, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_1F1_int(const int m, const int n, double x);
int gsl_sf_hyperg_1F1_e(const double a, const double b, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_1F1(double a, double b, double x);
int gsl_sf_hyperg_U_int_e(const int m, const int n, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_U_int(const int m, const int n, const double x);
int gsl_sf_hyperg_U_int_e10_e(const int m, const int n, const double x, gsl_sf_result_e10 * result);
int gsl_sf_hyperg_U_e(const double a, const double b, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_U(const double a, const double b, const double x);
int gsl_sf_hyperg_U_e10_e(const double a, const double b, const double x, gsl_sf_result_e10 * result);
int gsl_sf_hyperg_2F1_e(double a, double b, const double c, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_2F1(double a, double b, double c, double x);
int gsl_sf_hyperg_2F1_conj_e(const double aR, const double aI, const double c, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_2F1_conj(double aR, double aI, double c, double x);
int gsl_sf_hyperg_2F1_renorm_e(const double a, const double b, const double c, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_2F1_renorm(double a, double b, double c, double x);
int gsl_sf_hyperg_2F1_conj_renorm_e(const double aR, const double aI, const double c, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_2F1_conj_renorm(double aR, double aI, double c, double x);
int     gsl_sf_hyperg_2F0_e(const double a, const double b, const double x, gsl_sf_result * result);
double     gsl_sf_hyperg_2F0(const double a, const double b, const double x);
]]

--Definition of debye file
ffi.cdef[[
int     gsl_sf_debye_1_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_1(const double x);
int     gsl_sf_debye_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_2(const double x);
int     gsl_sf_debye_3_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_3(const double x);
int     gsl_sf_debye_4_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_4(const double x);
int     gsl_sf_debye_5_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_5(const double x);
int     gsl_sf_debye_6_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_6(const double x);
]]

--Definition of trig file
ffi.cdef[[
int gsl_sf_sin_e(double x, gsl_sf_result * result);
double gsl_sf_sin(const double x);
int gsl_sf_cos_e(double x, gsl_sf_result * result);
double gsl_sf_cos(const double x);
int gsl_sf_hypot_e(const double x, const double y, gsl_sf_result * result);
double gsl_sf_hypot(const double x, const double y);
int gsl_sf_complex_sin_e(const double zr, const double zi, gsl_sf_result * szr, gsl_sf_result * szi);
int gsl_sf_complex_cos_e(const double zr, const double zi, gsl_sf_result * czr, gsl_sf_result * czi);
int gsl_sf_complex_logsin_e(const double zr, const double zi, gsl_sf_result * lszr, gsl_sf_result * lszi);
int gsl_sf_sinc_e(double x, gsl_sf_result * result);
double gsl_sf_sinc(const double x);
int gsl_sf_lnsinh_e(const double x, gsl_sf_result * result);
double gsl_sf_lnsinh(const double x);
int gsl_sf_lncosh_e(const double x, gsl_sf_result * result);
double gsl_sf_lncosh(const double x);
int gsl_sf_polar_to_rect(const double r, const double theta, gsl_sf_result * x, gsl_sf_result * y);
int gsl_sf_rect_to_polar(const double x, const double y, gsl_sf_result * r, gsl_sf_result * theta);
int gsl_sf_sin_err_e(const double x, const double dx, gsl_sf_result * result);
int gsl_sf_cos_err_e(const double x, const double dx, gsl_sf_result * result);
int gsl_sf_angle_restrict_symm_e(double * theta);
double gsl_sf_angle_restrict_symm(const double theta);
int gsl_sf_angle_restrict_pos_e(double * theta);
double gsl_sf_angle_restrict_pos(const double theta);
int gsl_sf_angle_restrict_symm_err_e(const double theta, gsl_sf_result * result);
int gsl_sf_angle_restrict_pos_err_e(const double theta, gsl_sf_result * result);
]]

--Definition of gamma file
ffi.cdef[[
int gsl_sf_lngamma_e(double x, gsl_sf_result * result);
double gsl_sf_lngamma(const double x);
int gsl_sf_lngamma_sgn_e(double x, gsl_sf_result * result_lg, double *sgn);
int gsl_sf_gamma_e(const double x, gsl_sf_result * result);
double gsl_sf_gamma(const double x);
int gsl_sf_gammastar_e(const double x, gsl_sf_result * result);
double gsl_sf_gammastar(const double x);
int gsl_sf_gammainv_e(const double x, gsl_sf_result * result);
double gsl_sf_gammainv(const double x);
int gsl_sf_lngamma_complex_e(double zr, double zi, gsl_sf_result * lnr, gsl_sf_result * arg);
int gsl_sf_taylorcoeff_e(const int n, const double x, gsl_sf_result * result);
double gsl_sf_taylorcoeff(const int n, const double x);
int gsl_sf_fact_e(const unsigned int n, gsl_sf_result * result);
double gsl_sf_fact(const unsigned int n);
int gsl_sf_doublefact_e(const unsigned int n, gsl_sf_result * result);
double gsl_sf_doublefact(const unsigned int n);
int gsl_sf_lnfact_e(const unsigned int n, gsl_sf_result * result);
double gsl_sf_lnfact(const unsigned int n);
int gsl_sf_lndoublefact_e(const unsigned int n, gsl_sf_result * result);
double gsl_sf_lndoublefact(const unsigned int n);
int gsl_sf_lnchoose_e(unsigned int n, unsigned int m, gsl_sf_result * result);
double gsl_sf_lnchoose(unsigned int n, unsigned int m);
int gsl_sf_choose_e(unsigned int n, unsigned int m, gsl_sf_result * result);
double gsl_sf_choose(unsigned int n, unsigned int m);
int gsl_sf_lnpoch_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_lnpoch(const double a, const double x);
int gsl_sf_lnpoch_sgn_e(const double a, const double x, gsl_sf_result * result, double * sgn);
int gsl_sf_poch_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_poch(const double a, const double x);
int gsl_sf_pochrel_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_pochrel(const double a, const double x);
int gsl_sf_gamma_inc_Q_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_gamma_inc_Q(const double a, const double x);
int gsl_sf_gamma_inc_P_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_gamma_inc_P(const double a, const double x);
int gsl_sf_gamma_inc_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_gamma_inc(const double a, const double x);
int gsl_sf_lnbeta_e(const double a, const double b, gsl_sf_result * result);
double gsl_sf_lnbeta(const double a, const double b);
int gsl_sf_lnbeta_sgn_e(const double x, const double y, gsl_sf_result * result, double * sgn);
int gsl_sf_beta_e(const double a, const double b, gsl_sf_result * result);
double gsl_sf_beta(const double a, const double b);
int gsl_sf_beta_inc_e(const double a, const double b, const double x, gsl_sf_result * result);
double gsl_sf_beta_inc(const double a, const double b, const double x);
]]

--Definition of zeta file
ffi.cdef[[
int gsl_sf_zeta_int_e(const int n, gsl_sf_result * result);
double gsl_sf_zeta_int(const int n);
int gsl_sf_zeta_e(const double s, gsl_sf_result * result);
double gsl_sf_zeta(const double s);
int gsl_sf_zetam1_e(const double s, gsl_sf_result * result);
double gsl_sf_zetam1(const double s);
int gsl_sf_zetam1_int_e(const int s, gsl_sf_result * result);
double gsl_sf_zetam1_int(const int s);
int gsl_sf_hzeta_e(const double s, const double q, gsl_sf_result * result);
double gsl_sf_hzeta(const double s, const double q);
int gsl_sf_eta_int_e(int n, gsl_sf_result * result);
double gsl_sf_eta_int(const int n);
int gsl_sf_eta_e(const double s, gsl_sf_result * result);
double gsl_sf_eta(const double s);
]]

--Definition of transport file
ffi.cdef[[
int     gsl_sf_transport_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_transport_2(const double x);
int     gsl_sf_transport_3_e(const double x, gsl_sf_result * result);
double     gsl_sf_transport_3(const double x);
int     gsl_sf_transport_4_e(const double x, gsl_sf_result * result);
double     gsl_sf_transport_4(const double x);
int     gsl_sf_transport_5_e(const double x, gsl_sf_result * result);
double     gsl_sf_transport_5(const double x);
]]

--Definition of log file
ffi.cdef[[
int gsl_sf_log_e(const double x, gsl_sf_result * result);
double gsl_sf_log(const double x);
int gsl_sf_log_abs_e(const double x, gsl_sf_result * result);
double gsl_sf_log_abs(const double x);
int gsl_sf_complex_log_e(const double zr, const double zi, gsl_sf_result * lnr, gsl_sf_result * theta);
int gsl_sf_log_1plusx_e(const double x, gsl_sf_result * result);
double gsl_sf_log_1plusx(const double x);
int gsl_sf_log_1plusx_mx_e(const double x, gsl_sf_result * result);
double gsl_sf_log_1plusx_mx(const double x);
]]

--Definition of elljac file
ffi.cdef[[
int gsl_sf_elljac_e(double u, double m, double * sn, double * cn, double * dn);
]]

--Definition of ellint file
ffi.cdef[[
int gsl_sf_ellint_Kcomp_e(double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_Kcomp(double k, gsl_mode_t mode);
int gsl_sf_ellint_Ecomp_e(double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_Ecomp(double k, gsl_mode_t mode);
int gsl_sf_ellint_Pcomp_e(double k, double n, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_Pcomp(double k, double n, gsl_mode_t mode);
int gsl_sf_ellint_Dcomp_e(double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_Dcomp(double k, gsl_mode_t mode);
int gsl_sf_ellint_F_e(double phi, double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_F(double phi, double k, gsl_mode_t mode);
int gsl_sf_ellint_E_e(double phi, double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_E(double phi, double k, gsl_mode_t mode);
int gsl_sf_ellint_P_e(double phi, double k, double n, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_P(double phi, double k, double n, gsl_mode_t mode);
int gsl_sf_ellint_D_e(double phi, double k, double n, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_D(double phi, double k, double n, gsl_mode_t mode);
int gsl_sf_ellint_RC_e(double x, double y, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_RC(double x, double y, gsl_mode_t mode);
int gsl_sf_ellint_RD_e(double x, double y, double z, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_RD(double x, double y, double z, gsl_mode_t mode);
int gsl_sf_ellint_RF_e(double x, double y, double z, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_RF(double x, double y, double z, gsl_mode_t mode);
int gsl_sf_ellint_RJ_e(double x, double y, double z, double p, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_RJ(double x, double y, double z, double p, gsl_mode_t mode);
]]

--Definition of legendre file
ffi.cdef[[
int     gsl_sf_legendre_Pl_e(const int l, const double x, gsl_sf_result * result);
double  gsl_sf_legendre_Pl(const int l, const double x);
int gsl_sf_legendre_Pl_array(
  const int lmax, const double x,
  double * result_array
  );
int gsl_sf_legendre_Pl_deriv_array(
  const int lmax, const double x,
  double * result_array,
  double * result_deriv_array
  );
int gsl_sf_legendre_P1_e(double x, gsl_sf_result * result);
int gsl_sf_legendre_P2_e(double x, gsl_sf_result * result);
int gsl_sf_legendre_P3_e(double x, gsl_sf_result * result);
double gsl_sf_legendre_P1(const double x);
double gsl_sf_legendre_P2(const double x);
double gsl_sf_legendre_P3(const double x);
int gsl_sf_legendre_Q0_e(const double x, gsl_sf_result * result);
double gsl_sf_legendre_Q0(const double x);
int gsl_sf_legendre_Q1_e(const double x, gsl_sf_result * result);
double gsl_sf_legendre_Q1(const double x);
int gsl_sf_legendre_Ql_e(const int l, const double x, gsl_sf_result * result);
double gsl_sf_legendre_Ql(const int l, const double x);
int     gsl_sf_legendre_Plm_e(const int l, const int m, const double x, gsl_sf_result * result);
double  gsl_sf_legendre_Plm(const int l, const int m, const double x);
int gsl_sf_legendre_Plm_array(
  const int lmax, const int m, const double x,
  double * result_array
  );
int gsl_sf_legendre_Plm_deriv_array(
  const int lmax, const int m, const double x,
  double * result_array,
  double * result_deriv_array
  );
int     gsl_sf_legendre_sphPlm_e(const int l, int m, const double x, gsl_sf_result * result);
double  gsl_sf_legendre_sphPlm(const int l, const int m, const double x);
int gsl_sf_legendre_sphPlm_array(
  const int lmax, int m, const double x,
  double * result_array
  );
int gsl_sf_legendre_sphPlm_deriv_array(
  const int lmax, const int m, const double x,
  double * result_array,
  double * result_deriv_array
  );
int gsl_sf_legendre_array_size(const int lmax, const int m);
int gsl_sf_conicalP_half_e(const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_half(const double lambda, const double x);
int gsl_sf_conicalP_mhalf_e(const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_mhalf(const double lambda, const double x);
int gsl_sf_conicalP_0_e(const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_0(const double lambda, const double x);
int gsl_sf_conicalP_1_e(const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_1(const double lambda, const double x);
int gsl_sf_conicalP_sph_reg_e(const int l, const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_sph_reg(const int l, const double lambda, const double x);
int gsl_sf_conicalP_cyl_reg_e(const int m, const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_cyl_reg(const int m, const double lambda, const double x);
int gsl_sf_legendre_H3d_0_e(const double lambda, const double eta, gsl_sf_result * result);
double gsl_sf_legendre_H3d_0(const double lambda, const double eta);
int gsl_sf_legendre_H3d_1_e(const double lambda, const double eta, gsl_sf_result * result);
double gsl_sf_legendre_H3d_1(const double lambda, const double eta);
int gsl_sf_legendre_H3d_e(const int l, const double lambda, const double eta, gsl_sf_result * result);
double gsl_sf_legendre_H3d(const int l, const double lambda, const double eta);
int gsl_sf_legendre_H3d_array(const int lmax, const double lambda, const double eta, double * result_array);
]]


sf = {}

--Fermi dirac function sf.that returns [[value, error]]
function sf.fermi_dirac(n,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if n == -1 then
		status = gsl.gsl_sf_fermi_dirac_m1_e(x, result)
	elseif n == 0 then
		status = gsl.gsl_sf_fermi_dirac_0_e(x,result)
	elseif n == 1 then
		status = gsl.gsl_sf_fermi_dirac_1_e(x,result)
	elseif n == 2 then
		status = gsl.gsl_sf_fermi_dirac_2_e(x,result)
	elseif n == -0.5 then
		status = gsl.gsl_sf_fermi_dirac_mhalf_e(x,result)
	elseif n == 0.5 then
		status = gsl.gsl_sf_fermi_dirac_half_e(x,result)
	elseif n == 1.5 then
		status = gsl.gsl_sf_fermi_dirac_3half_e(x,result)
	else
		if is_integer(n) then
			status = gsl.gsl_sf_fermi_dirac_int_e(n,x,result)
		else
			error("Input n is not a correct integer value")
		end
	end

	return result.val, result.err
end

function sf.debye(n,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if n == 1 then
		status = gsl.gsl_sf_debye_1_e (x, result)
	elseif n == 2 then
		status = gsl.gsl_sf_debye_2_e (x, result)
	elseif n == 3 then
		status = gsl.gsl_sf_debye_3_e (x, result)
	elseif n == 4 then
		status = gsl.gsl_sf_debye_4_e (x, result)
	elseif n == 5 then
		status = gsl.gsl_sf_debye_5_e (x, result)
	elseif n == 6 then
		status = gsl.gsl_sf_debye_6_e (x, result)
	else
		error("n = " .. n .. " is not a valid integer value for this function.")
	end

	return result.val, result.err
end

function sf.hyperg1F1(m,n,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if is_integer(m) and is_integer(n) then
		status = gsl.gsl_sf_hyperg_1F1_int_e(m,n,x,result)
	else
		status = gsl.gsl_sf_hyperg_1F1_e(m,n,x,result)
	end

	return result.val, result.err
end

function sf.hypergU(m,n,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if is_integer(m) and is_integer(n) then
		status = gsl.gsl_sf_hyperg_U_int_e(m,n,x,result)
	else
		status = gsl.gsl_sf_hyperg_U_e(m,n,x,result)
	end

	return result.val, result.err
end

function sf.hyperg2F1conj(a,c,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	gsl.gsl_sf_hyperg_2F1_conj_e(complex.real(a), complex.imag(a), c, x, result);

	gsl_check(status)
	return result.val, result.err
end

function sf.laguerre(n,a,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if n == 1 then
		status = gsl.gsl_sf_laguerre_1_e (a, x, result)
	elseif n == 2 then
		status = gsl.gsl_sf_laguerre_2_e (a, x, result)
	elseif n == 3 then
		status = gsl.gsl_sf_laguerre_3_e (a, x, result)
	else
		if is_integer(n) then
			status = gsl.gsl_sf_laguerre_n_e (n,a, x, result);
		else
			error("n = " .. n .. " is not a valid integer value for this function.")
		end
	end

	return result.val, result.err
end


local function bessel_generic(letter, n, x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if n == 0 then
		status = gsl["gsl_sf_bessel_"..letter.."0_e"] ( x, result)
	elseif n == 1 then
		status = gsl["gsl_sf_bessel_"..letter.."1_e"] ( x, result)
	else
		if is_integer(n) then
			status = gsl["gsl_sf_bessel_"..letter.."n_e"] (n, x, result)
		else
			error("n = " .. n .. " is not a valid integer value for this function.")
		end
	end

	return result.val, result.err
end

function sf.besselJ(n,x)
	return bessel_generic("J", n,x)
end
function sf.besselY(n,x)
	return bessel_generic("Y", n,x)
end
function sf.besselI(n,x)
	return bessel_generic("I", n,x)
end
function sf.besselK(n,x)
	return bessel_generic("K", n,x)
end

function sf.besselJzero(n,s)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if s <= 0 then
		error("Second argument must be postive")
	end

	if n == 0 then
		status = gsl.gsl_sf_bessel_zero_J0_e(s,result)
	elseif n == 1 then
		status = gsl.gsl_sf_bessel_zero_J1_e(s,result)
	else
		status = gsl.gsl_sf_bessel_zero_Jnu_e(n,s,result)
	end

	gsl_check(status)
	return result.val, result.err
end

function sf.zeta(n)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if is_integer(n) then
		status = gsl.gsl_sf_zeta_int_e(n,result)
	else
		status = gsl.gsl_sf_zeta_int_e(n,result)
	end

	gsl_check(status)
	return result.val, result.err
end

function sf.expint(n,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if n == 1 then
		status = gsl.gsl_sf_expint_E1_e(x,result)
	elseif n == 2 then
		status = gsl.gsl_sf_expint_E2_e(x,result)
	else
		status = gsl.gsl_sf_expint_En_e(n,x,result)
	end

	gsl_check(status)
	return result.val, result.err
end

function sf.legendreP(n,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if n == 1 then
		status = gsl.gsl_sf_legendre_P1_e(x,result)
	elseif n == 2 then
		status = gsl.gsl_sf_legendre_P2_e(x,result)
	elseif n == 3 then
		status = gsl.gsl_sf_legendre_P3_e(x,result)
	else
		status = gsl.gsl_sf_legendre_P1_e(n,x,result)
	end

	gsl_check(status)
	return result.val, result.err
end

function sf.legendreQ(n,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	if n == 0 then
		status = gsl.gsl_sf_legendre_Q0_e(x,result)
	elseif n == 1 then
		status = gsl.gsl_sf_legendre_Q1_e(x,result)
	else
		status = gsl.gsl_sf_legendre_Ql_e(n,x,result)
	end

	gsl_check(status)
	return result.val, result.err
end

function sf.cdilog(z)
	local result_r = ffi.new("gsl_sf_result[1]")
	local result_i = ffi.new("gsl_sf_result[1]")
	local status = 0

	local r = complex.abs(z)
	local th = atan2(complex.imag(z), complex.real(z))

	status = gsl.gsl_sf_complex_dilog_e (r, th, result_r, result_i);

	gsl_check(status)
	return complex.new( result_r.val, result_i.val)
end

local GSL_PREC_DOUBLE = 0
local GSL_PREC_SINGLE = 1
local GSL_PREC_APPROX = 2

--Returns reference to a gsl function with an argument and a mode
function sf.get_gsl_sf_mode(name)
	return function(x,mode)
		return gsl["gsl_sf_"..name](x, mode or GSL_PREC_DOUBLE)
	end
end

--Returns reference to a gsl function that only takes integers
function sf.get_gsl_sf_int(name)
	return function(x)
		if is_integer(x) then
			return gsl["gsl_sf_"..name](x)
		else
			error("Argument is not an integer")
		end
	end
end

--Return refernece to a gsl function with one argument
function sf.get_gsl_sf(name)
	return gsl["gsl_sf_"..name]
end

sf.airyAi 	= sf.get_gsl_sf_mode("airy_Ai")
sf.airyBi 	= sf.get_gsl_sf_mode("airy_Bi")

sf.airyAizero 	= sf.get_gsl_sf_int("airy_zero_Ai")
sf.airyBizero 	= sf.get_gsl_sf_int("airy_zero_Bi")


sf.clausen 	= sf.get_gsl_sf("clausen")
sf.dawson 	= sf.get_gsl_sf("dawson")

sf.dilog 	= sf.get_gsl_sf("dilog")
sf.erf 		= sf.get_gsl_sf("erf")

sf.expintEi 	= sf.get_gsl_sf("expint_Ei")
sf.expint3 	= sf.get_gsl_sf("expint_3")
sf.Shi 		= sf.get_gsl_sf("Shi")
sf.Chi 		= sf.get_gsl_sf("Chi")
sf.Si 		= sf.get_gsl_sf("Si")
sf.Ci 		= sf.get_gsl_sf("Ci")
sf.atanint 	= sf.get_gsl_sf("atanint")

sf.fact 	= sf.get_gsl_sf("fact")
sf.doublefact 	= sf.get_gsl_sf("doublefact")
sf.lnfact 	= sf.get_gsl_sf("lnfact")
sf.choose 	= sf.get_gsl_sf("choose")
sf.lnchoose 	= sf.get_gsl_sf("lnchoose")

sf.gamma 	= sf.get_gsl_sf("gamma")
sf.lngamma 	= sf.get_gsl_sf("lngamma")
sf.beta 	= sf.get_gsl_sf("beta")
sf.lnbeta 	= sf.get_gsl_sf("lnbeta")

sf.hyperg0F1	= sf.get_gsl_sf("hyperg_0F1")
sf.hyperg2F1	= sf.get_gsl_sf("hyperg_2F1")

return sf
