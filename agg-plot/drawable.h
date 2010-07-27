#ifndef AGGPLOT_DRAWABLE_H
#define AGGPLOT_DRAWABLE_H

#include "scalable.h"

#include "agg_trans_affine.h"
#include "agg_conv_transform.h"

// Interface
class window_object {
public:
  virtual void apply_transform(const agg::trans_affine& m) = 0;
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;

  virtual bool dispose() = 0;

  virtual ~window_object() { };
};

class drawable: public vertex_source, public window_object {
public:
  virtual void apply_transform(const agg::trans_affine& m) = 0;
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;

  virtual bool dispose() = 0;

  virtual void rewind(unsigned path_id) = 0;
  virtual unsigned vertex(double* x, double* y) = 0;

  virtual ~drawable() { };
};

/* this class take a "scalable" object and it does transform it to a
   "drawable" by adding a conv_transform taht manage the window size
   transformation. */
class window_scalable : public drawable
{
  static agg::trans_affine dummy_matrix;

  scalable *m_source;
  agg::conv_transform<scalable> m_trans;
  
public:
  window_scalable(scalable *src, agg::trans_affine& mtx = dummy_matrix) : 
    drawable(), m_source(src), m_trans(*m_source, mtx)
  { };

  virtual void rewind(unsigned path_id);
  virtual unsigned vertex(double* x, double* y);

  virtual void apply_transform(const agg::trans_affine& m);
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2);

  virtual bool dispose();
};

#endif
