#include "utils.h"

agg::trans_affine identity_matrix;

/* Mnemonic: a.premultiply(b) means a * b in terms of matrix multiplication */

void
trans_affine_compose (agg::trans_affine& a, const agg::trans_affine& b)
{
    double a_tx = a.tx, a_ty = a.ty;

    a.premultiply(b);

    a.tx = b.sx  * a_tx + b.shx * a_ty + b.tx;
    a.ty = b.shy * a_tx + b.sy  * a_ty + b.ty;
}

agg::trans_affine affine_matrix(const agg::rect_i& r)
{
    double w = r.x2 - r.x1, h = r.y2 - r.y1;
    double tx = r.x1, ty = r.y1;
    return agg::trans_affine(w, 0.0, 0.0, h, tx, ty);
}
