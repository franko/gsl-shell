#ifndef AGGPLOT_SG_MARKER_H
#define AGGPLOT_SG_MARKER_H

#include "agg_conv_transform.h"
#include "agg_trans_affine.h"

#include "sg_object.h"
#include "markers.h"
#include "utils.h"

typedef agg::conv_transform<sg_object> conv_type;

namespace draw {

class marker_a : public sg_adapter<conv_type, no_approx_scale> {

public:
    marker_a(double x, double y, sg_object* sym, double size):
        sg_adapter<conv_type, no_approx_scale>(sym, m_trans),
        m_trans(size), m_symbol(sym), m_x(x), m_y(y)
    {
        // the following is needed to adjust the approximation_scale of the
        // symbol
        m_symbol->apply_transform(identity_matrix, size);
    }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
        double *tx = &m_trans.tx, *ty = &m_trans.ty;
        *tx = m_x;
        *ty = m_y;
        m.transform(tx, ty);
    }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
        *x1 = *x2 = m_x;
        *y1 = *y2 = m_y;
    }

protected:
    agg::trans_affine_scaling m_trans;
    sg_object* m_symbol;
    double m_x, m_y;
};

class marker : public marker_a {
public:
    marker(double x, double y, sg_object* s, double sz) : marker_a(x, y, s, sz) {}
    ~marker() {
        delete m_symbol;
    }
};
}

#endif
