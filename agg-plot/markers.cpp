
#include <math.h>

#include "markers.h"
#include "scalable.h"

#include "agg_path_storage.h"
#include "agg_ellipse.h"

struct symbol_reg {
  const char *name;
  vertex_source *(*builder)();
};

static vertex_source *build_circle();
static vertex_source *build_square();
static vertex_source *build_triangle();

static struct symbol_reg builder_table[] = {
  {"circle",   build_circle},
  {"square",   build_square},
  {"triangle", build_triangle},
  {NULL, NULL}
};

vertex_source *
build_circle()
{
  typedef vs_proxy<agg::ellipse, true> circle_type;
  circle_type *circle = new circle_type();
  circle->self().init(0.0, 0.0, 0.5, 0.5);
  return (vertex_source *) circle;
}

vertex_source *
build_square()
{
  typedef vs_proxy<agg::path_storage, false> path_type;
  path_type *p = new path_type();
  
  agg::path_storage& square = p->self();
  square.move_to(-0.5, -0.5);
  square.line_to( 0.5, -0.5);
  square.line_to( 0.5,  0.5);
  square.line_to(-0.5,  0.5);
  square.close_polygon();

  return (vertex_source *) p;
}

vertex_source *
build_triangle()
{
  typedef vs_proxy<agg::path_storage, false> path_type;
  path_type *p = new path_type();
  
  agg::path_storage& triangle = p->self();

  double ht = 0.86602540378444;
  triangle.move_to(-0.5, -ht/3);
  triangle.line_to( 0.5, -ht/3);
  triangle.line_to( 0.0,  2*ht/3);
  triangle.close_polygon();

  return (vertex_source *) p;
}

vertex_source*
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
