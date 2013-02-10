
#include <math.h>

#include "agg_path_storage.h"
#include "agg_ellipse.h"

#include "markers.h"
#include "trans.h"
#include "path.h"

/* used to encode a simple path */
struct path_code {
    enum {closed = 0, open = 1, end = 2};
    enum {scale = 32767};
    short x, y;
};

struct symbol_reg {
    const char *name;
    sg_object *(*builder)();
    const path_code* pcode;
};

static sg_object *build_circle();
static sg_object *build_path(const path_code* pcode);

/*
-- lua code to generate the coordinates

use 'math'

b = 2^15 - 1

function rot(alpha, x, y)
    return x*cos(alpha) - y*sin(alpha), y*cos(alpha) + x*sin(alpha)
end

function tr(x, y)
    return floor(x*b+0.5), floor(y*b+0.5)
end

-- example for asterisk
tr(rot(0, -0.08,  0.5))
tr(rot(0,  0.08,  0.5))
tr(rot(0,  0.08, -0.5))
tr(rot(0, -0.08, -0.5))

tr(rot(pi/3, -0.08,  0.5))
tr(rot(pi/3,  0.08,  0.5))
tr(rot(pi/3,  0.08, -0.5))
tr(rot(pi/3, -0.08, -0.5))
...
*/

static path_code asterisk[] = {
    {path_code::closed, 4},
    {-2621,  16384},
    { 2621,  16384},
    { 2621, -16384},
    {-2621, -16384},
    {path_code::closed, 4},
    {-15499,  5922},
    {-12878,  10462},
    { 15499, -5922},
    { 12878, -10462},
    {path_code::closed, 4},
    { 12878,  10462},
    { 15499,  5922},
    {-12878, -10462},
    {-15499, -5922},
    {path_code::end},
};

static path_code square[] = {
    {path_code::closed, 4},
    {-16383,  16384},
    { 16384,  16384},
    { 16384, -16383},
    {-16383, -16383},
    {path_code::end},
};

static path_code diamond[] = {
    {path_code::closed, 4},
    {-16383,  0},
    { 0,      16384},
    { 16384,  0},
    { 0,     -16383},
    {path_code::end},
};

static path_code triangle[] = {
    {path_code::closed, 3},
    {-18918, -10922},
    { 18918, -10922},
    { 0,      21845},
    {path_code::end},
};

static path_code plus[] = {
    {path_code::closed, 4},
    {-2621,  16384},
    { 2621,  16384},
    { 2621, -16384},
    {-2621, -16384},
    {path_code::closed, 4},
    {-16383,  2621},
    { 16384,  2621},
    { 16384, -2621},
    {-16383, -2621},
    {path_code::end},
};

static path_code cross[] = {
    {path_code::closed, 4},
    {-13438, 9731},
    {-9731, 13438},
    {13438, -9731},
    {9731, -13438},
    {path_code::closed, 4},
    {9731, 13438},
    {13438, 9731},
    {-9731, -13438},
    {-13438, -9731},
    {path_code::end},
};

const unsigned NB_SYMBOLS = 7;
static struct symbol_reg builder_table[NB_SYMBOLS+1] = {
    {"circle",   build_circle},
    {"square",   NULL, square},
    {"triangle", NULL, triangle},
    {"diamond",  NULL, diamond},
    {"plus",     NULL, plus},
    {"cross",    NULL, cross},
    {"asterisk", NULL, asterisk},
    {NULL, NULL}
};

sg_object *
build_circle()
{
    draw::ellipse* c = new draw::ellipse();
    c->self().init(0.0, 0.0, 0.5, 0.5);
    return c;
}

static inline void decode_coord(const path_code* p, double x[])
{
    x[0] = double(p->x) / path_code::scale;
    x[1] = double(p->y) / path_code::scale;
}

sg_object *
build_path(const path_code *pcode)
{
    draw::path* p = new draw::path();
    agg::path_storage& ps = p->self();

    for (const path_code* op = pcode; op->x != path_code::end; op = op + op->y + 1)
    {
        const path_code* code = op + 1;
        double x[2];
        decode_coord(code ++, x);
        ps.move_to(x[0], x[1]);
        for (short k = 1; k < op->y; k++)
        {
            decode_coord(code ++, x);
            ps.line_to(x[0], x[1]);
        }

        if (op->x == path_code::closed)
            ps.close_polygon();
    }

    return p;
}

sg_object*
new_marker_symbol_raw(const char *req_name)
{
    struct symbol_reg *reg;
    for (reg = builder_table; reg->name != NULL; reg++)
    {
        if (strcmp (reg->name, req_name) == 0)
        {
            if (reg->builder)
                return reg->builder();
            else
                return build_path(reg->pcode);
        }
    }

    return builder_table[0].builder();
}

sg_object*
new_marker_symbol (const char *req_name)
{
    sg_object* s = new_marker_symbol_raw(req_name);
    return new trans::scaling(s);
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
