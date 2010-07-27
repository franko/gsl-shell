
#include "trans.h"
#include "window-trans.h"
#include "path.h"

int
main()
{
  draw::path *p = new draw::path();

  agg::path_storage& pc = p->get_base();
  pc.move_to(0.0, 0.0);
  pc.line_to(1.0, 0.0);
  pc.line_to(1.0, 1.0);
  pc.line_to(0.0, 1.0);
  pc.close_polygon();

  trans::stroke *s1 = new trans::stroke(p);
  s1->self().width(10.0);
  s1->self().line_cap(agg::round_cap);

  window_scalable *ws1 = new window_scalable(s1);

  window::stroke *s2 = new window::stroke(ws1);
  s2->self().width(1.0);
  s2->self().line_cap(agg::round_cap);
  
  double x1, y1, x2, y2;
  s2->bounding_box(&x1, &y1, &x2, &y2);

  agg::trans_affine mtx(200.0, 0.0, 0.0, 100.0, 0.0, 0.0);
  s2->apply_transform(mtx);

  if (s2->dispose())
    delete s2;

  delete p;

  return 0;
}
