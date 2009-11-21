#ifndef AGGPLOT_CPLOT_H
#define AGGPLOT_CPLOT_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <list>

#include "canvas.h"
#include "units.h"

#include "agg_conv_transform.h"
#include "agg_color_rgba.h"
#include "agg_path_storage.h"

class line {
public:
  line(agg::rgba8 c) : path(), color(c) {};

  agg::path_storage path;
  agg::rgba8 color;
};

class cplot {
public:
  line& new_line(agg::rgba8 color);
  
  const agg::trans_affine& trans() const { return m_trans; };

  virtual void draw(canvas &canvas);

  cplot() : m_lines(), m_trans(), m_bbox_set(false) {};

protected:
  void draw_lines(canvas &canvas);
  bool bbox_update();
  virtual bool update();

  static void viewport_scale(agg::trans_affine& trans);

  std::list<line> m_lines;
  agg::trans_affine m_trans;

  // bounding box
  bool m_bbox_set;
  double m_x1, m_y1;
  double m_x2, m_y2;
};

#endif
