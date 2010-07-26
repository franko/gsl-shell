#ifndef GRAPHICAL_OBJECT_H
#define GRAPHICAL_OBJECT_H

#include "agg_trans_affine.h"

class graphical_object : public vertex_source {
public:
  virtual void apply_transform(const agg::trans_affine& m, double as) = 0;
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;
  virtual bool dispose() { return false; };
  virtual bool need_resize() { return true; };
};

#endif
