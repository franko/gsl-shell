
#include "text.h"

namespace draw {

  void
  text::apply_transform(agg:trans_affine& m)
  {
    double& x = m_matrix.tx;
    double& y = m_matrix.ty;
    m.transform(&x, &y);
  }

  void
  text::bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    *x1 = *x2 = m_x;
    *y1 = *y1 = m_y;
  }

  bool
  text::dispose()
  {
    return false;
  }
}
