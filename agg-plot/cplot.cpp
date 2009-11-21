
#include "utils.h"
#include "cplot.h"

#include "agg_conv_stroke.h"
#include "agg_bounding_rect.h"

line&
cplot::new_line(agg::rgba8 color)
{
  int n = m_lines.size();
  m_lines.resize(n+1, color);
  std::list<line>::iterator ln = -- m_lines.end();
  return *ln;
}

bool
cplot::bbox_update()
{
  bool updated = false;

  for (std::list<line>::iterator j = m_lines.begin(); j != m_lines.end(); j++)
    {
      line& ln = *j;
      double x1, y1, x2, y2;
      bounding_rect_single(ln.path, 0, &x1, &y1, &x2, &y2);
      
      if (! m_bbox_set)
	{
	  m_x1 = x1;
	  m_x2 = x2;
	  m_y1 = y1;
	  m_y2 = y2;

	  m_bbox_set = true;
	  updated = true;
	  continue;
	}

      if (x2 > m_x2 || x1 < m_x1 || y2 > m_y2 || y1 < m_y1)
	{
	  m_x1 = min(x1, m_x1);
	  m_y1 = min(y1, m_y1);
	  m_x2 = max(x2, m_x2);
	  m_y2 = max(y2, m_y2);

	  updated = true;
	}
    }

  return updated;
}

void
cplot::draw_lines(canvas &canvas)
{
  typedef agg::path_storage path_type;

  agg::trans_affine m = this->trans();
  viewport_scale(m);
  canvas.scale(m);

  for (std::list<line>::iterator j = m_lines.begin(); j != m_lines.end(); j++)
    {
      line& ln = *j;
      path_type& p = ln.path;
      agg::conv_transform<path_type> ptr(p, m);
      agg::conv_stroke<agg::conv_transform<path_type> > ps(ptr);
      canvas.draw(ps, ln.color);
    }
}

void
cplot::draw(canvas &canvas)
{
  update();
  draw_lines(canvas);
}

bool
cplot::update()
{
  if (bbox_update())
    {
      double fx = m_x2 - m_x1, fy = m_y2 - m_y1;
      m_trans.reset();
      m_trans.scale(1/fx, 1/fy);
      m_trans.translate(-m_x1/fx, -m_y1/fy);
      return true;
    }
  return false;
}

void 
cplot::viewport_scale(agg::trans_affine& m)
{
  const double xoffs = 0.09375, yoffs = 0.09375;
  static agg::trans_affine rsz(1-2*xoffs, 0.0, 0.0, 1-2*yoffs, xoffs, yoffs);
  trans_affine_compose (m, rsz);
}
