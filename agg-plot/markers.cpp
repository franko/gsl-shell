
#include <math.h>

#include "agg_path_storage.h"
#include "agg_ellipse.h"

#include "markers.h"
#include "trans.h"
#include "path.h"

struct symbol_reg {
    const char *name;
    sg_object *(*builder)(bool&);
};

static sg_object *build_circle(bool& stroke);
static sg_object *build_square(bool& stroke);
static sg_object *build_triangle(bool& stroke);
static sg_object *build_diamond(bool& stroke);
static sg_object *build_plus(bool& stroke);
static sg_object *build_cross(bool& stroke);

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
build_circle(bool& stroke)
{
    draw::ellipse* c = new draw::ellipse();
    c->self().init(0.0, 0.0, 0.5, 0.5);
    stroke = false;
    return c;
}

sg_object *
build_square(bool& stroke)
{
    draw::path* p = new draw::path();

    agg::path_storage& square = p->self();
    square.move_to(-0.5, -0.5);
    square.line_to( 0.5, -0.5);
    square.line_to( 0.5,  0.5);
    square.line_to(-0.5,  0.5);
    square.close_polygon();

    stroke = false;
    return p;
}

sg_object *
build_triangle(bool& stroke)
{
    draw::path* p = new draw::path();

    agg::path_storage& triangle = p->self();

    double ht = 0.86602540378444;
    triangle.move_to(-0.5, -ht/3);
    triangle.line_to( 0.5, -ht/3);
    triangle.line_to( 0.0,  2*ht/3);
    triangle.close_polygon();

    stroke = false;
    return p;
}

sg_object *
build_diamond(bool& stroke)
{
    draw::path* p = new draw::path();

    agg::path_storage& square = p->self();
    square.move_to(-0.5,  0.0);
    square.line_to( 0.0,  0.5);
    square.line_to( 0.5,  0.0);
    square.line_to( 0.0, -0.5);
    square.close_polygon();

    stroke = false;
    return p;
}

sg_object *
build_plus(bool& stroke)
{
    draw::path* p = new draw::path();

    agg::path_storage& plus = p->self();
    plus.move_to(-0.5,  0.0);
    plus.line_to( 0.5,  0.0);
    plus.move_to( 0.0, -0.5);
    plus.line_to( 0.0,  0.5);

    stroke = true;
    return p;
}

sg_object *
build_cross(bool& stroke)
{
    draw::path* p = new draw::path();

    agg::path_storage& plus = p->self();
    plus.move_to(-0.5, -0.5);
    plus.line_to( 0.5,  0.5);
    plus.move_to(-0.5,  0.5);
    plus.line_to( 0.5, -0.5);

    stroke = true;
    return p;
}

sg_object*
new_marker_symbol_raw(const char *req_name, bool& stroke)
{
    struct symbol_reg *reg;
    for (reg = builder_table; reg->name != NULL; reg++)
    {
        if (strcmp (reg->name, req_name) == 0)
            return reg->builder(stroke);
    }

    return builder_table[0].builder(stroke);
}

sg_object*
new_marker_symbol (const char *req_name)
{
    bool stroke;
    sg_object* s = new_marker_symbol_raw(req_name, stroke);

    trans::scaling *ss = new trans::scaling(s);
    sg_object* sf = ss;
    if (stroke)
        sf = new trans::stroke(sf);
    return sf;
}

sg_object*
new_marker_symbol (int n)
{
    n = (n-1) % NB_SYMBOLS;
    return new_marker_symbol(builder_table[n].name);
}
