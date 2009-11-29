
#include "utils.h"
#include "cplot.h"

#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_vcgen_markers_term.h"
#include "agg_bounding_rect.h"

void
cplot::add(drawable *d)
{
  m_lines.add(d);
}

bool
cplot::bbox_update()
{
  bool updated = false;

  for (unsigned j = 0; j < m_lines.size(); j++)
    {
      drawable* d = m_lines[j];

      double x1, y1, x2, y2;
      d->bounding_box(&x1, &y1, &x2, &y2);
      
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

  for (unsigned j = 0; j < m_lines.size(); j++)
    {
      m_lines[j]->draw(canvas, m);
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

void
line::draw(canvas& can, agg::trans_affine& t)
{
  typedef agg::path_storage path_type;

  path_type& p = this->path;
  agg::conv_transform<path_type> ptr(p, t);
  agg::conv_stroke<agg::conv_transform<path_type> > ps(ptr);
  can.draw(ps, this->color);
}

void
line::bounding_box(double *x1, double *y1, double *x2, double *y2)
{
  bounding_rect_single(this->path, 0, x1, y1, x2, y2);
}

void
dashed_line::draw(canvas& canvas, agg::trans_affine& t)
{
  typedef agg::path_storage path_type;
  typedef agg::conv_dash<agg::conv_transform<path_type>, agg::vcgen_markers_term> dash_type;

  path_type& ln = this->path;
  agg::conv_transform<path_type> lntr(ln, t);
  dash_type lndash(lntr);
  agg::conv_stroke<dash_type> lns(lndash);

  lndash.add_dash(m_l1, m_l2);
  canvas.draw(lns, this->color);
}

void
polygon::draw(canvas& canvas, agg::trans_affine& t)
{
  agg::path_storage& p = this->path;
  agg::conv_transform<agg::path_storage> ptr(p, t);
  canvas.draw(ptr, this->color);
}

void
poly_outline::draw(canvas& canvas, agg::trans_affine& t)
{
  typedef agg::path_storage path_type;

  path_type& p = this->path;
  agg::conv_transform<path_type> ptr(p, t);
  canvas.draw(ptr, this->color);

  agg::conv_stroke<agg::conv_transform<path_type> > ps(ptr);
  canvas.draw(ps, this->m_outline_color);
}
