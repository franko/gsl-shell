.. highlight:: lua

.. include:: <isogrk1.txt>

.. currentmodule:: num

Fast Fourier Transform
==============================

Mathematical Definitions
------------------------

Fast Fourier Transforms are efficient algorithms for calculating the discrete fourier transform (DFT)

.. math::
     x_j = \sum_{k=0}^{n-1} z_k \exp(-2\pi i j k / n)

The DFT usually arises as an approximation to the continuous fourier
transform when functions are sampled at discrete intervals in space or
time. The naive evaluation of the discrete fourier transform is a matrix-vector multiplication :math:`W\vec{z}`. A general matrix-vector multiplication takes O(n\ :sup:`2`) operations for n data-points. Fast fourier transform algorithms use a divide-and-conquer strategy to factorize the matrix W into smaller sub-matrices, corresponding to the integer factors of the length n. If n can be factorized into a product of integers f\ :sub:`1` f\ :sub:`2` ... f\ :sub:`m` then the DFT can be computed in O(n |Sgr| f\ :sub:`i`) operations. For a radix-2 FFT this gives an operation count of O(n log\ :sub:`2` n).

All the FFT functions offer two types of transform: forwards and inverse, based on the same mathematical definitions. The definition of the forward fourier transform, ``fft(z)``, is,

.. math::
     x_j = \sum_{k=0}^{n-1} z_k \exp(-2\pi i j k / n)

and the definition of the inverse fourier transform is,

.. math::
     z_j = {1 \over n} \sum_{k=0}^{n-1} x_k \exp(2\pi i j k / n).

The factor of 1/n makes this a true inverse.

In general there are two possible choices for the sign of the exponential in the transform/ inverse-transform pair.
GSL follows the same convention as fftpack, using a negative exponential for the forward transform.
The advantage of this convention is that the inverse transform recreates the original function with simple fourier synthesis.
Numerical Recipes uses the opposite convention, a positive exponential in the forward transform.

GSL Shell interface
-------------------

GSL Shell provide a simple interface to perform fourier transform of real data with the functions :func:`num.fft` and :func:`num.fftinv`.
The first function perform the Fourier transform of a column matrix and the second is the inverse Fourier transform.

The function :func:`num.fft` returns an half-complex array.
This latter is similar to a column matrix of complex numbers but it is actually a different object because the numbers are packed together following some specific rules related to the algorithm.

The idea is that you can access to the element of this vector for reading or writing by using a simple indexing.
You can also obtain the size of the vector using the operator '#'.
The valid indexes for an half complex object range from 0 to N-1 where N is the size if the vector.
Each element of the vector correspond to the coefficient :math:`z_k` defined above.

When performing fourier transform it is important to know that the computation speed can be greatly influenced by the size of the vector. In the size is a power of two then a very efficient algorithm can be used and we can talk in this case of Fast Fourier Transform (FFT). In addition the algorithm has the advantage that it does not require any additional workspace. When the size of the vector is not a power of two we can have two different cases:

 * the size if a product of small prime numbers
 * the size contain a big (> 7) prime number in its factorization

This detail is important because when the size is a product of small prime numbers a fast algorithm is still available but it is still somewhat slower and it does require some additional workspace.
In the worst case when the size cannot be factorized to small prime numbers the Fourier transform can still be computed but the calculation is slower especially for large array.

GSL Shell hide all the details and take cares of choosing the appropriate algorithm based on the size of the vector.
It does also provide transparently any additional workspace that may be needed for the algorithm.
In order to avoid repeated allocation of workspace memory the workspace allocated is kept in memory and reused *if the size of the array does not change*.
This mean that the approach of GSL Shell is quite optimal if you perform many times fourier transforms (direct or inverse) of the same size.

Even if GSL Shell take cares of the details automatically you should be aware of these performance notice because it can make a big difference in real applications.
From the practial point of view it is useful in most of the case to always provides samples whose size is a power of two.

