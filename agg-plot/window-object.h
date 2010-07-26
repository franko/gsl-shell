#include "vertex-source.h"

#include "agg_conv_transform.h"

class window_object : public vertex_source {
public:
  virtual void rewind(unsigned path_id) = 0;
  virtual unsigned vertex(double* x, double* y) = 0;

  virtual void apply_transform(const agg::trans_affine& m) = 0;
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;

  virtual ~window_object() { };
};

class win_scalable_object : public window_object {
  scalable_object *m_source;
  agg::conv_transform<scalable_object> m_trans;
  
public:
  win_scalable_object(agg::trans_affine& mtx, scalable_object *src) : 
    window_object(), m_source(src), m_trans(*m_source, mtx)
  { };

  virtual void apply_transform(const agg::trans_affine& m);
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2);
};
