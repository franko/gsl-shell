
#include "agg_bounding_rect.h"

#include "drawable.h"
#include "utils.h"

agg::trans_affine window_scalable::dummy_matrix;

void
window_scalable::apply_transform(const agg::trans_affine& m)
{
  m_trans.transformer(m);
  m_source->approximation_scale (m.scale());
}

void
window_scalable::bounding_box(double *x1, double *y1,
				  double *x2, double *y2)
{
  agg::bounding_rect_single (*m_source, 0, x1, y1, x2, y2);
}

void
window_scalable::rewind(unsigned path_id)
{
  m_trans.rewind(path_id);
}

unsigned
window_scalable::vertex(double* x, double* y)
{
  return m_trans.vertex(x, y);
}

bool
window_scalable::dispose()
{
  if (m_source->dispose())
    delete m_source;
  return true;
}
