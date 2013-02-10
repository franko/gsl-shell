
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
static sg_object *build_asterisk(bool& stroke);

const unsigned NB_SYMBOLS = 7;
static struct symbol_reg builder_table[NB_SYMBOLS+1] = {
    {"circle",   build_circle},
    {"square",   build_square},
    {"triangle", build_triangle},
    {"diamond",  build_diamond},
    {"plus",     build_plus},
    {"cross",    build_cross},
    {"asterisk", build_asterisk},
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
    triangle.move_to(-0.5/ht, -1.0/3.0);
    triangle.line_to( 0.5/ht, -1.0/3.0);
    triangle.line_to( 0.0,  2.0/3.0);
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
build_asterisk(bool& stroke)
{
    draw::path* p = new draw::path();
    const double lw = 0.16;
    const double x[4] = {-lw/2, lw/2, lw/2, -lw/2};
    const double y[4] = {0.5, 0.5, -0.5, -0.5};

    agg::path_storage& ps = p->self();

    ps.move_to(-lw/2,  0.5);
    ps.line_to( lw/2,  0.5);
    ps.line_to( lw/2, -0.5);
    ps.line_to(-lw/2, -0.5);
    ps.close_polygon();

    {
        const double a = M_PI / 3;
        const double s = sin(a), c = cos(a);
        ps.move_to(x[0] * c - y[0] * s, y[0] * c + x[0] * s);
        ps.line_to(x[1] * c - y[1] * s, y[1] * c + x[1] * s);
        ps.line_to(x[2] * c - y[2] * s, y[2] * c + x[2] * s);
        ps.line_to(x[3] * c - y[3] * s, y[3] * c + x[3] * s);
        ps.close_polygon();
    }

    {
        const double a = - M_PI / 3;
        const double s = sin(a), c = cos(a);
        ps.move_to(x[0] * c - y[0] * s, y[0] * c + x[0] * s);
        ps.line_to(x[1] * c - y[1] * s, y[1] * c + x[1] * s);
        ps.line_to(x[2] * c - y[2] * s, y[2] * c + x[2] * s);
        ps.line_to(x[3] * c - y[3] * s, y[3] * c + x[3] * s);
        ps.close_polygon();
    }

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

const char*
marker_lookup(int n)
{
    n = (n-1) % NB_SYMBOLS;
    return builder_table[n].name;
}
