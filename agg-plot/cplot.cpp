
#include "utils.h"
#include "cplot.h"

#include "agg_conv_stroke.h"
#include "agg_bounding_rect.h"

template <class T>
T max (T a, T b) {
  return (b < a) ? a : b;
}

template <class T>
T min (T a, T b) {
  return (b > a) ? a : b;
}

void
cplot::add_line(line &ln)
{
  double x1, y1, x2, y2;
  m_lines.push_back(ln);
  bounding_rect_single(ln.path, 0, &x1, &y1, &x2, &y2);
  if (x2 > m_x2 || x1 < m_x1 || y2 > m_y2 || y1 < m_y1)
    {
      m_x1 = min(x1, m_x1);
      m_y1 = min(y1, m_y1);
      m_x2 = max(x2, m_x2);
      m_y2 = max(y2, m_y2);

      double fx = m_x2 - m_x1, fy = m_y2 - m_y1;
      m_trans_matrix.reset();
      m_trans_matrix.scale(1/fx, 1/fy);
      m_trans_matrix.translate(-m_x1/fx, -m_y1/fy);
    }
}

void
cplot::draw(canvas &canvas)
{
  typedef agg::path_storage path_type;

  std::list<line>::iterator j;

  agg::path_storage box;
  agg::conv_stroke<agg::path_storage> boxl(box);
  agg::conv_transform<agg::conv_stroke<agg::path_storage> > boxtr(boxl, canvas.trans_matrix());

  box.move_to(0.1, 0.1);
  box.line_to(0.1, 0.9);
  box.line_to(0.9, 0.9);
  box.line_to(0.9, 0.1);
  box.close_polygon();

  boxl.width(0.001);

  canvas.draw(boxtr, agg::rgba8(0, 0, 0));

  agg::trans_affine m = m_trans_matrix;
  agg::trans_affine resize(0.8, 0.0, 0.0, 0.8, 0.1, 0.1);
  trans_affine_compose (m, resize);
  trans_affine_compose (m, canvas.trans_matrix());

  for (j = m_lines.begin(); j != m_lines.end(); j++)
    {
      line& ln = *j;
      path_type& p = ln.path;
      agg::conv_stroke<path_type> pl(p);
      agg::conv_transform<agg::conv_stroke<path_type> > tr(pl, m);

      pl.width(0.2);

      canvas.draw(tr, ln.color);
    }
}
