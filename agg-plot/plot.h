
/* plot.h
 * 
 * Copyright (C) 2009, 2010 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AGGPLOT_CPLOT_H
#define AGGPLOT_CPLOT_H

#include "utils.h"
#include "drawable.h"
#include "canvas.h"
#include "units.h"
#include "resource-manager.h"

#include "agg_array.h"
#include "agg_vcgen_markers_term.h"
#include "agg_conv_transform.h"
#include "agg_color_rgba.h"
#include "agg_path_storage.h"
#include "agg_array.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_gsv_text.h"


static void
bbox_enlarge(double *x1, double* y1, double* x2, double* y2,
	     double x, double y)
{
  if (x < *x1) *x1 = x;
  if (y < *y1) *y1 = y;
  if (x > *x2) *x2 = x;
  if (y > *y2) *y2 = y;
}

template<class VertexSource, class resource_manager = no_management>
class plot {

  class container {
  public:
    VertexSource* vs;
    agg::rgba8 color;
    bool outline;

    container(): vs(NULL), color(), outline(false) {};

    container(VertexSource* vs, agg::rgba8 *c, bool as_outline): 
      vs(vs), color(*c), outline(as_outline)
    {};

    ~container() {};

    void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
      VertexSource& vsi = get_vertex_source();
      vsi.bounding_box(x1, y1, x2, y2);
    };

    VertexSource& get_vertex_source() { return *vs; };
  };

public:
  plot() : 
    m_elements(), m_trans(), m_bbox_updated(true),
    m_title_buf(), m_use_units(true)
  {
    m_title_buf.capacity(32);
    m_title = m_title_buf.data();
    m_title[0] = '\0';
  };

  ~plot() 
  {
    for (unsigned j = 0; j < m_elements.size(); j++)
      {
	container& d = m_elements[j];
	resource_manager::dispose(d.vs);
      }
  };

  void set_title(const char *text) {
    unsigned int len = strlen(text);
    m_title_buf.resize(len+1);
    m_title = m_title_buf.data();
    memcpy (m_title, text, len+1);
  };

  const char *get_title() const { return m_title; };

  bool use_units() const { return m_use_units; };
  void set_units(bool use_units);

  void add(VertexSource* vs, agg::rgba8 *color, bool outline = false) 
  { 
    container d(vs, color, outline);
    m_elements.add(d);
    m_bbox_updated = false;
    resource_manager::acquire(vs);
  };

  void draw(canvas &canvas, agg::trans_affine& m);

  void trans_matrix_update();
  void user_transform(agg::trans_affine& m)
  {
    m = m_trans;
    viewport_scale(m);
  };

private:
  void draw_elements(canvas &canvas, agg::trans_affine& m);
  void draw_title(canvas& canvas, agg::trans_affine& m);
  void draw_axis(canvas& can, agg::trans_affine& m);

  void update_viewport_trans();

  void calc_bounding_box();

  static void viewport_scale(agg::trans_affine& trans);

  agg::pod_bvector<container> m_elements;
  agg::trans_affine m_trans;

  // bounding box
  bool   m_bbox_updated;
  double m_x1, m_y1;
  double m_x2, m_y2;

  agg::pod_vector<char> m_title_buf;
  char *m_title;

  bool m_use_units;
  units m_ux, m_uy;
};

template <class VS, class RM>
void plot<VS,RM>::draw(canvas &canvas, agg::trans_affine& canvas_mtx)
{
  trans_matrix_update();
  draw_title(canvas, canvas_mtx);
  if (m_use_units)
    draw_axis(canvas, canvas_mtx);
  draw_elements(canvas, canvas_mtx);
};

template <class VS, class RM>
void plot<VS,RM>::draw_title(canvas &canvas, agg::trans_affine& canvas_mtx)
{
  double xt = 0.5, yt = 1;

  agg::trans_affine m;
  this->viewport_scale(m);
  trans_affine_compose (m, canvas_mtx);

  agg::gsv_text title;
  agg::conv_stroke<agg::gsv_text> titlestroke(title);

  title.size(12.0);
  title.text(m_title);

  titlestroke.width(1.5);
  titlestroke.line_cap(agg::round_cap);
  titlestroke.line_join(agg::round_join);

  m.transform(&xt, &yt);

  xt += -title.text_width() / 2;
  yt += 10.0;

  title.start_point(xt, yt);
  canvas.draw(titlestroke, agg::rgba(0, 0, 0));
}

template<class VS, class RM>
void plot<VS,RM>::draw_elements(canvas &canvas, agg::trans_affine& canvas_mtx)
{
  agg::trans_affine m = m_trans;
  viewport_scale(m);

  trans_affine_compose (m, canvas_mtx);

  for (unsigned j = 0; j < m_elements.size(); j++)
    {
      container& d = m_elements[j];
      VS& vs = d.get_vertex_source();
      vs.apply_transform(m, 1.0);

      if (d.outline)
	canvas.draw_outline(vs, d.color);
      else
	canvas.draw(vs, d.color);
    }
}

template<class VS, class RM>
void plot<VS,RM>::update_viewport_trans()
{
  double xi, yi, xs, ys;

  if (m_use_units)
    {
      int ixi, ixs, iyi, iys;
      double xd, yd;
      m_ux.limits(ixi, ixs, xd);
      xi = ixi * xd;
      xs = ixs * xd;

      m_uy.limits(iyi, iys, yd);
      yi = iyi * yd;
      ys = iys * yd;
    }
  else
    {
      xi = m_x1;
      yi = m_y1;
      xs = m_x2;
      ys = m_y2;
    }

  double fx = 1/(xs - xi), fy = 1/(ys - yi);
  this->m_trans = agg::trans_affine(fx, 0.0, 0.0, fy, -xi*fx, -yi*fy);
}

template<class VS, class RM>
void plot<VS,RM>::trans_matrix_update()
  {
    if (this->m_bbox_updated)
      return;

    this->calc_bounding_box();

    m_ux = units(this->m_x1, this->m_x2);
    m_uy = units(this->m_y1, this->m_y2);

    this->update_viewport_trans();
    this->m_bbox_updated = true;
  }

template<class VS, class RM>
void plot<VS,RM>::calc_bounding_box()
{
  bool is_set = false;

  for (unsigned j = 0; j < m_elements.size(); j++)
  {
    container& d = m_elements[j];

    double sx1, sy1, sx2, sy2;
    d.vs->bounding_box(&sx1, &sy1, &sx2, &sy2);
      
    if (! is_set)
    {
      m_x1 = sx1;
      m_x2 = sx2;
      m_y1 = sy1;
      m_y2 = sy2;

      is_set = true;
    }
    else
    {
      bbox_enlarge(&m_x1, &m_y1, &m_x2, &m_y2, sx1, sy1);
      bbox_enlarge(&m_x1, &m_y1, &m_x2, &m_y2, sx2, sy2);
    }
  }
}

template <class VS, class RM>
void plot<VS,RM>::draw_axis(canvas &canvas, agg::trans_affine& canvas_mtx)
{
  typedef agg::path_storage path_type;
  typedef agg::conv_dash<agg::conv_transform<path_type>, agg::vcgen_markers_term> dash_type;

  agg::trans_affine m;
  this->viewport_scale(m);
  trans_affine_compose (m, canvas_mtx);

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
	labs.width(1.5);

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
	labs.width(1.5);

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
  canvas.draw(lns, agg::rgba(0.0, 0.0, 0.0));

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

template<class VS, class RM>
void plot<VS,RM>::viewport_scale(agg::trans_affine& m)
{
  const double xoffs = 0.09375, yoffs = 0.09375;
  static agg::trans_affine rsz(1-2*xoffs, 0.0, 0.0, 1-2*yoffs, xoffs, yoffs);
  trans_affine_compose (m, rsz);
}

template<class VS, class RM>
void plot<VS,RM>::set_units(bool use_units)
{ 
  m_use_units = use_units; 
  this->update_viewport_trans();
}

#endif
