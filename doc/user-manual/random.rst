.. highlight:: lua

.. include:: <isogrk1.txt>

Random Numbers
==============

This chapter describes routines for generating random numbers. GSL
provides a large collection of random number generators that can be
accessed through a uniform interface. You can choose very easily the
generator you want to use and even switch between generators without
needing to change your commands.

Each instance of a generator keeps track of its own state so that you
can have several different generators and they will not interfere with
each other. Additional functions are available for transforming
uniform random numbers into samples from continuous or discrete
probability distributions such as the Gaussian, log-normal or Poisson
distributions.

General comments on random numbers
----------------------------------

In 1988, Park and Miller wrote a paper entitled "Random number
generators: good ones are hard to find." [park-miller]_.
Fortunately, some excellent random number generators are available,
though poor ones are still in common use.  You may be happy with the
system-supplied random number generator on your computer, but you should
be aware that as computers get faster, requirements on random number
generators increase.  Nowadays, a simulation that calls a random number
generator millions of times can often finish before you can make it down
the hall to the coffee machine and back.

A very nice review of random number generators was written by Pierre
L'Ecuyer, as Chapter 4 of the book: Handbook on Simulation, Jerry
Banks, ed. (Wiley, 1997).  The chapter is available in postscript from
L'Ecuyer's ftp site (see references).  Knuth's volume on Seminumerical
Algorithms (originally published in 1968) devotes 170 pages to random
number generators, and has recently been updated in its 3rd edition
(1997).  It is brilliant, a classic.  If you don't own it, you should
stop reading right now, run to the nearest bookstore, and buy it.

A good random number generator will satisfy both theoretical and
statistical properties.  Theoretical properties are often hard to
obtain (*they require real math!*), but one prefers a random number
generator with a long period, low serial correlation, and a tendency
*not* to "fall mainly on the planes."  Statistical tests are performed
with numerical simulations.  Generally, a random number generator is
used to estimate some quantity for which the theory of probability
provides an exact answer.  Comparison to this exact answer provides a
measure of "randomness".

The Random Number Generator Interface
-------------------------------------

In order to generate random numbers you need a "Random Number
Generator" or ``rng`` for short. A random number generator is an
object that can be used to generate number whenever is needed. Of
course that is just what you want for a random number generator, but
to achieve this effect, the generator must keep track of some kind of
"state" variable.  Sometimes this state is just an integer (sometimes
just the value of the previously generated random number), but often
it is more complicated than that and may involve a whole array of
numbers, possibly with some indices thrown in.  To use the random
number generators, you do not need to know the details of what
comprises the state, and besides that varies from algorithm to
algorithm.

.. module:: rng

So, if you want to generate random numbers the first step is to use the :func:`rng.new` function,

.. function:: new([name])
   
   This function returns a "random number generator" object of the
   specified type ``name``. If you do not specify a particular
   generator the default ``taus2`` generator will be used. See the
   paragraph on :ref:`algorithms <rng-algorithms>` for a description
   of the different algorithms available.

.. class:: Rng

   .. method:: get()

     This function returns a real number uniformly distributed in the
     range [0,1).  The range includes 0.0 but excludes 1.0. Some
     generators compute this ratio internally so that they can provide
     floating point numbers with more than 32 bits of randomness.

   .. method:: getint(n)

     This function returns a random integer from 0 to n-1 inclusive by
     scaling down and/or discarding samples from the generator R.  All
     integers in the range [0,n-1] are produced with equal probability.

   .. method:: set(seed)
   
     This method set the seed of the generator to the given integer value.

.. function:: list()

     Return an array with all the list of all the supported generator type.

Examples
--------

So here a simple example that prints 20 integer numbers between 0 and 999::

   r = rng.new() -- we create a random number generator
   for k=1,20 do
      local n = r:getint(1000) -- we obtain a random integer < 1000 
      print(n)
   end

and here another example that creates a 5x5 matrix and initiliaze it with random numbers like before::

   r = rng.new() -- we create a random number generator
   m = new(5, 5, |i,j| r:getint(1000)) -- create the matrix

.. _rng-algorithms:

Random Number Generator Algorithms
----------------------------------

The following generators are recommended for use in simulation.  They
have extremely long periods, low correlation and pass most statistical
tests.  For the most reliable source of uncorrelated numbers, the
second-generation RANLUX generators have the strongest proof of
randomness.

