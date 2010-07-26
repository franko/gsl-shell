
#ifndef AGGPLOT_DRAWABLES_H
#define AGGPLOT_DRAWABLES_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "vertex-source.h"

#include "agg_gsv_text.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"

namespace draw {

  class text : public window_object {
    typedef agg::gsv_text vs_text;
    typedef agg::conv_transform<vs_text> vs_trans_text;
    typedef agg::conv_stroke<vs_trans_text> vs_stroked_text;

    agg::trans_affine m_matrix;

    vs_text m_text;
    vs_trans_text m_trans;
    vs_stroked_text m_stroke;

    double m_x, m_y;
    double m_angle;

  public:
    text(double size = 10.0, double width = 1.0):
      m_matrix(), m_text(), m_trans(m_text, m_matrix), m_stroke(m_trans)
    {
    };

    void set_angle(double th)
    {
      double c = cos(th), s = sin(th);
      m_matrix.sx  =  c;
      m_matrix.shx =  s;
      m_matrix.shy = -s;
      m_matrix.sy  =  c;
    };

    void set_text(const char *s) { m_text.text(s); };

    virtual void apply_transform(agg:trans_affine& m);
    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2);
    virtual bool dispose();
  };
}
