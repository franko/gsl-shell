
local function rng_test(name, seed, n, expected)
   return function()
	     local r = rng.new(name)
	     if seed ~= 0 then r:set(seed) end
	     for i = 1, n do
		k = r:int()
	     end
	     if k ~= expected then
		return "expected: " .. expected .. ", got: " .. k
	     end
	  end
end

return {
   rand       = rng_test('rand', 1, 10000, 1910041713),
   randu      = rng_test('randu', 1, 10000, 1623524161),
   cmrg       = rng_test('cmrg', 1, 10000, 719452880),
   minstd     = rng_test('minstd', 1, 10000, 1043618065),
   mrg        = rng_test('mrg', 1, 10000, 2064828650),
   taus       = rng_test('taus', 1, 10000, 2733957125),
   taus2      = rng_test('taus2', 1, 10000, 2733957125),
   taus113    = rng_test('taus113', 1, 1000, 1925420673),
   transputer = rng_test('transputer', 1, 10000, 1244127297),
   vax        = rng_test('vax', 1, 10000, 3051034865),

  --  Borosh13 test value from PARI: (1812433253^10000)%(2^32)
  borosh13 = rng_test('borosh13', 1, 10000, 2513433025),

  --  Fishman18 test value from PARI: (62089911^10000)%(2^31-1)
  fishman18 = rng_test('fishman18', 1, 10000, 330402013),

  -- Fishman2x test value from PARI:
  -- ((48271^10000)%(2^31-1) - (40692^10000)%(2^31-249))%(2^31-1)
  fishman2x = rng_test('fishman2x', 1, 10000, 540133597),

  -- Knuthran2 test value from PARI:
  --     { xn1=1; xn2=1; for (n=1,10000,
  --            xn = (271828183*xn1 - 314159269*xn2)%(2^31-1);
  --            xn2=xn1; xn1=xn; print(xn); ) }
  knuthran2 = rng_test('knuthran2', 1, 10000, 1084477620),

  --  Knuthran test value taken from p188 in Knuth Vol 2. 3rd Ed
  knuthran = rng_test('knuthran', 310952, 1009 * 2009 + 1, 461390032),

  --  Knuthran improved test value from Knuth's source
  knuthran2002_1 = rng_test('knuthran2002', 310952, 1, 708622036),
  knuthran2002_2 = rng_test('knuthran2002', 310952, 2, 1005450560),
  knuthran2002_3 = rng_test('knuthran2002', 310952, 100 * 2009 + 1, 995235265),
  knuthran2002_4 = rng_test('knuthran2002', 310952, 1009 * 2009 + 1, 704987132),

  --  Lecuyer21 test value from PARI: (40692^10000)%(2^31-249)
  lecuyer21 = rng_test('lecuyer21', 1, 10000, 2006618587),

  --  Waterman14 test value from PARI: (1566083941^10000)%(2^32)
  waterman14 = rng_test('waterman14', 1, 10000, 3776680385),

  --  specific tests of known results for 10000 iterations with seed = 6

  -- Coveyou test value from PARI:
  --    x=6; for(n=1,10000,x=(x*(x+1))%(2^32);print(x);)

  coveyou = rng_test('coveyou', 6, 10000, 1416754246),

  --  Fishman20 test value from PARI: (6*48271^10000)%(2^31-1)
  fishman20 = rng_test('fishman20', 6, 10000, 248127575),

  -- FIXME: the ranlux tests below were made by running the fortran code and
  -- getting the expected value from that. An analytic calculation
  -- would be preferable.

  ranlux = rng_test('ranlux', 314159265, 10000, 12077992),
  ranlux389 = rng_test('ranlux389', 314159265, 10000, 165942),

  ranlxs0 = rng_test('ranlxs0', 1, 10000, 11904320),
  --  0.709552764892578125 * ldexp(1.0,24)

  ranlxs1 = rng_test('ranlxs1', 1, 10000, 8734328),
  --  0.520606517791748047 * ldexp(1.0,24)

  ranlxs2 = rng_test('ranlxs2', 1, 10000, 6843140),
  --  0.407882928848266602 * ldexp(1.0,24)

  ranlxd1 = rng_test('ranlxd1', 1, 10000, 1998227290),
  --  0.465248546261094020 * ldexp(1.0,32)

  ranlxd2 = rng_test('ranlxd2', 1, 10000, 3949287736),
  --  0.919515205581550532 * ldexp(1.0,32)

  -- FIXME: the tests below were made by running the original code in
  -- the ../random directory and getting the expected value from
  -- that. An analytic calculation would be preferable.

  slatec = rng_test('slatec', 1, 10000, 45776),
  uni = rng_test('uni', 1, 10000, 9214),
  uni32 = rng_test('uni32', 1, 10000, 1155229825),
  zuf = rng_test('zuf', 1, 10000, 3970),

  -- The tests below were made by running the original code and
  -- getting the expected value from that. An analytic calculation
  -- would be preferable.

  r250 = rng_test('r250', 1, 10000, 1100653588),
  mt19937 = rng_test('mt19937', 4357, 1000, 1186927261),
  mt19937_1999 = rng_test('mt19937_1999', 4357, 1000, 1030650439),
  mt19937_1998 = rng_test('mt19937_1998', 4357, 1000, 1309179303),
  tt800 = rng_test('tt800', 0, 10000, 2856609219),

  ran0 = rng_test('ran0', 0, 10000, 1115320064),
  ran1 = rng_test('ran1', 0, 10000, 1491066076),
  ran2 = rng_test('ran2', 0, 10000, 1701364455),
  ran3 = rng_test('ran3', 0, 10000, 186340785),

  ranmar = rng_test('ranmar', 1, 10000, 14428370),

  rand48_1 = rng_test('rand48', 0, 10000, 0xDE095043),
  rand48_2 = rng_test('rand48', 1, 10000, 0xEDA54977),

  ranf_1 = rng_test('ranf', 0, 10000, 2152890433),
  ranf_2 = rng_test('ranf', 2, 10000, 339327233),
}
