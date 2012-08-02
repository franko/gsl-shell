#ifndef AGGPLOT_TEXT_SHAPE_H
#define AGGPLOT_TEXT_SHAPE_H

#include "text_label.h"
#include "sg_object.h"

namespace draw {

  class text_shape : public sg_object  {
  public:
    text_shape(double x, double y, const char* text,
               double _size = 10.0, double hjustif = 0.0, double vjustif = 0.0):
    m_text_label(text, _size), m_scaling(0), m_size(_size)
    {
        m_matrix.tx = x;
        m_matrix.ty = y;
        m_text_label.model_mtx(m_matrix);
    }

    virtual void rewind(unsigned path_id)
    {
        m_text_label.rewind(0.0, 0.0);
    }

    virtual unsigned vertex(double* x, double* y)
    {
        unsigned cmd = m_text_label.vertex(x, y);
        if (m_scaling && agg::is_vertex(cmd))
            m_scaling->transform(x, y);
        return cmd;
    }

    virtual bool render_nogamma() const { return true; }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
        const double pad = 1.0;
        const double tx = m_matrix.tx, ty = m_matrix.ty;
        *x1 = tx - pad;
        *y1 = ty - pad;
        *x2 = tx + m_text_label.get_text_width() + pad;
        *y2 = ty + m_text_label.get_text_height() + pad;
    }

    virtual str write_svg(int id, agg::rgba8 c, double h)
    {
        const str& text = m_text_label.text();
        double txt_size = m_size;

        const agg::trans_affine& m = m_matrix;

        double x = m.tx, y = m.ty;
        if (m_scaling)
        {
            m_scaling->transform(&x, &y);
            txt_size *= m_scaling->sy;
        }

        str s = str::print("<text x=\"%g\" y=\"%g\" id=\"text%i\"" \
                           " style=\"font-size:%i\">" \
                           " <tspan id=\"tspan%i\">%s</tspan>" \
                           "</text>",
                           x, svg_y_coord(y, h), id, int(txt_size), id, text.cstr());

        return s;
    }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
        m_scaling = &m;
        m_text_label.approximation_scale(m.scale());
    }

  private:
    text_label m_text_label;
    agg::trans_affine m_matrix;
    const agg::trans_affine* m_scaling;
    double m_size;
  };
}

#endif
