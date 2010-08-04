
#include "trans.h"
#include "path.h"
#include "text.h"

int
main()
{
  draw::path *p = new draw::path();

  agg::path_storage& pc = p->self();
  pc.move_to(0.0, 0.0);
  pc.line_to(20.0, 0.0);
  pc.line_to(20.0, 20.0);
  pc.line_to(0.0, 20.0);
  pc.close_polygon();

  trans<scalable_context>::stroke *s1 = new trans<scalable_context>::stroke(p);
  s1->self().width(4.0);
  s1->self().line_cap(agg::round_cap);

  double c = 0.707, s= 0.707;
  agg::trans_affine rmat(c, s, -s, c, 0.0, 0.0);
  trans<scalable_context>::affine *rs1 = new trans<scalable_context>::affine(s1, rmat);

  window_scalable *ws1 = new window_scalable(rs1);

  trans<drawable_context>::stroke *s2 = new trans<drawable_context>::stroke(ws1);
  s2->self().width(1.0);
  s2->self().line_cap(agg::round_cap);
  
  double x1, y1, x2, y2;
  s2->bounding_box(&x1, &y1, &x2, &y2);

  agg::trans_affine mtx(200.0, 0.0, 0.0, 100.0, 0.0, 0.0);
  s2->apply_transform(mtx);

  draw::text *txt = new draw::text(12.0);
  txt->self().text("Hello world!");
  txt->self().start_point(4.0, 5.0);

  trans<drawable_context>::dash *d2 = new trans<drawable_context>::dash(txt);
  d2->self().add_dash(2.0, 2.0);

  if (s2->dispose())
    delete s2;

  if (d2->dispose())
    delete d2;

  delete txt;
  delete p;

  return 0;
}
