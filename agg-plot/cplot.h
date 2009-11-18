#ifndef AGGPLOT_CPLOT_H
#define AGGPLOT_CPLOT_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <list>

#include "canvas.h"

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
  void add_line(line &ln);
  void draw(canvas &canvas);

  cplot() : m_lines(), m_trans_matrix(), m_x1(0.0), m_y1(0.0), 
	    m_x2(1.0), m_y2(1.0) {};

private:
  std::list<line> m_lines;
  agg::trans_affine m_trans_matrix;

  // bounding box
  double m_x1, m_y1;
  double m_x2, m_y2;
};

#endif
