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

  agg::path_storage mark;
  agg::conv_transform<path_type> mark_tr(mark, m);
  agg::conv_stroke<agg::conv_transform<path_type> > mark_stroke(mark_tr);

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
	char lab_text[32];
	double xlab = 0, ylab = y;

	lab.size(10.0);
	m_uy.mark_label(lab_text, 32, j);
	lab.text(lab_text);
	labs.width(0.7);

	m.transform(&xlab, &ylab);

	xlab += -lab.text_width() - 8.0;
	ylab += -10.0/2.0;

	lab.start_point(xlab, ylab);
	canvas.draw(labs, agg::rgba(0, 0, 0));

	mark.move_to(0.0, y);
	mark.line_to(-0.01, y);

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
	char lab_text[32];
	double xlab = x, ylab = 0;

	lab.size(10.0);
	m_ux.mark_label(lab_text, 32, j);
	lab.text(lab_text);
	labs.width(0.7);

	m.transform(&xlab, &ylab);

	xlab += -lab.text_width()/2.0;
	ylab += -10.0 - 10.0;

	lab.start_point(xlab, ylab);
	canvas.draw(labs, agg::rgba(0, 0, 0));

	mark.move_to(x, 0.0);
	mark.line_to(x, -0.01);

	if (j > jinf && j < jsup)
	  {
	    ln.move_to(x, 0.0);
	    ln.line_to(x, 1.0);
	  }
      }
  }

  lndash.add_dash(8.0, 4.0);

  lns.width(0.25);
  canvas.draw(lns, agg::rgba(0.2, 0.2, 0.2));

  mark_stroke.width(1.0);
  canvas.draw(mark_stroke, agg::rgba8(0, 0, 0));

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
