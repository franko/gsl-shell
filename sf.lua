--Spatial function sf.module of gsl-shell
--Documentation overview: http://www.gnu.org/software/gsl/manual/html_node/Special-Functions.html

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

----------------------------------------------------------------------------------------------------------
--This section contains small and versatile wrapper functions for the generically named special functions
----------------------------------------------------------------------------------------------------------

local GSL_PREC_DOUBLE = 0
local GSL_PREC_SINGLE = 1
local GSL_PREC_APPROX = 2

--stores the result of a special function call (global because we do not want to allocate memory at every function call and it is not nescessary)
local result = ffi.new("gsl_sf_result")
local result10 = ffi.new("gsl_sf_result_e10")

--checks an argument list for the correct count and signals error in false cases
local function check_arg(arg, num_arg)
	if arg == nil then error("Did not give argument count...") end
	if #arg ~= num_arg then
		error("You gave " .. #arg .. "arguments, but the function needs " .. num_arg .. " arguments. Please consult the documentation.")
	end
end

--Returns reference to a wrapper function that wraps a gsl sf mode function (with error) with a certain name and a variable number of arguments num_arg
local function get_gsl_sf_mode(name, num_arg)   
    local fullname = "gsl_sf_"..name.."_e"
    
	return function(...)
		local arg = {...}

		local status = 0
		
		--check for correct argument count
        --is left out due to performance reasons, shell will notify about errors anyway
		--check_arg(arg, num_arg)

		--very ugly construction with variable argument list
		--a simple insertion of ... in this C function call would always only give the first (and not automatically all the others, as one would suspect)
		if num_arg == 1 then status = gsl[fullname](arg[1],GSL_PREC_DOUBLE,result)
		elseif num_arg == 2 then status = gsl[fullname](arg[1], arg[2],GSL_PREC_DOUBLE,result)
		elseif num_arg == 3 then status = gsl[fullname](arg[1], arg[2],arg[3],GSL_PREC_DOUBLE,result)
		elseif num_arg == 4 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],GSL_PREC_DOUBLE,result)
		elseif num_arg == 5 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],GSL_PREC_DOUBLE,result)
		elseif num_arg == 6 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],arg[6],GSL_PREC_DOUBLE,result)
		elseif num_arg == 7 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],GSL_PREC_DOUBLE,result)
		elseif num_arg == 8 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],GSL_PREC_DOUBLE,result)
		elseif num_arg == 9 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],GSL_PREC_DOUBLE,result) end	
		
		--do the error checking and return with the result
		gsl_check(status)
        sf.last_error = result.err
		return result.val
	end
end

--Returns wrapper to a gsl function that only takes one integer
local function get_gsl_sf_int(name)
    local fullname = "gsl_sf_"..name.."_e"
    
	return function(x)
		if is_integer(x) then
			--execute the gsl function
			local status = gsl[fullname](x,result)
			gsl_check(status)
			sf.last_error = result.err
            return result.val
		else
			error("Argument is not an integer")
		end
	end
end

--calls a gsl function by the name of 'fullname' with a variable argument list 'arg' and stores the result in 'result'
local function call_variable_arg(fullname,arg, result)
	local num_arg = #arg
	local status = 0
	--very ugly construction with variable argument list
	if num_arg == 1 then status = gsl[fullname](arg[1],result)
	elseif num_arg == 2 then status = gsl[fullname](arg[1], arg[2],result)
	elseif num_arg == 3 then status = gsl[fullname](arg[1], arg[2],arg[3],result)
	elseif num_arg == 4 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],result)
	elseif num_arg == 5 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],result)
	elseif num_arg == 6 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],arg[6],result)
	elseif num_arg == 7 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],result)
	elseif num_arg == 8 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],result)
	elseif num_arg == 9 then status = gsl[fullname](arg[1], arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],result) end	
	return status
end

--Returns warpper to a gsl function with variable argument count
local function get_gsl_sf(name, num_arg)    
    local fullname = "gsl_sf_"..name.."_e"
    
	return function(...)
		local arg = {...}
		
        --check for correct argument count
        --is left out due to performance reasons, shell will notify about errors anyway
        --check_arg(arg, num_arg)

		local status = call_variable_arg(fullname, arg, result)

		gsl_check(status)
		sf.last_error = result.err
        return result.val
	end
