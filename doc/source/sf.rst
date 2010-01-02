GSH_SF_MODE(D, airy_Ai, airyAi)
GSH_SF_MODE(D, airy_Bi, airyBi)
GSH_SF_ZERO_COMP(U, airy, Ai)

GSH_SF_ZERO_COMP(U, airy, Bi)

GSH_SF_COMP(D,  bessel, J0)
GSH_SF_COMP(D,  bessel, J1)
GSH_SF_COMP(ID, bessel, Jn)
GSH_SF_COMP(D,  bessel, Y0)
GSH_SF_COMP(D,  bessel, Y1)
GSH_SF_COMP(ID, bessel, Yn)
GSH_SF_COMP(D,  bessel, I0)
GSH_SF_COMP(D,  bessel, I1)
GSH_SF_COMP(ID, bessel, In)
GSH_SF_COMP(D,  bessel, K0)
GSH_SF_COMP(D,  bessel, K1)
GSH_SF_COMP(ID, bessel, Kn)
GSH_SF_ZERO_COMP(U,  bessel, J0)
GSH_SF_ZERO_COMP(U,  bessel, J1)
GSH_SF_ZERO_COMP(DU, bessel, Jnu)

GSH_SF(D, clausen)
GSH_SF(D, dawson)

GSH_SF_CUSTOM(laguerre)
GSH_SF_CUSTOM(debye)
GSH_SF_CUSTOM(fermi_dirac)

GSH_SF(D, dilog)

GSH_SF(D, erf)

GSH_SF_COMP(D,expint, E1)
GSH_SF_COMP(D,expint, E2)
GSH_SF_COMP(ID,expint, En)
GSH_SF_COMP(D,expint, Ei)
GSH_SF_COMP(D,expint, 3)

GSH_SF(D, Shi)
GSH_SF(D, Chi)
GSH_SF(D, Si)
GSH_SF(D, Ci)
GSH_SF(D, atanint)

GSH_SF(U, fact)
GSH_SF(U, doublefact)
GSH_SF(U, lnfact)
GSH_SF(U, lndoublefact)
GSH_SF(UU, choose)
GSH_SF(UU, lnchoose)

GSH_SF(D, gamma)
GSH_SF(D, lngamma)
GSH_SF(DD, beta)
GSH_SF(DD, lnbeta)

GSH_SF_COMP(D,legendre, P1)
GSH_SF_COMP(D,legendre, P2)
GSH_SF_COMP(D,legendre, P3)
GSH_SF_COMP(ID,legendre, Pl)
GSH_SF_COMP(D,legendre, Q0)
GSH_SF_COMP(D,legendre, Q1)
GSH_SF_COMP(ID,legendre, Ql)

GSH_SF_COMP(DD, hyperg, 0F1)
GSH_SF_CUSTOM(hyperg1F1)
GSH_SF_CUSTOM(hypergU)
// GSH_SF_IID_SUFFIX(hyperg, 1F1, int)
/* GSH_SF_COMP(DDD,hyperg, 1F1) */
// GSH_SF_IID_SUFFIX(hyperg, U, int)
/* GSH_SF_COMP(DDD,hyperg, U) */

/* GSH_SF_I_SIMPLE_SUFFIX(zeta, int) */
GSH_SF(D, zeta)
