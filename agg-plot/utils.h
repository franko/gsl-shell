#ifndef AGGPLOT_UTILS_H
#define AGGPLOT_UTILS_H

#include "agg_trans_affine.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


template <typename T>
T min(T a, T b)
{
  return (a < b) ? a : b;
};

template <typename T>
T max(T a, T b)
{
  return (a > b) ? a : b;
};

extern void   trans_affine_compose  (agg::trans_affine& a,
				     const agg::trans_affine& b);

extern agg::trans_affine identity_matrix;

#endif
