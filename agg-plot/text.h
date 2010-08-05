#ifndef AGGPLOT_TEXT_H
#define AGGPLOT_TEXT_H

#include "drawable.h"

#include "agg_gsv_text.h"
#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"

namespace draw {

  class text : public drawable {
    typedef agg::gsv_text vs_text;
    typedef agg::conv_transform<vs_text> vs_trans_text;
    typedef agg::conv_stroke<vs_trans_text> vs_stroked_text;

    agg::trans_affine m_matrix;

    vs_text m_text;
    vs_trans_text m_trans;
    vs_stroked_text m_stroke;

    double m_x, m_y;
    double m_angle;

    double m_text_width;
    double m_text_height;

    double m_hjustif;
    double m_vjustif;

  public:
    text(double size = 10.0, double width = 1.0):
      m_matrix(), m_text(), m_trans(m_text, m_matrix), m_stroke(m_trans),
      m_x(0.0), m_y(0.0), m_text_width(0.0), m_text_height(size),
      m_hjustif(0.0), m_vjustif(0.0)
    {
      m_stroke.width(width + 0.5);
      m_stroke.line_cap(agg::round_cap);
      m_stroke.line_join(agg::round_join);
      m_text.size(size);
    };

    void angle(double th)
    {
      double c = cos(th), s = sin(th);

      m_matrix.sx  =  c;
      m_matrix.shx = -s;
      m_matrix.shy =  s;
      m_matrix.sy  =  c;
    };

    void set_text(const char *txt) 
    { 
      m_text.text(txt);
      m_text_width = m_text.text_width();
    };

    void set_point(double x, double y)
    {
      m_x = x;
      m_y = y;
    };

    void hjustif(double hj) { m_hjustif = hj; };
    void vjustif(double vj) { m_vjustif = vj; };

    virtual void rewind(unsigned path_id);
    virtual unsigned vertex(double* x, double* y);

    virtual void apply_transform(const agg::trans_affine& m, double as);
    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2);
    virtual bool dispose();

    vs_text& self() { return m_text; };
  };
}

#endif
