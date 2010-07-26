#ifndef VERTEX_SOURCE_H
#define VERTEX_SOURCE_H

class vertex_source {
public:
  virtual void rewind(unsigned path_id) = 0;
  virtual unsigned vertex(double* x, double* y) = 0;

  virtual ~vertex_source() { };
};

class scalable_object : public vertex_source {
public:
  virtual void rewind(unsigned path_id) = 0;
  virtual unsigned vertex(double* x, double* y) = 0;

  virtual void approximation_scale(double as) = 0;
  virtual bool dispose() = 0;

  virtual ~scalable_object() { };
}


  /*
  virtual void apply_transform(const agg::trans_affine& m, double as) = 0;
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;
  virtual bool dispose() { return false; };
  virtual bool need_resize() { return true; };
  */

#endif
