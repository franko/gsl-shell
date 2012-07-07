#ifndef AGGPLOT_TEXT_H
#define AGGPLOT_TEXT_H

#include "agg_trans_affine.h"

#include "sg_object.h"
#include "text_label.h"

namespace draw {

class text : public sg_object
{
    agg::trans_affine m_matrix;
    agg::trans_affine m_user_matrix;

    double m_x, m_y;
    double m_angle;

    double m_hjustif;
    double m_vjustif;

    text_label m_text_label;

  public:
    text(const char* text, double size = 10.0, double hjustif = 0.0, double vjustif = 0.0):
    m_matrix(), m_user_matrix(), m_x(0.0), m_y(0.0), m_angle(0.0),
    m_hjustif(hjustif), m_vjustif(vjustif), m_text_label(text, 1.4*size)
    { }

    virtual void rewind(unsigned path_id)
    {
        m_text_label.model_mtx(m_matrix);
        m_text_label.rewind(m_hjustif, m_vjustif);
    }

    virtual unsigned vertex(double* x, double* y)
    {
        return m_text_label.vertex(x, y);
    }

    void angle(double th)
    {
        double c = cos(th), s = sin(th);

        m_angle = th;

        m_user_matrix.sx  =  c;
        m_user_matrix.shx = -s;
        m_user_matrix.shy =  s;
        m_user_matrix.sy  =  c;
    }

    double angle() const { return m_angle; };

    const char * get_text() const { return m_text_label.text().cstr(); }

    double text_height() const { return 1.4 * m_text_label.text_height(); }

    void set_point(double x, double y)
    {
        m_x = x;
        m_y = y;

        m_user_matrix.tx = m_x;
        m_user_matrix.ty = m_y;
    }

    void hjustif(double hj) { m_hjustif = hj; }
    void vjustif(double vj) { m_vjustif = vj; }

    virtual void apply_transform(const agg::trans_affine& m, double as);
    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2);

    virtual str write_svg(int id, agg::rgba8 c, double h);

    virtual bool use_subpixel() { return true; }
  };
}

#endif
