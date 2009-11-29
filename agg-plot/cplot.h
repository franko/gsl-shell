#ifndef AGGPLOT_CPLOT_H
#define AGGPLOT_CPLOT_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "canvas.h"
#include "units.h"

#include "agg_conv_transform.h"
#include "agg_color_rgba.h"
#include "agg_path_storage.h"
#include "agg_array.h"

struct drawable {
  virtual void draw(canvas& can, agg::trans_affine& t) = 0;
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;
  virtual ~drawable() {};
};

struct line : public drawable {
  agg::path_storage path;
  agg::rgba8 color;

  line(agg::rgba8 col): path(), color(col) {};

  virtual void draw(canvas& can, agg::trans_affine& t);
  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2);
};

struct dashed_line : public line {
public:
  dashed_line(double l1, double l2, agg::rgba8 col): 
    line(col), m_l1(l1), m_l2(l2) 
  {};

  virtual void draw(canvas& can, agg::trans_affine& t);

private:
  double m_l1;
  double m_l2;
};

struct polygon : public line {
  polygon(agg::rgba8 col): line(col) {};
  virtual void draw(canvas& can, agg::trans_affine& t);
};

struct poly_outline : public line {
  agg::rgba8 m_outline_color;

  poly_outline(agg::rgba8 col, agg::rgba8 out_col) : 
    line(col), m_outline_color(out_col)
  {};

  virtual void draw(canvas& can, agg::trans_affine& t);
};

class cplot {
  typedef agg::pod_bvector<drawable *> container_type;

public:

  cplot() : m_lines(), m_trans(), m_bbox_set(false) {};

  virtual ~cplot() 
  {
    for (unsigned j = 0; j < m_lines.size(); j++)
      delete m_lines[j];
  };

  void add(drawable *d);

  virtual void draw(canvas &canvas);

  const agg::trans_affine& trans() const { return m_trans; };

protected:
  void draw_lines(canvas &canvas);
  bool bbox_update();
  virtual bool update();

  static void viewport_scale(agg::trans_affine& trans);

  container_type m_lines;
  agg::trans_affine m_trans;

  // bounding box
  bool m_bbox_set;
  double m_x1, m_y1;
  double m_x2, m_y2;
};

#endif