end

--Returns warpper to a gsl function with variable argument count and additional result10 error
local function get_gsl_sf10(name, num_arg)    
    local fullname = "gsl_sf_"..name.."_e"
    
    return function(...)
        local arg = {...}
        
        --check for correct argument count
        --is left out due to performance reasons, shell will notify about errors anyway
        --check_arg(arg, num_arg)

        local status = call_variable_arg(fullname, arg, result10)

        gsl_check(status)
        sf.last_error = result10.err
        sf.last_error10 = result10.e10
        return result10.val
    end
end

--Returns wrapper for a gsl function that takes an order as the first argument
--This will map to different functions according to the order
--The mapping is stored in the choices table as {[[order]]=='function_name',...}
local function get_gsl_sf_choice(choices, num_arg)
    for k,v in pairs(choices) do choices[k] = "gsl_sf_" .. choices[k] .. "_e" end
    
	return function(n,...)
		local arg = {...}
		
		--check for correct argument count
        --is left out due to performance reasons, shell will notify about errors anyway
        --check_arg(arg, num_arg)

		--look if the order choice n is in the list, if yes, we will choose this function, otherwise, look for a '?' entry
		local l
		if choices[n] == nil and is_integer(n) then
			l = n
			n = '?'
		end
		
		--if we are using the generic function, we have to begin the argument list with the n value
		if n == '?' then table.insert(arg, 1, tonumber(l)) end

		local status = call_variable_arg(choices[n],arg, result)

		--check the status and return the result
		gsl_check(status)
        sf.last_error = result.err
        return result.val
	end
end

--wrapper function for gsl functions with integer and double version
local function get_gsl_sf_int_double(name_int, name_double)
    local int_name = "gsl_sf_" .. name_int .. "_e"
    local double_name = "gsl_sf_" .. name_double .. "_e"
    
	return function(n)

		local status = 0

		if is_integer(n) then
			status = gsl[int_name](n,result)
		else
			status = gsl[double_name](n,result)
		end

        --check the status and return the result
        gsl_check(status)
        sf.last_error = result.err
        return result.val
	end
end

-------------------------------------------------------
--Create the main special function table that holds the namespace for all the warpper functions
sf = {}
-------------------------------------------------------

--Returns the last computed error
function sf.get_last_error()
   return sf.last_error
end

--Returns the last computed error10 value
function sf.get_last_error10()
   return sf.last_error10
end
-------------------------------------------------------

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

sf.airyAi 		= get_gsl_sf_mode("airy_Ai", 1)
sf.airyBi 		= get_gsl_sf_mode("airy_Bi", 1)
sf.airyAi_scaled 	= get_gsl_sf_mode("airy_Ai_scaled", 1)
sf.airyBi_scaled 	= get_gsl_sf_mode("airy_Bi_scaled", 1)

sf.airyAi_deriv 		= get_gsl_sf_mode("airy_Ai_deriv", 1)
sf.airyBi_deriv 		= get_gsl_sf_mode("airy_Bi_deriv", 1)
sf.airyAi_deriv_scaled 		= get_gsl_sf_mode("airy_Ai_deriv_scaled", 1)
sf.airyBi_deriv_scaled 		= get_gsl_sf_mode("airy_Bi_deriv_scaled", 1)

sf.airyAizero 		= get_gsl_sf_int("airy_zero_Ai",1 )
sf.airyBizero 		= get_gsl_sf_int("airy_zero_Bi",1 )

sf.airyAizero_deriv 		= get_gsl_sf_int("airy_zero_Ai_deriv", 1 )
sf.airyBizero_deriv 		= get_gsl_sf_int("airy_zero_Bi_deriv", 1 )

-------------------------------------------------------

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

