local ffi=require 'ffi'
local defines = {}
function defines.register_hashdefs(fftw, C)
   fftw.MEASURE = 0
   fftw.DESTROY_INPUT = 1
   fftw.UNALIGNED = 2
   fftw.CONSERVE_MEMORY = 4
   fftw.EXHAUSTIVE = 8 -- /* NO_EXHAUSTIVE is default */
   fftw.PRESERVE_INPUT = 16 --  /* cancels DESTROY_INPUT */
   fftw.PATIENT =32 --  /* IMPATIENT is default */
   fftw.ESTIMATE = 64
   fftw.WISDOM_ONLY = 2097152 --  (1U << 21)

   fftw.FORWARD  = -1
   fftw.BACKWARD =  1

   fftw.NO_TIMELIMIT = (-1.0)

   -- -- typedef
   fftw.r2r_kind = ffi.typeof('fftw_r2r_kind')
   fftw.R2HC=0
   fftw.HC2R=1
   fftw.DHT=2

   fftw.REDFT00=3
   fftw.REDFT01=4
   fftw.REDFT10=5
   fftw.REDFT11=6

   fftw.RODFT00=7
   fftw.RODFT01=8
   fftw.RODFT10=9
   fftw.RODFT11=10
end

return defines