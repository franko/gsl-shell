
#include "trans.h"
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

  trans::dash *d1 = new trans::dash(p);
  d1->self().add_dash(7.0, 3.0);

  trans::stroke *s2 = new trans::stroke(s1);
  s1->self().width(1.0);

  if (s2->dispose())
    delete s2;

  if (d1->dispose())
    delete d1;

  delete p;

  return 0;
}
