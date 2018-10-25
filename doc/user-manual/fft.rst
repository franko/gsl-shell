.. highlight:: lua

.. include:: <isogrk1.txt>

.. currentmodule:: num

Fast Fourier Transform
==============================

Mathematical Definitions
------------------------

Fast Fourier Transforms are efficient algorithms for calculating the discrete Fourier transform (DFT)

.. math::
     x_j = \sum_{k=0}^{n-1} z_k \exp(-2\pi i j k / n)

The DFT usually arises as an approximation to the continuous Fourier
transform when functions are sampled at discrete intervals in space or
time. The naive evaluation of the discrete Fourier transform is a matrix-vector multiplication :math:`W\vec{z}`. A general matrix-vector multiplication takes O(n\ :sup:`2`) operations for n data-points. Fast Fourier transform algorithms use a divide-and-conquer strategy to factorize the matrix W into smaller sub-matrices, corresponding to the integer factors of the length n. If n can be factorized into a product of integers f\ :sub:`1` f\ :sub:`2` ... f\ :sub:`m` then the DFT can be computed in O(n |Sgr| f\ :sub:`i`) operations. For a radix-2 FFT this gives an operation count of O(n log\ :sub:`2` n).

All the FFT functions offer two types of transform: forwards and inverse, based on the same mathematical definitions. The definition of the forward Fourier transform, ``fft(z)``, is,

.. math::
     x_j = \sum_{k=0}^{n-1} z_k \exp(-2\pi i j k / n)

and the definition of the inverse Fourier transform is,

.. math::
     z_j = {1 \over n} \sum_{k=0}^{n-1} x_k \exp(2\pi i j k / n).

The factor of 1/n makes this a true inverse.

In general there are two possible choices for the sign of the exponential in the transform/ inverse-transform pair. GSL follows the same convention as FFTW, using a negative exponential for the forward transform.
The advantage of this convention is that the inverse transform recreates the original function with simple Fourier synthesis.
Numerical Recipes uses the opposite convention, a positive exponential in the forward transform.


Methods
------------------------------

The FFT implementation in gsl-shell is an interface to the `FFTW library <http://www.fftw.org/fftw3_doc/>`_ which is well tested and very fast for repetitive transformations.

.. function:: fft(input [, output])

   Performs the Fourier transform of the complex-valued column matrix ``input``.
   The transformed signal is stored in a complex-valued column matrix ``output`` which is allocated by the function each time. To speed up repetitive Fourier transformations it is recommended to give the output column matrix as a second argument to eliminate the allocation overhead. 

.. function:: fftinv(input [, output])

   Performas the inverse Fourier transformation of the complex-valued column matrix ``input``. The output is stored in a complex-valued column matrix which is allocated by the function each time. To speed up repetitive Fourier transformations it is recommended to give the output column matrix as a second argument to eliminate the allocation overhead.

   This transformation is the inverse of the function :func:`num.fft`, so that if you perform the two transformations consecutively you will obtain a vector identical to the initial one up to a factor ``size=#input``.

   A typical usage of :func:`fftinv` is to revert the transformation made with :func:`fft` but by doing some transformations along the way.
   So a typical usage path could be::

      -- we assume v is a column matrix with our data
      ft = num.fft(v) -- Fourier transform

      -- here we can manipulate the half-complex array 'ft'
      -- using the methods `get' and `set'
      {some code here}

      vt = num.fftinv(ft)/#ft -- we perform the inverse Fourier transform
      -- now vt is a vector of the same size of v

.. function:: rfft(input[, output])

    Performs the Fourier transformation of real-valued input and complex-valued output.
    In the output, the Fourier coefficients satisfy the relation

    .. math::
         z_k = z_{N-k}^*

    where :math:`N` is the size of the vector and :math:`k` is any integer number from 0 to :math:`N-1`.
    As a result of this symmetry, half of the output :math:`z` is redundant (being the complex conjugate of the other half), and so the transform only outputs elements :math:`0...\frac{n}{2}` of :math:`z` (:math:`\frac{n}{2}+1` complex numbers), where the division by 2 is rounded down.

