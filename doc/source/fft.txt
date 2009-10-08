.. highlight:: lua

.. include:: <isogrk1.txt>

Radix-2 Fast Fourier Transform
==============================

Mathematical Definitions
------------------------

Fast Fourier Transforms are efficient algorithms for calculating the discrete fourier transform (DFT)

.. math::
     x_j = \sum_{k=0}^{n-1} z_k \exp(-2\pi i j k / n)

The DFT usually arises as an approximation to the continuous fourier
transform when functions are sampled at discrete intervals in space or
time. The naive evaluation of the discrete fourier transform is a matrix-vector multiplication :math:`W\vec{z}`. A general matrix-vector multiplication takes O(n^2) operations for n data-points. Fast fourier transform algorithms use a divide-and-conquer strategy to factorize the matrix W into smaller sub-matrices, corresponding to the integer factors of the length n. If n can be factorized into a product of integers f\ :sub:`1` f\ :sub:`2` ... f\ :sub:`m` then the DFT can be computed in O(n |Sgr| f\ :sub:`i`) operations. For a radix-2 FFT this gives an operation count of O(n log\ :sub:`2` n).

All the FFT functions offer three types of transform: forwards, inverse and backwards, based on the same mathematical definitions. The definition of the forward fourier transform, x = FFT(z), is,

.. math::
     x_j = \sum_{k=0}^{n-1} z_k \exp(-2\pi i j k / n)

and the definition of the inverse fourier transform, x = IFFT(z), is,

.. math::
     z_j = {1 \over n} \sum_{k=0}^{n-1} x_k \exp(2\pi i j k / n).

The factor of 1/n makes this a true inverse. For example, a call to gsl_fft_complex_forward followed by a call to gsl_fft_complex_inverse should return the original data (within numerical errors).

In general there are two possible choices for the sign of the exponential in the transform/ inverse-transform pair. GSL follows the same convention as fftpack, using a negative exponential for the forward transform. The advantage of this convention is that the inverse transform recreates the original function with simple fourier synthesis. Numerical Recipes uses the opposite convention, a positive exponential in the forward transform.

The backwards FFT is simply our terminology for an unscaled version of the inverse FFT,

.. math::
     z^{\textrm{backwards}}_j = \sum_{k=0}^{n-1} x_k \exp(2\pi i j k / n).

When the overall scale of the result is unimportant it is often convenient to use the backwards FFT instead of the inverse to save unnecessary divisions.