**mt19937**
     The MT19937 generator of Makoto Matsumoto and Takuji Nishimura is a
     variant of the twisted generalized feedback shift-register
     algorithm, and is known as the "Mersenne Twister" generator.  It
     has a Mersenne prime period of 2\ :sup:`19937`  - 1 (about 10\ :sup:`6000` ) and is
     equi-distributed in 623 dimensions.  It has passed the DIEHARD
     statistical tests.  It uses 624 words of state per generator and is
     comparable in speed to the other generators.  The original
     generator used a default seed of 4357 and choosing S equal to zero
     in 'set' reproduces this.  Later versions switched to 5489
     as the default seed, you can choose this explicitly via
     'set' instead if you require it.

     For more information see,

     -    Makoto Matsumoto and Takuji Nishimura, "Mersenne Twister: A
          623-dimensionally equidistributed uniform pseudorandom number
          generator". 'ACM Transactions on Modeling and Computer
          Simulation', Vol. 8, No. 1 (Jan. 1998), Pages 3-30

     The generator 'mt19937' uses the second revision of the
     seeding procedure published by the two authors above in 2002.  The
     original seeding procedures could cause spurious artifacts for
     some seed values. They are still available through the alternative
     generators 'mt19937_1999' and 'mt19937_1998'.

**ranlxs0, ranlxs1, ranlxs2**
     The generator 'ranlxs0' is a second-generation version of the
     RANLUX algorithm of Lüscher, which produces "luxury random
     numbers".  This generator provides single precision output (24
     bits) at three luxury levels 'ranlxs0', 'ranlxs1' and 'ranlxs2',
     in increasing order of strength.  It uses double-precision
     floating point arithmetic internally and can be significantly
     faster than the integer version of 'ranlux', particularly on
     64-bit architectures.  The period of the generator is about
     10\ :sup:`171` .  The algorithm has mathematically proven properties and
     can provide truly decorrelated numbers at a known level of
     randomness.  The higher luxury levels provide increased
     decorrelation between samples as an additional safety margin.

**ranlxd1, ranlxd2**
     These generators produce double precision output (48 bits) from the
     RANLXS generator.  The library provides two luxury levels
     'ranlxd1' and 'ranlxd2', in increasing order of strength.

**ranlux, ranlux389**
     The 'ranlux' generator is an implementation of the original
     algorithm developed by Lüscher.  It uses a
     lagged-fibonacci-with-skipping algorithm to produce "luxury random
     numbers".  It is a 24-bit generator, originally designed for
     single-precision IEEE floating point numbers.  This implementation
     is based on integer arithmetic, while the second-generation
     versions RANLXS and RANLXD described above provide floating-point
     implementations which will be faster on many platforms.  The
     period of the generator is about 10\ :sup:`171` .  The algorithm has
     mathematically proven properties and it can provide truly
     decorrelated numbers at a known level of randomness.  The default
     level of decorrelation recommended by Lüscher is provided by
     'ranlux', while 'ranlux389' gives the highest
     level of randomness, with all 24 bits decorrelated.  Both types of
     generator use 24 words of state per generator.

     For more information see,
     
     -  M. Lüscher, "A portable high-quality random number generator
        for lattice field theory calculations", 'Computer Physics
        Communications', 79 (1994) 100-110.

     -  F. James, "RANLUX: A Fortran implementation of the
        high-quality pseudo-random number generator of Lüscher",
        'Computer Physics Communications', 79 (1994) 111-114

**cmrg**
     This is a combined multiple recursive generator by L'Ecuyer.  Its
     sequence is,

     z\ :sub:`n`  = (x\ :sub:`n`  - y\ :sub:`n` ) mod m\ :sub:`1` 

     where the two underlying generators x\ :sub:`n`  and y\ :sub:`n`  are,

     x\ :sub:`n` = (a\ :sub:`1` x\ :sub:`n-1`  + a\ :sub:`2` x\ :sub:`n-2`  + a\ :sub:`3` x\ :sub:`n-3` ) mod m\ :sub:`1`

     y\ :sub:`n` = (b\ :sub:`1` y\ :sub:`n-1`  + b\ :sub:`2` y\ :sub:`n-2`  + b\ :sub:`3` y\ :sub:`n-3` ) mod m\ :sub:`2`

     with coefficients a\ :sub:`1`  = 0, a\ :sub:`2`  = 63308, a\ :sub:`3`  = -183326, b\ :sub:`1`  = 86098,
     b\ :sub:`2`  = 0, b\ :sub:`3`  = -539608, and moduli m\ :sub:`1`  = 2\ :sup:`31`  - 1 = 2147483647 and
     m\ :sub:`2`  = 2145483479.

     The period of this generator is lcm(m\ :sub:`1`\ :sup:`3` -1, m\ :sub:`2`\ :sup:`3` -1), which is
     approximately 2\ :sup:`185`  (about 10\ :sup:`56` ).  It uses 6 words of state per
     generator.  For more information see,

     -    P. L'Ecuyer, "Combined Multiple Recursive Random Number
          Generators", 'Operations Research', 44, 5 (1996), 816-822.

