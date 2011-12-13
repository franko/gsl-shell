
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

static struct symbol_reg builder_table[] = {
  {"circle",   build_circle},
  {"square",   build_square},
  {"triangle", build_triangle},
  {NULL, NULL}
};

sg_object *
build_circle()
{
  draw::ellipse* c = new draw::ellipse();
  trans::scaling* s = new trans::scaling((sg_object*)c);
  c->self().init(0.0, 0.0, 0.5, 0.5);
  return s;
}

sg_object *
build_square()
{
  draw::path* p = new draw::path();
  trans::scaling* s = new trans::scaling((sg_object*)p);
  
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
  trans::scaling* s = new trans::scaling((sg_object*)p);
  
  agg::path_storage& triangle = p->self();

  double ht = 0.86602540378444;
  triangle.move_to(-0.5, -ht/3);
  triangle.line_to( 0.5, -ht/3);
  triangle.line_to( 0.0,  2*ht/3);
  triangle.close_polygon();

  return s;
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
