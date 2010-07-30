
#include <math.h>

#include "markers.h"
#include "scalable.h"

#include "agg_path_storage.h"
#include "agg_ellipse.h"

struct symbol_reg {
  const char *name;
  scalable *path;
};

static vs_proxy<agg::ellipse, false> s_circle;
static vs_proxy<agg::path_storage, false> s_square, s_triangle;

static struct symbol_reg symbol_table[] = {
  {"circle",   &s_circle},
  {"square",   &s_square},
  {"triangle", &s_triangle},
  {NULL, NULL}
};

static scalable& s_default = s_circle;

namespace markers {

  void init()
  {
    agg::ellipse& ellipse = s_circle.self();
    ellipse.init(0.0, 0.0, 0.5, 0.5);

    agg::path_storage& square = s_square.self();

    square.move_to(-0.5, -0.5);
    square.line_to( 0.5, -0.5);
    square.line_to( 0.5,  0.5);
    square.line_to(-0.5,  0.5);
    square.close_polygon();

    agg::path_storage& triangle = s_triangle.self();

    double ht = 0.86602540378444;
    triangle.move_to(-0.5, -ht/3);
    triangle.line_to( 0.5, -ht/3);
    triangle.line_to( 0.0,  2*ht/3);
    triangle.close_polygon();
  }
 
  scalable& get (const char *req_name)
  {
    struct symbol_reg *reg;
    for (reg = symbol_table; reg->name != NULL; reg++)
      {
	if (strcmp (reg->name, req_name) == 0)
	  return *reg->path;
      }
    return s_default;
  }

}