local function bessel_generic(letter, suffix, integer_name)
	suffix = suffix or ""
	integer_name = integer_name or "n"
	return function(n,x)
		local result = ffi.new("gsl_sf_result")
		local status = 0
		if n == 0 then
			status = gsl["gsl_sf_bessel_"..letter.."0"..suffix.."_e"] ( x, result)
		elseif n == 1 then
			status = gsl["gsl_sf_bessel_"..letter.."1"..suffix.."_e"] ( x, result)
		else
			if is_integer(n) then
				status = gsl["gsl_sf_bessel_"..letter..integer_name..suffix.."_e"] (n, x, result)
			else
				error("n = " .. n .. " is not a valid integer value for this function.")
			end
		end

		gsl_check(status)
		return result.val, result.err
	end
end

local function bessel_nu_generic(letter, suffix, prefix)
	suffix = suffix or ""
	prefix = prefix or ""
	return get_gsl_sf("bessel_"..prefix..letter.."nu" .. suffix, 2)	
end

--Regular Cylindrical
sf.besselJ 		= bessel_generic("J")
--Irregular Cylindrical
sf.besselY 		= bessel_generic("Y")

--Regular Modified Cylindrical
sf.besselI 		= bessel_generic("I")
--Regular Modified Cylindrical scaled
sf.besselI_scaled 	= bessel_generic("I","_scaled")

--Irregular Modified Cylindrical
sf.besselK 		= bessel_generic("K")
--Irregular Modified Cylindrical scaled
sf.besselK_scaled 	= bessel_generic("K","_scaled")


--Regular Spherical
sf.besselj 		= bessel_generic("j","","l")
--Irregular Spherical
sf.bessely 		= bessel_generic("y","","l")

--Regular Modified Spherical
--sf.besseli 		= bessel_generic("i","","l")
--Regular Modified Spherical scaled
sf.besseli_scaled 	= bessel_generic("i","_scaled","l")

--Irregular Modified Spherical
--sf.besselk		 = bessel_generic("k","","l")
--Irregular Modified Spherical scaled
sf.besselk_scaled 	= bessel_generic("k","_scaled", "l")


sf.besselJnu		= bessel_nu_generic("J")
sf.besselYnu		= bessel_nu_generic("Y")
sf.besselInu		= bessel_nu_generic("I")
sf.besselInu_scaled	= bessel_nu_generic("I", "_scaled")
sf.besselKnu		= bessel_nu_generic("K")
sf.bessellnKnu		= bessel_nu_generic("K","","ln")
sf.besselKnu_scaled	= bessel_nu_generic("K", "_scaled")


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


-------------------------------------------------------

--Definition of clausen file
ffi.cdef[[
int gsl_sf_clausen_e(double x, gsl_sf_result * result);
double gsl_sf_clausen(const double x);
]]

sf.clausen 	= get_gsl_sf("clausen", 1)

-------------------------------------------------------

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

sf.hydrogenicR_1 = get_gsl_sf("hydrogenicR_1", 2)
sf.coulomb_CL = get_gsl_sf("coulomb_CL", 2)

function sf.hydrogenicR(n, l, Z, r)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	status = gsl["gsl_sf_hydrogenicR_e"] ( n, l, Z, r, result)

	gsl_check(status)
	return result.val, result.err
end

function sf.coulomb_wave_FG(eta, x, L_F, k)
	local F = ffi.new("gsl_sf_result")
	local Fp = ffi.new("gsl_sf_result")
	local G = ffi.new("gsl_sf_result")
	local Gp = ffi.new("gsl_sf_result")
	local exp_F = ffi.new("double[1]")
	local exp_G = ffi.new("double[1]")
	local status = 0

	status = gsl["gsl_sf_coulomb_wave_FG_e"](eta, x, L_F,k,F,Fp,G,Gp,exp_F,exp_G)

	gsl_check(status)
	return {	F=F.val,	F_err=F.err,
			Fp=Fp.val,	Fp_err=Fp.err,
			G=G.val,	G_err=G.err,
			Gp=Gp.val,	Gp_err=Gp.err,
			exp_F = exp_F,	exp_G = exp_G }
end


-------------------------------------------------------

--Definition of coupling coefficients file
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

