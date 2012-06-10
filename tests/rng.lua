
local function rng_test(name, seed, n)
   return function()
	     local r = rng.new(name)
	     if seed ~= 0 then r:set(seed) end
	     for i = 1, n do
		k = r:int()
	     end
	     return k
	  end
end

return {
   rand       = rng_test('rand', 1, 10000), -- expected: 1910041713
   randu      = rng_test('randu', 1, 10000), -- expected: 1623524161
   cmrg       = rng_test('cmrg', 1, 10000), -- expected: 719452880
   minstd     = rng_test('minstd', 1, 10000), -- expected: 1043618065
   mrg        = rng_test('mrg', 1, 10000), -- expected: 2064828650
   taus       = rng_test('taus', 1, 10000), -- expected: 2733957125UL
   taus2      = rng_test('taus2', 1, 10000), -- expected: 2733957125UL
   taus113    = rng_test('taus113', 1, 1000), -- expected: 1925420673UL
   transputer = rng_test('transputer', 1, 10000), -- expected: 1244127297UL
   vax        = rng_test('vax', 1, 10000), -- expected: 3051034865UL

  --  Borosh13 test value from PARI: (1812433253^10000)%(2^32)
  borosh13 = rng_test('borosh13', 1, 10000), -- expected: 2513433025UL

  --  Fishman18 test value from PARI: (62089911^10000)%(2^31-1)
  fishman18 = rng_test('fishman18', 1, 10000), -- expected: 330402013UL

  -- Fishman2x test value from PARI:
  -- ((48271^10000)%(2^31-1) - (40692^10000)%(2^31-249))%(2^31-1)
  fishman2x = rng_test('fishman2x', 1, 10000), -- expected: 540133597UL

  -- Knuthran2 test value from PARI:
  --     { xn1=1; xn2=1; for (n=1,10000,
  --            xn = (271828183*xn1 - 314159269*xn2)%(2^31-1);
  --            xn2=xn1; xn1=xn; print(xn); ) }
  knuthran2 = rng_test('knuthran2', 1, 10000), -- expected: 1084477620UL

  --  Knuthran test value taken from p188 in Knuth Vol 2. 3rd Ed
  knuthran = rng_test('knuthran', 310952, 1009 * 2009 + 1), -- expected: 461390032

  --  Knuthran improved test value from Knuth's source
  knuthran2002_1 = rng_test('knuthran2002', 310952, 1), -- expected: 708622036
  knuthran2002_2 = rng_test('knuthran2002', 310952, 2), -- expected: 1005450560
  knuthran2002_3 = rng_test('knuthran2002', 310952, 100 * 2009 + 1), -- expected: 995235265
  knuthran2002_4 = rng_test('knuthran2002', 310952, 1009 * 2009 + 1), -- expected: 704987132

  --  Lecuyer21 test value from PARI: (40692^10000)%(2^31-249)
  lecuyer21 = rng_test('lecuyer21', 1, 10000), -- expected: 2006618587UL

  --  Waterman14 test value from PARI: (1566083941^10000)%(2^32)
  waterman14 = rng_test('waterman14', 1, 10000), -- expected: 3776680385UL

  --  specific tests of known results for 10000 iterations with seed = 6

  -- Coveyou test value from PARI:
  --    x=6; for(n=1,10000,x=(x*(x+1))%(2^32);print(x);)

  coveyou = rng_test('coveyou', 6, 10000), -- expected: 1416754246UL

  --  Fishman20 test value from PARI: (6*48271^10000)%(2^31-1)
  fishman20 = rng_test('fishman20', 6, 10000), -- expected: 248127575UL

  -- FIXME: the ranlux tests below were made by running the fortran code and
  -- getting the expected value from that. An analytic calculation
  -- would be preferable.

  ranlux = rng_test('ranlux', 314159265, 10000), -- expected: 12077992
  ranlux389 = rng_test('ranlux389', 314159265, 10000), -- expected: 165942

  ranlxs0 = rng_test('ranlxs0', 1, 10000), -- expected: 11904320
  --  0.709552764892578125 * ldexp(1.0,24)

  ranlxs1 = rng_test('ranlxs1', 1, 10000), -- expected: 8734328
  --  0.520606517791748047 * ldexp(1.0,24)

  ranlxs2 = rng_test('ranlxs2', 1, 10000), -- expected: 6843140
  --  0.407882928848266602 * ldexp(1.0,24)

  ranlxd1 = rng_test('ranlxd1', 1, 10000), -- expected: 1998227290UL
  --  0.465248546261094020 * ldexp(1.0,32)

  ranlxd2 = rng_test('ranlxd2', 1, 10000), -- expected: 3949287736UL
  --  0.919515205581550532 * ldexp(1.0,32)

  -- FIXME: the tests below were made by running the original code in
  -- the ../random directory and getting the expected value from
  -- that. An analytic calculation would be preferable.

  slatec = rng_test('slatec', 1, 10000), -- expected: 45776
  uni = rng_test('uni', 1, 10000), -- expected: 9214
  uni32 = rng_test('uni32', 1, 10000), -- expected: 1155229825
  zuf = rng_test('zuf', 1, 10000), -- expected: 3970

  -- The tests below were made by running the original code and
  -- getting the expected value from that. An analytic calculation
  -- would be preferable.

  r250 = rng_test('r250', 1, 10000), -- expected: 1100653588
  mt19937 = rng_test('mt19937', 4357, 1000), -- expected: 1186927261
  mt19937_1999 = rng_test('mt19937_1999', 4357, 1000), -- expected: 1030650439
  mt19937_1998 = rng_test('mt19937_1998', 4357, 1000), -- expected: 1309179303
  tt800 = rng_test('tt800', 0, 10000), -- expected: 2856609219UL

  ran0 = rng_test('ran0', 0, 10000), -- expected: 1115320064
  ran1 = rng_test('ran1', 0, 10000), -- expected: 1491066076
  ran2 = rng_test('ran2', 0, 10000), -- expected: 1701364455
  ran3 = rng_test('ran3', 0, 10000), -- expected: 186340785

  ranmar = rng_test('ranmar', 1, 10000), -- expected: 14428370

  rand48_1 = rng_test('rand48', 0, 10000), -- expected: 0xDE095043UL
  rand48_2 = rng_test('rand48', 1, 10000), -- expected: 0xEDA54977UL

  ranf_1 = rng_test('ranf', 0, 10000), -- expected: 2152890433UL
  ranf_2 = rng_test('ranf', 2, 10000), -- expected: 339327233
}
