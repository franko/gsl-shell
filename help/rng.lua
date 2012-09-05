local RNG = rng.new()

local M = {
	[rng.new] = [[
rng.new([name])

   This function returns a "random number generator" object of the
   specified type "name". If you do not specify a particular generator
   the default "taus2" generator will be used.
  ]],
	[rng.list] = [[
rng.list()

   Return an array with all the list of all the supported generator
   type.
]],
 	[RNG.get] = [[
<rng>:get()

   This function returns a real number uniformly distributed in the
   range [0,1).  The range includes 0.0 but excludes 1.0. Some
   generators compute this ratio internally so that they can provide
   floating point numbers with more than 32 bits of randomness.
 ]],
 	[RNG.getint] = [[
<rng>:getint(n)

   This function returns a random integer from 0 to n-1 inclusive by
   scaling down and/or discarding samples from the generator R.  All
   integers in the range [0,n-1] are produced with equal probability.
]],
 	[RNG.set] = [[
<rng>:set(seed)

   This method set the seed of the generator to the given integer
   value.
]],
}

return M

