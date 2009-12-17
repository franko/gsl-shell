#ifndef VERTEX_SOURCE_H
#define VERTEX_SOURCE_H

#include "agg_trans_affine.h"

class vertex_source_base
{
public:
  virtual void rewind(unsigned path_id) = 0;
  virtual unsigned vertex(double* x, double* y) = 0;
  virtual ~vertex_source_base() {};
};

class vertex_source : public vertex_source_base {
public:
  virtual void apply_transform(agg::trans_affine& m, double as) = 0;
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;

  virtual void ref() = 0;
  virtual unsigned unref() = 0;

  virtual bool need_resize() { return true; };
};

#endif