Another specificity of the functions :func:`num.fft` and :func:`num.fftinv` is that they can optionally perform the transformation *in place* by modifying the original data instead of creating a copy.
When a trasformation *in place* is requested the routine still return a new vector (either a real matrix or an half-complex array) but this latter will point to the same underlying data of the original vector.
The transformation *in place* can be useful in same cases to avoid unnecessary data compying and memory allocation.


Fourier Transform of Real Data
------------------------------

For real data the fourier coefficients satisfies the relation

.. math::
     z_k = z_{N-k}^*

where N is the size of the vector and k is any integer number from 0 to N-1.
Because of this relation the data is packed in a special type of object called an half-complex array.

To access element in half-complex array you can use the indexing with an integer number between 0 and N-1, inclusive. So, for example::

   -- get a random number generator
   r = rng.new()

   -- create a vector with random numbers
   x = matrix.new(256, 1, || rnd.gaussian(r, 1))

   -- take the fourier transform
   ft = num.fft(x)

   -- print all the coefficients of the fourier transform
   for k=0, #ft-1 do print(ft[k]) end

As shown in the example above you can use the Lua operator '#' to obtain the size of an half-complex array.

.. function:: fft(v[, in_place])

   Perform the Fourier transform of the real valued column matrix ``x``.
   If ``in_place`` is ``true`` then the original data is altered and the resulting vector will point to the same underlying data of the original vector.

   Please note that the value you obtain is not an ordinary matrix but an half-complex array.
   You can access the element of such kind of array by indexing the vector.
   If you want to have an ordinary matrix you can easily build it with the following instructions::

      -- we suppose that f is an half-complex array
      m = matrix.cnew(#f, 1, |i,j| f[i-1])

.. function:: fftinv(hc[, in_place])

   Return a column matrix that contains then inverse Fourier transform of the half-complex vector ``hc``.
   If ``in_place`` is ``true`` then the original data is altered and the resulting vector will point to the same underlying data of the original vector.

   This trasformation is the inverse of the function :func:`num.fft` so that if you perform the two trasformations consecutively you will obtain a vector identical to the initial one.

   A tipical usage of :func:`fft_inv` is to revert the trasformation made with :func:`fft` but by doing some transformations of the way.
   So a typical usage path could be::

      -- we assume v is a column matrix with our data
      ft = num.fft(v) -- fourier transform

      -- here we can manipulate the half-complex array 'ft' with
      -- using the methods `get' and `set'
      some code here

      vt = num.fftinv(ft) -- we perform the inverse fourier transform
      -- now vt is a vector of the same size of v

FFT example
-----------

In this example we will treat a square pulse in the temporal domain. To illustrate a typical example of FFT usage we perform the Fourier Transform of the signal and we cut the higher order frequencies. Than we perform the inverse transform and we compare the result with the original time signal.

So, first we define our square pulse in the time domain. Actually it will be a matrix with just one column::


   n, ncut = 256, 16

   -- we create a pulse signal in the time domain
   y = matrix.new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

Than we create two new plots, one for the Fourier transform and one for the signal itself::

   pt = graph.plot('Original signal / reconstructed')

   pt:addline(graph.filine(|i| y[i], 1, n), 'black')

Now we are ready to perform:

  - the Fourier transform
  - cut the higher frequencies
  - transform back the signal in the time domain

and plot the results::

   ft = num.fft(y)

   pf = graph.fibars(|k| complex.abs(ft[k]), 0, 60)
   pf.title = 'FFT Power Spectrum'

   for k=ncut, n/2 do ft[k] = 0 end
   ytr = num.fftinv(ft)

   pt:addline(graph.filine(|i| ytr[i], n), 'red')

   pt:show()

.. figure:: fft-example-power-spectrum.png

   Fourier transform spectrum

.. figure:: fft-example-time-signal.png

   Time signal before (black) and after (red) the transformation

You can observe in the reconstructed signal (the red curve) that we obtain approximatively the square pulse but with a lot of oscillations. Of course this is an artifact of our transformations. The reason is that in order to reproduce prefectly a sharp signal we need also all the high frequencies of the Fourier transform.
