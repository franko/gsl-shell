.. highlight:: lua
.. _monte-vegas:

Monte Carlo Integration
=======================

This chapter describes the VEGAS Monte Carlo integration method. 

Introduction
------------

The VEGAS algorithm of Lepage is based on importance sampling. It samples points from the probability distribution described by the function :math:`|f|`, so that the points are concentrated in the regions that make the largest contribution to the integral.

In general, if the Monte Carlo integral of :math:`f` is sampled with points distributed according to a probability distribution described by the function :math:`g`, we obtain an estimate :math:`E_g(f; N)`,

.. math::
      E_g(f; N) = E(f/g; N)

with a corresponding variance,

.. math::
     \textrm{Var}_g(f; N) = \textrm{Var}(f/g; N).

If the probability distribution is chosen as :math:`g = |f|/\int(|f|)` then it can be shown that the variance `\textrm{Var}_g(f; N)` vanishes, and the error in the estimate will be zero. In practice it is not possible to sample from the exact distribution g for an arbitrary function, so importance sampling algorithms aim to produce efficient approximations to the desired distribution.
The VEGAS algorithm uses a fixed number of calls to evaluate the integral. 
The VEGAS algorithm approximates the exact distribution by making a number of passes over the integration region while histogramming the function :math:`f`. Each histogram is used to define a sampling distribution for the next pass. Asymptotically this procedure converges to the desired distribution. In order to avoid the number of histogram bins growing like :math:`K^d` the probability distribution is approximated by a separable function: :math:`g(x_1, x_2, ...) = g_1(x_1) g_2(x_2) ...` so that the number of bins required is only :math:`Kd`. This is equivalent to locating the peaks of the function from the projections of the integrand onto the coordinate axes. The efficiency of VEGAS depends on the validity of this assumption. It is most efficient when the peaks of the integrand are well-localized. If an integrand can be rewritten in a form which is approximately separable this will increase the efficiency of integration with VEGAS.

VEGAS incorporates a number of additional features, and combines both stratified sampling and importance sampling. The integration region is divided into a number of “boxes”, with each box getting a fixed number of points (the goal is 2). Each box can then have a fractional number of bins, but if the ratio of bins-per-box is less than two, VEGAS switches to a kind variance reduction (rather than importance sampling).

Errors and consistency
----------------------

The VEGAS algorithm computes a number of independent estimates of the integral internally, and returns their weighted average. Random sampling of the integrand can occasionally produce an estimate where the error is zero, particularly if the function is constant in some regions. An estimate with zero error causes the weighted average to break down and must be handled separately. In the original FORTRAN implementations of VEGAS, the error estimate is made non-zero by substituting a small value (typically 1e-30). The implementation in GSL differs from this and avoids the use of an arbitrary constant -- it either assigns the value a weight which is the average weight of the preceding estimates or discards it according to the following procedure,

* current estimate has zero error, weighted average has finite error

    The current estimate is assigned a weight which is the average weight of the preceding estimates.
* current estimate has finite error, previous estimates had zero error

    The previous estimates are discarded and the weighted averaging procedure begins with the current estimate. 
* current estimate has zero error, previous estimates had zero error
    
    The estimates are averaged using the arithmetic mean, but no error is computed.

The convergence of the algorithm can be tested using the overall chi-squared value of the results. A value which differs significantly from 1 indicates that the values from different iterations are inconsistent. In this case the weighted error will be underestimated, and further iterations of the algorithm are needed to obtain reliable results.

The VEGAS algorithm uses a fixed number of calls to evaluate the integral. It is possible to call the continuation function, which is returned by :func:`num.monte_vegas`, with a higher number of calls to increase the accuracy of the result. Keep in mind that reducing :math:`\sigma` by a certain factor typically increases the number of calls quadratically, because :math:`\sigma \propto 1/\sqrt{n}`.

Functions
---------

.. module:: num

.. function:: monte_vegas(f, a, b[, calls, r, chi_dev])

   Use the VEGAS Monte Carlo algorithm to integrate the function ``f`` over the dim-dimensional hypercubic region defined by the lower and upper limits in the vectors ``a`` and ``b``. The integration uses a fixed number of function calls ``calls``, and obtains random sampling points using the :mod:`rng` random number generator ``r``. The results of the integration are based on a weighted average of five independent samples. ``chi_dev`` is the tolerated deviation from 1 of the chi-squared per degree of freedom for the weighted average. This quantity must be consistent with 1 for the weighted average to be reliable.
   The function returns the result of the integration, the error estimate and the number of runs needed to reach the desired chi-squared. The fourth return value is a continuation function that takes a number of calls as an argument. This function can be invoked to recalculate the integral with a higher number of calls, to increase precision. 
   The continuation function returns the new result, error and number of runs. Note that this function discards the previous results, but retains the optimized grid. Typically the continuation function is called with a multiple of the original number of calls, to improve the error.
  
Usage example
-------------

The subject of statistical physics features many intractable multiple integrals. One example of such an integral is the partition function of a real (interacting) gas. The partition function is an essential quantity in statistical physics, from which other quantities such as the free energy and the pressure can be derived. In the case of an ideal (non-interacting) gas, the partition function factorizes to a product of single integrals. In a real gas, the interactions introduce correlations between the particles, and the multiple integral no longer factorizes. The partition function for a system with a temperature :math:`T`, a volume :math:`V` and a number of particles :math:`N` assumes the following form:

.. math::
   \mathcal{Z}(T,V,N) &= \frac{1}{N! h^{3N}} \int dp_1 dp_2 ... dp_N \int dr_1 dr_2 ... dr_N exp(-E/kT) \\
                      &= \frac{1}{N! h^{3N}} \int \exp( -\frac{1}{kT} \frac{1}{2m} (p_1^2 + p_2^2 + ... + p_N^2)) dp_1 dp_2 ... dp_N \\
                      &\times \int \exp( -\frac{1}{kT} \frac{1}{2}\sum_{i,j}^{N} U(r_i,r_j)) dr_1 dr_2 ... dr_N. 

The integral over the momenta :math:`p_i` factorizes, but the presence of the potential :math:`U(r_i,r_j)` prevents the integral over the coordinates to be written as a product of single integrals. This integral is called the configurational partition function: 

.. math::
   Q(T,V,N) = \frac{1}{V^N} \int \exp( -\frac{1}{kT} \sum_{i>j}^{N} U(r_i,r_j)) dr_1 dr_2 ... dr_N

so that :math:`\mathcal{Z}(T,V,N) = \mathcal{Z}_{\textrm{ideal}}(T,V,N) \times Q(T,V,N)`. 

Using the VEGAS algorithm, we can perform a naive calculation of :math:`Q(T,V,N)` for a one-dimensional box containing 5 particles with a Gaussian repulsive interaction::

  local exp = math.exp
  local T,V,N = 2,100,5

  -- the potential between two particles
  local U = |r1,r2| 0.1*exp(-(r1-r2)^2/2)

  -- the Boltzmann factor exp(-E/kT)
  local function boltzmann(p)
    local Epot = 0
    for i=1,N do
      for j=1,i-1 do -- i>j avoids counting pairs twice
        Epot = Epot + U(p[i],p[j])
      end
    end
    return exp(-Epot/T)
  end

  -- set the lower and upper boundaries
  local lo, hi = {},{}
  for i=1,N do lo[i],hi[i] = 0,V end

  -- calculate the integral and print the results
  local res,sig,num,cont = num.monte_vegas(boltzmann,lo,hi,1e5)
  io.write("Q(T=",T,",V=",V,",N=",N,") = ",res/V^N," +/- ",sig/V^N,"\n")

