#ifndef AGGPLOT_CPLOT_H
#define AGGPLOT_CPLOT_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "vertex-source.h"

#include "drawables.h"
#include "canvas.h"
#include "units.h"

#include "agg_conv_transform.h"
#include "agg_color_rgba.h"
#include "agg_path_storage.h"
#include "agg_ellipse.h"
#include "agg_array.h"

struct drawable {
  vertex_source* vs;
  agg::rgba8 color;

  drawable(): vs(NULL), color() {};
  drawable(vertex_source* vs, agg::rgba8 c): vs(vs), color(c) {};

  ~drawable() {};

  void bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    vertex_source& vsi = get_vertex_source();
    vsi.bounding_box(x1, y1, x2, y2);
  };

  vertex_source& get_vertex_source() { return *vs; };

/*
  void draw(canvas& canvas, agg::trans_affine& t)
  {
    vs_type& vsi = get_vertex_source();
    agg::conv_transform<vs_type> p(vsi, t);
    canvas.draw(p, color);
  };
*/
};

class plot {
public:
  plot() : m_elements(), m_trans(), m_bbox_updated(true) { };
  virtual ~plot() {};

  void add(vertex_source* vs, agg::rgba8 color) 
  { 
    drawable d(vs, color);
    m_elements.add(d);
    m_bbox_updated = false;
  };

  virtual void draw(canvas &canvas)
  {
    trans_matrix_update();
    draw_elements(canvas);
  };

protected:
  void draw_elements(canvas &canvas);
  void bounding_box(double *x1, double *y1, double *x2, double *y2);
  virtual void trans_matrix_update();

  static void viewport_scale(agg::trans_affine& trans);

  agg::pod_bvector<drawable> m_elements;
  agg::trans_affine m_trans;

  // bounding box
  bool m_bbox_updated;
};

#endif