sf.coupling_3j = get_gsl_sf("coupling_3j",6)
sf.coupling_6j = get_gsl_sf("coupling_6j",6)
sf.coupling_9j = get_gsl_sf("coupling_9j",9)
--sf.coupling_RacahW = get_gsl_sf("coupling_RacahW",6)
--sf.coupling_6j_INCORRECT = get_gsl_sf("coupling_6j_INCORRECT",6)


-------------------------------------------------------

--Definition of dawson file
ffi.cdef[[
int     gsl_sf_dawson_e(double x, gsl_sf_result * result);
double     gsl_sf_dawson(double x);
]]

sf.dawson 	= get_gsl_sf("dawson", 1)

-------------------------------------------------------

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

	gsl_check(status)
	return result.val, result.err
end

-------------------------------------------------------

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

function sf.cdilog(z)
	local result_r = ffi.new("gsl_sf_result")
	local result_i = ffi.new("gsl_sf_result")
	local status = 0

	local r = complex.abs(z)
	local th = math.atan2(complex.imag(z), complex.real(z))

	status = gsl.gsl_sf_complex_dilog_e (r, th, result_r, result_i);

	gsl_check(status)
	return complex.new( result_r.val, result_i.val)
end


sf.dilog 	= get_gsl_sf("dilog", 1)


-------------------------------------------------------

--Definition of elementary file
ffi.cdef[[
int gsl_sf_multiply_e(const double x, const double y, gsl_sf_result * result);
double gsl_sf_multiply(const double x, const double y);
int gsl_sf_multiply_err_e(const double x, const double dx, const double y, const double dy, gsl_sf_result * result);
]]

sf.multiply = get_gsl_sf("multiply", 2)
sf.multiply_err = get_gsl_sf("multiply_err", 4)

-------------------------------------------------------

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

sf.ellint_Kcomp = get_gsl_sf_mode("ellint_Kcomp",1)
sf.ellint_Ecomp = get_gsl_sf_mode("ellint_Ecomp",1)

sf.ellint_F = get_gsl_sf_mode("ellint_F",2)
sf.ellint_E = get_gsl_sf_mode("ellint_E",2)
sf.ellint_P = get_gsl_sf_mode("ellint_P",3)
sf.ellint_D = get_gsl_sf_mode("ellint_D",3)

sf.ellint_RC = get_gsl_sf_mode("ellint_RC",2)
sf.ellint_RD = get_gsl_sf_mode("ellint_RD",3)
sf.ellint_RF = get_gsl_sf_mode("ellint_RF",3)
sf.ellint_RJ = get_gsl_sf_mode("ellint_RJ",4)

-------------------------------------------------------

--Definition of elljac file
ffi.cdef[[
int gsl_sf_elljac_e(double u, double m, double * sn, double * cn, double * dn);
]]

function sf.elljac(u, m)
	local sn = ffi.new("double[1]")
	local cn = ffi.new("double[1]")
	local dn = ffi.new("double[1]")
	local status = gsl.gsl_sf_elljac_e(u,m,sn,cn,dn)

	gsl_check(status)
	return sn, cn, dn
end


-------------------------------------------------------

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


sf.erf 			= get_gsl_sf("erf", 1)
sf.erfc			= get_gsl_sf("erfc", 1)
sf.log_erfc		= get_gsl_sf("log_erfc", 1)
sf.erf_Z		= get_gsl_sf("erf_Z", 1)
sf.erf_Q		= get_gsl_sf("erf_Q", 1)
sf.hazard		= get_gsl_sf("hazard", 1)

-------------------------------------------------------

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

sf.exp			= get_gsl_sf("exp", 1)
sf.exp_e10		= get_gsl_sf10("exp_e10", 1)
sf.exp_mult		= get_gsl_sf("exp_mult", 2)
sf.exp_mult_e10		= get_gsl_sf10("exp_mult_e10", 2)

sf.expm1		= get_gsl_sf("expm1", 1)
sf.exprel		= get_gsl_sf("exprel", 1)
sf.exprel_2		= get_gsl_sf("exprel_2", 1)
sf.exprel_n		= get_gsl_sf("exprel_n", 2)

