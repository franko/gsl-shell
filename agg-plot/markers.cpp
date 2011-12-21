
#include <math.h>

#include "agg_path_storage.h"
#include "agg_ellipse.h"

#include "markers.h"
#include "trans.h"
#include "path.h"

struct symbol_reg {
  const char *name;
  sg_object *(*builder)();
};

static sg_object *build_circle();
static sg_object *build_square();
static sg_object *build_triangle();
static sg_object *build_diamond();
static sg_object *build_plus();
static sg_object *build_cross();

const unsigned NB_SYMBOLS = 6;
static struct symbol_reg builder_table[NB_SYMBOLS+1] = {
  {"circle",   build_circle},
  {"square",   build_square},
  {"triangle", build_triangle},
  {"diamond",  build_diamond},
  {"plus",     build_plus},
  {"cross",    build_cross},
  {NULL, NULL}
};

sg_object *
build_circle()
{
  draw::ellipse* c = new draw::ellipse();
  trans::scaling* s = new trans::scaling(c);
  c->self().init(0.0, 0.0, 0.5, 0.5);
  return s;
}

sg_object *
build_square()
{
  draw::path* p = new draw::path();
  trans::scaling* s = new trans::scaling(p);

  agg::path_storage& square = p->self();
  square.move_to(-0.5, -0.5);
  square.line_to( 0.5, -0.5);
  square.line_to( 0.5,  0.5);
  square.line_to(-0.5,  0.5);
  square.close_polygon();

  return s;
}

sg_object *
build_triangle()
{
  draw::path* p = new draw::path();
  trans::scaling* s = new trans::scaling(p);

  agg::path_storage& triangle = p->self();

  double ht = 0.86602540378444;
  triangle.move_to(-0.5, -ht/3);
  triangle.line_to( 0.5, -ht/3);
  triangle.line_to( 0.0,  2*ht/3);
  triangle.close_polygon();

  return s;
}

sg_object *
build_diamond()
{
  draw::path* p = new draw::path();
  trans::scaling* s = new trans::scaling(p);

  agg::path_storage& square = p->self();
  square.move_to(-0.5,  0.0);
  square.line_to( 0.0,  0.5);
  square.line_to( 0.5,  0.0);
  square.line_to( 0.0, -0.5);
  square.close_polygon();

  return s;
}

sg_object *
build_plus()
{
  draw::path* p = new draw::path();
  trans::scaling* s = new trans::scaling(p);

  agg::path_storage& plus = p->self();
  plus.move_to(-0.5,  0.0);
  plus.line_to( 0.5,  0.0);
  plus.move_to( 0.0, -0.5);
  plus.line_to( 0.0,  0.5);

  return new trans::stroke(s);
}

sg_object *
build_cross()
{
  draw::path* p = new draw::path();
  trans::scaling* s = new trans::scaling(p);

  agg::path_storage& plus = p->self();
  plus.move_to(-0.5, -0.5);
  plus.line_to( 0.5,  0.5);
  plus.move_to(-0.5,  0.5);
  plus.line_to( 0.5, -0.5);

  return new trans::stroke(s);
}

sg_object*
new_marker_symbol (const char *req_name)
{
  struct symbol_reg *reg;
  for (reg = builder_table; reg->name != NULL; reg++)
    {
      if (strcmp (reg->name, req_name) == 0)
	return reg->builder();
    }

  return builder_table[0].builder();
}

sg_object*
new_marker_symbol (int n)
{
  n = (n-1) % NB_SYMBOLS;
  return builder_table[n].builder();
}
