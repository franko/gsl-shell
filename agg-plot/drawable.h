#ifndef AGGPLOT_DRAWABLE_H
#define AGGPLOT_DRAWABLE_H

#include "scalable.h"
#include "utils.h"

#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_bounding_rect.h"

struct drawable : public vertex_source {
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;
  virtual ~drawable() { };
};

class boxed_drawable : public drawable {
  drawable *m_object;

 public:
  boxed_drawable(drawable *p) : drawable(), m_object(p) {};

  ~boxed_drawable() { };

  virtual void rewind(unsigned path_id) { m_object->rewind(path_id); };
  virtual unsigned vertex(double* x, double* y) { return m_object->vertex(x, y); };
  virtual void apply_transform(const agg::trans_affine& m, double as) { m_object->apply_transform(m, as); };

  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    m_object->bounding_box(x1, y1, x2, y2);
  };

 private:
  boxed_drawable();
};

/* this class take a "scalable" object and it does transform it to a
   "drawable" by adding a conv_transform that manage the window size
   transformation. */
class window_scalable : public drawable
{
  scalable *m_source;
  agg::conv_transform<scalable> m_trans;
  
public:
  window_scalable(scalable *src, agg::trans_affine& mtx = identity_matrix) : 
    drawable(), m_source(src), m_trans(*m_source, mtx)
  { };

  ~window_scalable() { delete m_source; };

  virtual void rewind(unsigned path_id) { m_trans.rewind(path_id); };

  virtual unsigned vertex(double* x, double* y) { return m_trans.vertex(x, y); };

  virtual void apply_transform(const agg::trans_affine& m, double as)
  {
    m_trans.transformer(m);
    m_source->apply_transform (m, as * m.scale());
  };

  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    agg::bounding_rect_single (*m_source, 0, x1, y1, x2, y2);
  };
};

template<class conv_type>
class drawable_adapter : public vs_adapter<conv_type, drawable> {
  typedef vs_adapter<conv_type, drawable> root_type;

public:
  drawable_adapter(drawable *src) : root_type(src) { };

  template <class init_type>
  drawable_adapter(drawable* src, init_type& val): root_type(src, val) {};

  virtual void apply_transform(const agg::trans_affine& m, double as)
  {
    this->m_source->apply_transform(m, as);
  };

  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    this->m_source->bounding_box(x1, y1, x2, y2);
  };
};

#endif
