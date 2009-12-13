
#include "utils.h"
#include "plot.h"

#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_contour.h"
#include "agg_vcgen_markers_term.h"
#include "agg_bounding_rect.h"

void
plot::bounding_box(double *px1, double *py1, double *px2, double *py2)
{
  bool is_set = false;
  double& m_x1 = *px1;
  double& m_y1 = *py1;
  double& m_x2 = *px2;
  double& m_y2 = *py2;

  for (unsigned j = 0; j < m_elements.size(); j++)
  {
    drawable& d = m_elements[j];

    double x1, y1, x2, y2;
    d.vs->bounding_box(&x1, &y1, &x2, &y2);
      
    if (! is_set)
    {
      m_x1 = x1;
      m_x2 = x2;
      m_y1 = y1;
      m_y2 = y2;

      is_set = true;
    }
    else if (x2 > m_x2 || x1 < m_x1 || y2 > m_y2 || y1 < m_y1)
    {
      m_x1 = min(x1, m_x1);
      m_y1 = min(y1, m_y1);
      m_x2 = max(x2, m_x2);
      m_y2 = max(y2, m_y2);
    }
  }
}

void
plot::trans_matrix_update()
{
  if (! m_bbox_updated)
  {
    double x1, y1, x2, y2;
    bounding_box(&x1, &y1, &x2, &y2);

    double fx = x2 - x1, fy = y2 - y1;
    m_trans.reset();
    m_trans.scale(1/fx, 1/fy);
    m_trans.translate(-x1/fx, -y1/fy);
    m_bbox_updated = true;
  }
}

void
plot::draw_elements(canvas &canvas)
{
  agg::trans_affine m = m_trans;
  viewport_scale(m);
  canvas.scale(m);

  for (unsigned j = 0; j < m_elements.size(); j++)
    {
      drawable& d = m_elements[j];
      vertex_source& vs = d.get_vertex_source();
      vs.apply_transform(m, 1.0);
      canvas.draw(vs, d.color);
    }
}

void 
plot::viewport_scale(agg::trans_affine& m)
{
  const double xoffs = 0.09375, yoffs = 0.09375;
  static agg::trans_affine rsz(1-2*xoffs, 0.0, 0.0, 1-2*yoffs, xoffs, yoffs);
  trans_affine_compose (m, rsz);
}