**mrg**
     This is a fifth-order multiple recursive generator by L'Ecuyer,
     Blouin and Coutre.  Its sequence is,

     x\ :sub:`n`  = (a\ :sub:`1`  x\ :sub:`n-1`  + a\ :sub:`5`  x\ :sub:`n-5` )
     mod m

     with a\ :sub:`1`  = 107374182, a\ :sub:`2`  = a\ :sub:`3`  = a\ :sub:`4`  = 0, a\ :sub:`5`  = 104480 and m =
     2\ :sup:`31`  - 1.

     The period of this generator is about 10\ :sup:`46` .  It uses 5 words of
     state per generator.  More information can be found in the
     following paper,

     -    P. L'Ecuyer, F. Blouin, and R. Coutre, "A search for good
          multiple recursive random number generators", 'ACM
          Transactions on Modeling and Computer Simulation' 3, 87-98
          (1993).

**taus, taus2**
     This is a maximally equidistributed combined Tausworthe generator
     by L'Ecuyer.  The sequence is,

     x\ :sub:`n` = (s1\ :sub:`n` ^^ s2\ :sub:`n` ^^ s3\ :sub:`n`)

     where,

     s1\ :sub:`n+1` = (((s1\ :sub:`n` & 4294967294)<<12)^^(((s1\ :sub:`n` <<13)^^s1\ :sub:`n`)>>19))

     s2\ :sub:`n+1` = (((s2\ :sub:`n` & 4294967288)<< 4)^^(((s2\ :sub:`n` << 2)^^s2\ :sub:`n`)>>25))

     s3\ :sub:`n+1` = (((s3\ :sub:`n` & 4294967280)<<17)^^(((s3\ :sub:`n` << 3)^^s3\ :sub:`n`)>>11))

     computed modulo 2\ :sup:`32` .  In the formulas above ^^ denotes
     "exclusive-or".  Note that the algorithm relies on the properties
     of 32-bit unsigned integers and has been implemented using a
     bitmask of '0xFFFFFFFF' to make it work on 64 bit machines.

     The period of this generator is 2\ :sup:`88`  (about 10\ :sup:`26` ).  It uses 3
     words of state per generator.  For more information see,

     -   P. L'Ecuyer, "Maximally Equidistributed Combined Tausworthe
         Generators", 'Mathematics of Computation', 65, 213 (1996),
         203-213.

     The generator 'taus2' uses the same algorithm as
     'taus' but with an improved seeding procedure described in
     the paper,

     -    P. L'Ecuyer, "Tables of Maximally Equidistributed Combined
          LFSR Generators", 'Mathematics of Computation', 68, 225
          (1999), 261-269

     The generator 'taus2' should now be used in preference to
     'taus'.

**gfsr4**
     The 'gfsr4' generator is like a lagged-fibonacci generator, and
     produces each number as an 'xor''d sum of four previous values.

     r\ :sub:`n`  = r\ :sub:`n-A`  ^^ r\ :sub:`n-B`  ^^ r\ :sub:`n-C`  ^^ r\ :sub:`n-D` 

     Ziff (ref below) notes that "it is now widely known" that two-tap
     registers (such as R250, which is described below) have serious
     flaws, the most obvious one being the three-point correlation that
     comes from the definition of the generator.  Nice mathematical
     properties can be derived for GFSR's, and numerics bears out the
     claim that 4-tap GFSR's with appropriately chosen offsets are as
     random as can be measured, using the author's test.

     This implementation uses the values suggested the example on p392
     of Ziff's article: A=471, B=1586, C=6988, D=9689.

     If the offsets are appropriately chosen (such as the one ones in
     this implementation), then the sequence is said to be maximal;
     that means that the period is 2\ :sup:`D` - 1, where D is the longest lag.
     (It is one less than 2\ :sup:`D` because it is not permitted to have all
     zeros in the 'ra[]' array.)  For this implementation with D=9689
     that works out to about 10\ :sup:`2917` .

     Note that the implementation of this generator using a 32-bit
     integer amounts to 32 parallel implementations of one-bit
     generators.  One consequence of this is that the period of this
     32-bit generator is the same as for the one-bit generator.
     Moreover, this independence means that all 32-bit patterns are
     equally likely, and in particular that 0 is an allowed random
     value.  (We are grateful to Heiko Bauke for clarifying for us these
     properties of GFSR random number generators.)

     For more information see,

     -    Robert M. Ziff, "Four-tap shift-register-sequence
          random-number generators", 'Computers in Physics', 12(4),
          Jul/Aug 1998, pp 385-392.


.. [park-miller] Commun. ACM, 31, 1192-1201
