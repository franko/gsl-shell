#ifndef AGGPLOT_UTILS_H
#define AGGPLOT_UTILS_H

#include <string>
#include "agg_trans_affine.h"

template <class T>
T max (T a, T b) {
  return (b < a) ? a : b;
}

template <class T>
T min (T a, T b) {
  return (b > a) ? a : b;
}

extern void   trans_affine_compose  (agg::trans_affine& a, 
				     const agg::trans_affine& b);

extern agg::trans_affine identity_matrix;

#endif
