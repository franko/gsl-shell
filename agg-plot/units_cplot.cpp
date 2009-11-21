#include "units_cplot.h"

#include "agg_vcgen_markers_term.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_gsv_text.h"

void
units_cplot::draw_axis(canvas &canvas)
{
  typedef agg::path_storage path_type;
  typedef agg::conv_dash<agg::conv_transform<path_type>, agg::vcgen_markers_term> dash_type;

  agg::trans_affine m;
  viewport_scale(m);
  canvas.scale(m);

  agg::path_storage ln;
  agg::conv_transform<path_type> lntr(ln, m);
  dash_type lndash(lntr);
  agg::conv_stroke<dash_type> lns(lndash);

  {
    int jinf = m_uy.begin(), jsup = m_uy.end();
    for (int j = jinf; j <= jsup; j++)
      {
	double y = double(j - jinf) / double(jsup - jinf);
	agg::gsv_text lab;
	agg::conv_stroke<agg::gsv_text> labs(lab);
	agg::conv_transform<agg::conv_stroke<agg::gsv_text> > labo(labs, m);
	std::string lab_text;
	
        labs.line_join(agg::round_join);
        labs.line_cap(agg::round_cap);
        labs.approximation_scale(m.scale());

	lab.size(0.03, 0.02);
	labs.width(1.5 * 1/max(m.sx, m.sy));
	m_uy.mark_label(lab_text, j);
	lab.text(lab_text.c_str());

	lab.start_point(-0.01 - lab.text_width(), y - 0.015);
	canvas.draw(labo, agg::rgba(0, 0, 0));

	if (j > jinf && j < jsup)
	  {
	    ln.move_to(0.0, y);
	    ln.line_to(1.0, y);
	  }
      }
  }

  {
    int jinf = m_ux.begin(), jsup = m_ux.end();
    for (int j = jinf; j <= jsup; j++)
      {
	double x = double(j - jinf) / double(jsup - jinf);
	agg::gsv_text lab;
	agg::conv_stroke<agg::gsv_text> labs(lab);
	agg::conv_transform<agg::conv_stroke<agg::gsv_text> > labo(labs, m);
	std::string lab_text;
	
        labs.line_join(agg::round_join);
        labs.line_cap(agg::round_cap);
        labs.approximation_scale(m.scale());

	lab.size(0.03, 0.02);
	labs.width(1.5 * 1/max(m.sx, m.sy));
	m_ux.mark_label(lab_text, j);
	lab.text(lab_text.c_str());

	lab.start_point(x - lab.text_width()/2.0, -0.025 - 0.03);
	canvas.draw(labo, agg::rgba(0, 0, 0));

	if (j > jinf && j < jsup)
	  {
	    ln.move_to(x, 0.0);
	    ln.line_to(x, 1.0);
	  }
      }
  }

  lndash.add_dash(8.0, 4.0);

  lns.width(0.25);
  canvas.draw(lns, agg::rgba8(0.8, 0.8, 0.8));

  agg::path_storage box;
  agg::conv_transform<path_type> boxtr(box, m);
  agg::conv_stroke<agg::conv_transform<path_type> > boxs(boxtr);

  box.move_to(0.0, 0.0);
  box.line_to(0.0, 1.0);
  box.line_to(1.0, 1.0);
  box.line_to(1.0, 0.0);
  box.close_polygon();

  canvas.draw(boxs, agg::rgba8(0, 0, 0));
};