.. function:: rfftinv(input[, output])

    Performs the inverse Fourier transformation with complex-valued input and real-valued output. Due to the implementation of the FFTW library, changes in the input matrix can occur which is why it is copied internally.
    For the input of :math:`n`, the output has size :math:`(n-1)\times2`.
    This is the direct inversion of :func:`num.rfft` as see in the example::

        --Forward transformation
        ft = num.rfft(matrix.vec{1,2,3,4})

        --Backward transformation
        orig = num.rfftinv(ft) / #ft

.. function:: fft2(input, [output])
.. function:: fft2inv(input, [output])

    Performs a 2D forward and backward Fourier transformation of an input matrix ``input``. Giving a preallocated output matrix as a second argument speeds up repetitive transformations.

.. function:: rfft2(input, [output])
    
    Performs a 2D forward Fourier transformation with real-valued input matrix ``input``. Returns the complex-valued output with reduced dimension. Giving a preallocated output matrix as a second argument speeds up repetitive transformations.

.. function:: rfft2inv(input, [output])

    Performs the 2D inverse Fourier transformation with complex-valued input matrix ``input``. Due to the implementation of the FFTW library, changes in the input matrix can occur which is why it is copied internally.
    Returns the real-valued output with increased dimension. Giving a preallocated output matrix as a second argument speeds up repetitive transformations.

.. function:: fftn(input, dimlist, [output])
.. function:: fftninv(input, dimlist, [output])

    Performs the n-dimensional forward and backward Fourier transformation of input data ``input``.
    The input is considered to be stored in a `row-major format` meaning that if you had an array with dimensions :math:`n_1 \times n_2 \times n_3` then a point at :math:`(i_1,i_2,i_3)` is stored at index position: :math:`i_3 + n_3\cdot(i_2 + n_2\cdot i_1)`.
    The dimensions are given as a table ``dimlist`` as :math:`{n1, n2, n3, ...}`.

    As before, providing the output array as well limits the overhead of allocating the output array each call.

.. function:: rfftn(input, dimlist, [output])
.. function:: rfftninv(input, dimlist, [output])
    
    Performs the n-dimensional real forward and backward Fourier transformation. Here the size of the input and output arrays are similar to the 1D and 2D couterparts.

    For forward transformations the real-valued input has size :math:`n_1 \times n_2 \times ... \times n_N` and the complex-valued output is of size :math:`n_1 \times n_2 \times ... \times n_N/2+1`.

    For the backward transformation the complex-valued input has size :math:`n_1 \times n_2 \times ... \times n_N/2+1` and the real-valued output has size :math:`n_1 \times n_2 \times ... \times n_N` consequently. Due to the implementation of the FFTW library, changes in the input matrix can occur for backward transformations which is why the input matrix is copied internally.

Examples
-----------

In this example we will treat a square pulse in the temporal domain. To illustrate a typical example of FFT usage we perform the Fourier Transform of the signal and we cut the higher order frequencies. Then we perform the inverse transform and we compare the result with the original time signal.

So, first we define our square pulse in the time domain. Actually it will be a matrix with just one column::


   n, ncut = 256, 16

   -- we create a pulse signal in the time domain
   y = matrix.new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

Then we create two new plots, one for the Fourier transform and one for the signal itself::

   pt = graph.plot('Original signal / reconstructed')

   pt:addline(graph.filine(|i| y[i], 1, n), 'black')

Now we are ready to perform:

  - the Fourier transform
  - cut the higher frequencies
  - transform back the signal in the time domain

and plot the results::

   ft = num.rfft(y)

   pf = graph.fibars(|k| complex.abs(ft[k]), 1, 60)
   pf.title = 'FFT Power Spectrum'

   for k=ncut, #ft do ft[k] = 0 end
   ytr = num.rfftinv(ft)/n

   pt:addline(graph.filine(|i| ytr[i], n), 'red')

   pt:show()

.. figure:: fft-example-power-spectrum.png

   Fourier transform spectrum

.. figure:: fft-example-time-signal.png

   Time signal before (black) and after (red) the transformation

You can observe in the reconstructed signal (the red curve) that we obtain approximately the square pulse, but with a lot of oscillations. Of course this is an artifact of our transformations. The reason is that in order to perfectly reproduce a sharp signal, we also need all the high frequencies of the Fourier transform.