sf.exp_err		= get_gsl_sf("exp_err", 2)
sf.exp_err_e10		= get_gsl_sf10("exp_err_e10", 2)
sf.exp_mult_err		= get_gsl_sf("exp_mult_err", 4)
sf.exp_mult_err_e10	= get_gsl_sf10("exp_mult_err_e10", 4)

-------------------------------------------------------

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

sf.expint_E			= get_gsl_sf_choice( {[1]="expint_E1", [2]="expint_E2", ['?']="expint_En"},1)
sf.expint_Ei		= get_gsl_sf("expint_Ei", 1)

sf.Shi			= get_gsl_sf("Shi", 1)
sf.Chi			= get_gsl_sf("Chi", 1)

sf.expint3		= get_gsl_sf("expint_3", 1)
sf.Si			= get_gsl_sf("Si", 1)
sf.Ci			= get_gsl_sf("Ci", 1)
sf.atanint		= get_gsl_sf("atanint", 1)

-------------------------------------------------------

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

sf.fermi_dirac	= get_gsl_sf_choice({
									[-1]="fermi_dirac_m1",
									[0]="fermi_dirac_0",
									[1]="fermi_dirac_1",
									[2]="fermi_dirac_2",
									[-0.5]="fermi_dirac_mhalf",
									[0.5]="fermi_dirac_half",
									[1.5]="fermi_dirac_3half",
									['?']="fermi_dirac_int"}, 1)

sf.fermi_dirac_inc = get_gsl_sf("fermi_dirac_inc_0", 2)


-------------------------------------------------------

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



sf.fact 			= get_gsl_sf("fact", 1)
sf.doublefact 		= get_gsl_sf("doublefact", 1)
sf.lnfact 			= get_gsl_sf("lnfact", 1)
sf.lndoublefact 	= get_gsl_sf("lndoublefact", 1)

sf.choose 			= get_gsl_sf("choose",2)
sf.lnchoose 		= get_gsl_sf("lnchoose",2)
sf.taylorcoeff 		= get_gsl_sf("taylorcoeff", 2)

sf.gamma 			= get_gsl_sf("gamma", 1)
sf.lngamma 			= get_gsl_sf("lngamma", 1)

sf.gammastar		= get_gsl_sf("gammastar", 1)
sf.gammainv			= get_gsl_sf("gammainv", 1)

sf.gamma_inc		= get_gsl_sf("gamma_inc", 2)
sf.gamma_inc_Q		= get_gsl_sf("gamma_inc_Q", 2)
sf.gamma_inc_P		= get_gsl_sf("gamma_inc_P", 2) 

function sf.lngammac(z)

   local lnr = ffi.new("gsl_sf_result")
   local arg = ffi.new("gsl_sf_result")

   local status = gsl.gsl_sf_lngamma_complex_e( complex.real(z), complex.imag(z), lnr, arg)

   gsl_check(status)
   
   return lnr.val, lnr.err, arg.val, arg.err
end

sf.beta 	= get_gsl_sf("beta", 2)
sf.lnbeta 	= get_gsl_sf("lnbeta", 2)
sf.beta_inc	= get_gsl_sf("beta_inc", 3)

sf.poch		= get_gsl_sf("poch", 2)
sf.lnpoch	= get_gsl_sf("lnpoch", 2)
sf.pochrel	= get_gsl_sf("pochrel", 2)

-------------------------------------------------------

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

sf.gegenpoly = get_gsl_sf_choice({[1]="gegenpoly_1", [2]="gegenpoly_2", [3]="gegenpoly_3", ['?']="gegenpoly_n"}, 2)

-------------------------------------------------------

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

function sf.hyperg2F1conj_renorm(a,c,x)
	local result = ffi.new("gsl_sf_result")
	local status = 0

	gsl.gsl_sf_hyperg_2F1_conj_renorm_e(complex.real(a), complex.imag(a), c, x, result);

	gsl_check(status)
	return result.val, result.err
end
sf.hyperg0F1		= get_gsl_sf("hyperg_0F1", 2)
sf.hyperg2F1		= get_gsl_sf("hyperg_2F1", 4)
sf.hyperg2F1_renorm	= get_gsl_sf("hyperg_2F1_renorm", 4)
sf.hyperg2F0		= get_gsl_sf("hyperg_2F0", 3)

