.. highlight:: lua

Complex Numbers
===============

The functions described in this chapter provide support for complex numbers.
The algorithms take care to avoid unnecessary intermediate underflows and overflows, allowing the functions to be evaluated over as much of the complex plane as possible.

For multiple-valued functions the branch cuts have been chosen to follow the conventions of Abramowitz and Stegun in the 'Handbook of Mathematical Functions'.
The functions return principal values which are the same as those in GNU Calc, which in turn are the same as those in 'Common Lisp, The Language (Second Edition)' and the HP-28/48 series of calculators.

Complex Functions
-----------------

.. module:: complex

In this section we describe the mathematical functions on complex numbers.

.. note::
   Please note that the standard functions from the :mod:`math` does *not* accept complex numbers.

Elementary Complex Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: sqrt(z)

   This function returns the square root of the complex number z,
   :math:`\sqrt{z}`.
   The branch cut is the negative real axis.
   The result always lies in the right half of the complex plane.

.. function:: exp(z)

   This function returns the complex exponential of the complex number z, :math:`\exp(z)`.

.. function:: log(z)

   This function returns the complex natural logarithm (base e) of the complex number z, :math:`\log(z)`.
   The branch cut is the negative real axis.

.. function:: log10(z)

   This function returns the complex base-10 logarithm of the complex number z, :math:`\log_{10}(z)`.

.. function:: pow(z, a)

   The function returns the complex number z raised to the complex power a, :math:`z^a`.
   This is computed as :math:`\exp(a \, \log(z))` using complex logarithms and complex exponentials.

Complex Trigonometric Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: sin(z)

   This function returns the complex sine of the complex number z,

   .. math::
      \sin(z) = \frac{e^{i \, z} - e^{-i\, z}}{2 \, i}

.. function:: cos(z)

   This function returns the complex cosine of the complex number z,

   .. math::
      \cos(z) = \frac{e^{i \, z} + e^{-i\, z}}{2}

.. function:: tan(z)

   This function returns the complex tangent of the complex number z, :math:`\tan(z) = \sin(z)/\cos(z)`.

.. function:: sec(z)

   This function returns the complex secant of the complex number z, :math:`\sec(z) = 1/\cos(z)`.

.. function:: csc(z)

   This function returns the complex cosecant of the complex number z, :math:`\csc(z) = 1/\sin(z)`.

.. function:: cot(z)

   This function returns the complex cotangent of the complex number z, :math:`\cot(z) = 1/\tan(z)`.

Inverse Complex Trigonometric Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: asin(z)

   This function returns the complex arcsine of the complex number z, :math:`\arcsin(z)`.
   The branch cuts are on the real axis, less than -1 and greater than 1.

.. function:: acos(z)

    This function returns the complex arccosine of the complex number z, :math:`\arccos(z)`.
    The branch cuts are on the real axis, less than -1 and greater than 1.

.. function:: atan(z)

   This function returns the complex arctangent of the complex number z, :math:`\arctan(z)`.
   The branch cuts are on the imaginary axis, below -i and above i.

.. function:: asec(z)

   This function returns the complex arcsecant of the complex number z, :math:`\textrm{asec}(z) = \textrm{acos}(1/z)`.

.. function:: acsc(z)

   This function returns the complex arccosecant of the complex number z, :math:`\textrm{acsc}(z) = \textrm{asin}(1/z)`.

.. function:: acot(z)

   This function returns the complex arccotangent of the complex number z, :math:`\textrm{acot}(z) = \textrm{atan}(1/z)`.

Complex Hyperbolic Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: sinh(z)

   This function returns the complex hyperbolic sine of the complex
     number z, :math:`\sinh(z) = (\exp(z) - \exp(-z))/2`.

.. function:: cosh(z)

   This function returns the complex hyperbolic cosine of the complex
     number z, :math:`\cosh(z) = (\exp(z) + \exp(-z))/2`.

.. function:: tanh(z)

   This function returns the complex hyperbolic tangent of the
     complex number z, :math:`\tanh(z) = \sinh(z)/\cosh(z)`.

.. function:: sech(z)

   This function returns the complex hyperbolic secant of the complex
     number z, :math:`\textrm{sech}(z) = 1/\cosh(z)`.

.. function:: csch(z)

   This function returns the complex hyperbolic cosecant of the
     complex number z, :math:`\textrm{csch}(z) = 1/\sinh(z)`.

.. function:: coth(z)

   This function returns the complex hyperbolic cotangent of the
     complex number z, :math:`\coth(z) = 1/\tanh(z)`.

Inverse Complex Hyperbolic Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: asinh(z)

   This function returns the complex hyperbolic arcsine of the complex number z, :math:`\textrm{asinh}(z)`.
   The branch cuts are on the imaginary axis, below -i and above i.

.. function:: acosh(z)

   This function returns the complex hyperbolic arccosine of the complex number z, :math:`\textrm{acosh}(z)`.
   The branch cut is on the real axis, less than 1.
   Note that in this case we use the negative square root in formula 4.6.21 of Abramowitz & Stegun giving :math:`\textrm{acosh}(z)=\log(z-\sqrt{z^2-1})`.

.. function:: real(z)

   This function returns the complex hyperbolic arccosine of the real number z, :math:`\textrm{acosh}(z)`.

.. function:: atanh(z)

   This function returns the complex hyperbolic arctangent of the complex number z, :math:`\textrm{atanh}(z)`.
   The branch cuts are on the real axis, less than -1 and greater than 1.

.. function:: asech(z)

   This function returns the complex hyperbolic arcsecant of the complex number z, :math:`\textrm{asech}(z) = \textrm{acosh}(1/z)`.

.. function:: acsch(z)

   This function returns the complex hyperbolic arccosecant of the complex number z, :math:`\textrm{acsch}(z) = \textrm{asin}(1/z)`.

.. function:: acoth(z)

   This function returns the complex hyperbolic arccotangent of the complex number z, :math:`\textrm{acoth}(z) = \textrm{atanh}(1/z)`.
