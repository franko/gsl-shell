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

extern void   trans_affine_compose  (agg::trans_affine& a, const agg::trans_affine& b);
extern double trans_affine_max_norm (const agg::trans_affine& m);

extern void    bbox_enlarge(double *x1, double* y1, double* x2, double* y2, 
			    double x, double y);

#endif