-------------------------------------------------------

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

sf.laguerre = get_gsl_sf_choice({[1]="laguerre_1",[2]="laguerre_2", [3]="laguerre_3",['?']="laguerre_n"},2)

-------------------------------------------------------

--Definition of lambert file
ffi.cdef[[
int     gsl_sf_lambert_W0_e(double x, gsl_sf_result * result);
double  gsl_sf_lambert_W0(double x);
int     gsl_sf_lambert_Wm1_e(double x, gsl_sf_result * result);
double  gsl_sf_lambert_Wm1(double x);
]]

sf.lambertW0	= get_gsl_sf("lambert_W0", 1)
sf.lambertWm1	= get_gsl_sf("lambert_Wm1", 1)


-------------------------------------------------------

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

sf.legendreP	 	= get_gsl_sf_choice({[1]='legendre_P1', [2]='legendre_P2', [3]='legendre_P3', ['?']='legendre_Pl'}, 1)
sf.legendreQ	 	= get_gsl_sf_choice({[0]='legendre_Q0', [1]='legendre_Q1', ['?']='legendre_Ql'}, 1)

sf.legendrePlm		= get_gsl_sf("legendre_Plm", 3)
sf.legendresphPlm	= get_gsl_sf("legendre_sphPlm", 3)

sf.conicalP		= get_gsl_sf_choice({[0.5]='conicalP_half',[-0.5]='conicalP_mhalf',[0]='conicalP_0',[1]='conicalP_1'}, 2)

sf.conicalPsphreg	= get_gsl_sf("conicalP_sph_reg", 3)
sf.conicalPcylreg	= get_gsl_sf("conicalP_cyl_reg", 3)

sf.legendreH3d		= get_gsl_sf_choice({[0]='legendre_H3d_0',[1]='legendre_H3d_1',['?']='legendre_H3d'},2)

-------------------------------------------------------

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

sf.log		= get_gsl_sf("log", 1)
sf.log_abs	= get_gsl_sf("log_abs", 1)
sf.log_1plusx	= get_gsl_sf("log_1plusx", 1)
sf.log_1plusx_mx= get_gsl_sf("log_1plusx_mx", 1)

-------------------------------------------------------

--Definition of pow_int file
ffi.cdef[[
int     gsl_sf_pow_int_e(double x, int n, gsl_sf_result * result);
double  gsl_sf_pow_int(const double x, const int n);
]]

sf.pow_int	= get_gsl_sf("pow_int", 2)

-------------------------------------------------------

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

sf.psi	= get_gsl_sf_int_double("psi_int", "psi")
sf.psi_1piy	= get_gsl_sf("psi_1piy", 1)

sf.psi_1 = get_gsl_sf_int_double("psi_1_int", "psi_1")

sf.psi_n	= get_gsl_sf("psi_n", 2)



-------------------------------------------------------

--Definition of syn file
ffi.cdef[[
int     gsl_sf_synchrotron_1_e(const double x, gsl_sf_result * result);
double     gsl_sf_synchrotron_1(const double x);
int     gsl_sf_synchrotron_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_synchrotron_2(const double x);
]]

sf.synchrotron1	= get_gsl_sf("synchrotron_1", 1)--get_gsl_sf_choice({[1]="synchrotron_1", [2]='synchrotron_2'},1)
sf.synchrotron2 = get_gsl_sf("synchrotron_2", 1)


-------------------------------------------------------

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

sf.transport	= get_gsl_sf_choice({[2]='transport_2',[3]='transport_3',[4]='transport_4',[5]='transport_5'},1)

-------------------------------------------------------

--Definition of ma file
--[[ffi.cdef[[
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

-------------------------------------------------------

--Definition of trig file
--[[ffi.cdef[[
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

-------------------------------------------------------

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

sf.zeta		= get_gsl_sf_int_double("zeta_int", "zeta")
sf.zetam1	= get_gsl_sf_int_double("zetam1_int", "zetam1")
sf.eta		= get_gsl_sf_int_double("eta_int", "eta")
sf.hzeta	= get_gsl_sf("hzeta", 2)


-------------------------------------------------------


return sf
