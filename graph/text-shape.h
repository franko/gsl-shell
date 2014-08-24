#ifndef AGGPLOT_TEXT_SHAPE_H
#define AGGPLOT_TEXT_SHAPE_H

#include "text_label.h"
#include "sg_object.h"

namespace draw {

class text_shape : public sg_object  {
public:
    text_shape(double x, double y, const char* text,
               double _size = 10.0, double hjustif = 0.0, double vjustif = 0.0):
        m_text_label(text, round(_size)), m_x(x), m_y(y), m_font_size(_size), m_size(_size)
    {
        m_matrix.tx = m_x;
        m_matrix.ty = m_y;
        m_text_label.model_mtx(m_matrix);
        compute_bounding_box();
    }

    virtual void rewind(unsigned path_id)
    {
        m_text_label.rewind(0.0, 0.0);
    }

    virtual unsigned vertex(double* x, double* y)
    {
        return m_text_label.vertex(x, y);
    }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
        *x1 = m_bbox.x1;
        *y1 = m_bbox.y1;
        *x2 = m_bbox.x2;
        *y2 = m_bbox.y2;
    }

    void compute_bounding_box()
    {
        const double pad = 1.0;
        const double tx = m_matrix.tx, ty = m_matrix.ty;
        m_bbox.x1 = tx - pad;
        m_bbox.y1 = ty - pad;
        m_bbox.x2 = tx + m_text_label.get_text_width() + pad;
        m_bbox.y2 = ty + m_text_label.get_text_height() + pad;
    }

    virtual str write_svg(int id, agg::rgba8 c, double h)
    {
        const str& text = m_text_label.text();
        double txt_size = m_size;

        const agg::trans_affine& m = m_matrix;

        double x = m.tx, y = m.ty;

        str s = str::print("<text x=\"%g\" y=\"%g\" id=\"text%i\"" \
                           " style=\"font-size:%i\">" \
                           " <tspan id=\"tspan%i\">%s</tspan>" \
                           "</text>",
                           x, svg_y_coord(y, h), id, int(txt_size), id, text.cstr());

        return s;
    }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
        m_text_label.font_size(m.sx * m_font_size, round(m.sy * m_font_size));

        double x = m_x, y = m_y;
        m.transform(&x, &y);
        m_matrix.tx = x;
        m_matrix.ty = y;

        m_text_label.approximation_scale(m.scale());
    }

private:
    text_label m_text_label;
    double m_x, m_y;
    double m_font_size;
    agg::trans_affine m_matrix;
    double m_size;
    agg::rect_base<double> m_bbox;
};
}

#endif
