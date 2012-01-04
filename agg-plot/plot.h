
/* plot.h
 *
 * Copyright (C) 2009-2011 Francesco Abbate
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

#ifndef AGGPLOT_PLOT_H
#define AGGPLOT_PLOT_H

#include <new>

#include "utils.h"
#include "my_list.h"
#include "strpp.h"
#include "canvas.h"
#include "units.h"
#include "resource-manager.h"
#include "colors.h"
#include "rect.h"
#include "canvas_svg.h"
#include "trans.h"
#include "text.h"

#include "agg_array.h"
#include "agg_bounding_rect.h"
#include "agg_vcgen_markers_term.h"
#include "agg_conv_transform.h"
#include "agg_color_rgba.h"
#include "agg_path_storage.h"
#include "agg_array.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_gsv_text.h"

template <class VertexSource>
struct virtual_canvas {
  virtual void draw(VertexSource& vs, agg::rgba8 c) = 0;
  virtual void draw_outline(VertexSource& vs, agg::rgba8 c) = 0;

  virtual void clip_box(const agg::rect_base<int>& clip) = 0;
  virtual void reset_clipping() = 0;

  virtual ~virtual_canvas() { }
};

template <class Canvas, class VertexSource>
class canvas_adapter : public virtual_canvas<VertexSource> {
public:
  canvas_adapter(Canvas* c) : m_canvas(c) {}

  virtual void draw(VertexSource& vs, agg::rgba8 c) { m_canvas->draw(vs, c); }
  virtual void draw_outline(VertexSource& vs, agg::rgba8 c) { m_canvas->draw_outline(vs, c); }

  virtual void clip_box(const agg::rect_base<int>& clip) { m_canvas->clip_box(clip);  }
  virtual void reset_clipping() { m_canvas->reset_clipping(); }

private:
  Canvas* m_canvas;
};

template<class VertexSource>
struct plot_item {
  VertexSource* vs;
  agg::rgba8 color;
  bool outline;

  plot_item() : vs(0) {};

  plot_item(VertexSource* vs, agg::rgba8& c, bool as_outline):
    vs(vs), color(c), outline(as_outline)
  {};

  VertexSource& content() { return *vs; };
};

template<class VertexSource, class ResourceManager>
class plot {
  typedef plot_item<VertexSource> item;
  typedef agg::pod_bvector<item> item_list;

  static const unsigned max_layers = 8;

public:
  typedef pod_list<item> iterator;
  typedef virtual_canvas<VertexSource> canvas_type;

  enum axis_e { x_axis, y_axis };

  plot(bool use_units = true) :
    m_root_layer(), m_layers(), m_current_layer(&m_root_layer),
    m_drawing_queue(0), m_clip_flag(true),
    m_need_redraw(true), m_rect(),
    m_use_units(use_units), m_pad_units(false), m_title(),
    m_xlabel(), m_ylabel(), m_sync_mode(true),
    m_xlabels_angle(0.0), m_ylabels_angle(0.0),
    m_xlabels_hjustif(0.5), m_xlabels_vjustif(1.0),
    m_ylabels_hjustif(1.0), m_ylabels_vjustif(0.5)
  {
    compute_user_trans();
  };

  virtual ~plot()
  {
    layer_dispose_elements(m_root_layer);
    for (unsigned k = 0; k < m_layers.size(); k++)
      {
	item_list *layer = m_layers[k];
	layer_dispose_elements(*layer);
	delete layer;
      }
  };

  str& title() { return m_title; }
  str& x_axis_title() { return m_xlabel; }
  str& y_axis_title() { return m_ylabel; }

  void set_axis_labels_angle(enum axis_e axis, double angle);

  double get_axis_labels_angle(enum axis_e axis) const
  {
    return (axis == x_axis ? m_xlabels_angle : m_ylabels_angle);
  }

  void set_units(bool use_units);
  bool use_units() const { return m_use_units; };

  void set_limits(const agg::rect_base<double>& r);

  virtual void add(VertexSource* vs, agg::rgba8& color, bool outline);
  virtual void before_draw() { };

  template <class Canvas>
  void draw(Canvas& canvas, agg::trans_affine& m);

  virtual bool push_layer();
  virtual bool pop_layer();

  /* drawing queue related methods */
  void push_drawing_queue();
  void clear_drawing_queue();
  void clear_current_layer();
  int current_layer_index();

  bool clip_is_active() const { return m_clip_flag; };
  void set_clip_mode(bool flag) { m_clip_flag = flag; };

  bool need_redraw() const { return m_need_redraw; };
  void commit_pending_draw();

  template <class Canvas>
  void draw_queue(Canvas& canvas, agg::trans_affine& m, opt_rect<double>& bbox);

  void sync_mode(bool req_mode) { m_sync_mode = req_mode; };
  bool sync_mode() const { return m_sync_mode; };

  void pad_mode(bool req)
  {
    if (req != m_pad_units)
      {
	m_pad_units = req;
	m_need_redraw = true;
	compute_user_trans();
      }
  };

  bool pad_mode() const { return m_pad_units; };

protected:
  void draw_elements(canvas_type &canvas, agg::trans_affine& m);
  void draw_element(item& c, canvas_type &canvas, agg::trans_affine& m);
  void draw_axis(canvas_type& can, agg::trans_affine& m);

  agg::trans_affine get_scaled_matrix(agg::trans_affine& canvas_mtx);

  void clip_plot_area(canvas_type& canvas, agg::trans_affine& canvas_mtx);

  void compute_user_trans();

  agg::trans_affine viewport_scale(agg::trans_affine& m);

  bool fit_inside(VertexSource *obj) const;

  void layer_dispose_elements (item_list& layer);

  item_list& current_layer() { return *m_current_layer; };

  item_list m_root_layer;
  agg::pod_auto_vector<item_list*, max_layers> m_layers;
  item_list *m_current_layer;

  agg::trans_affine m_trans;
  pod_list<item> *m_drawing_queue;

  bool m_clip_flag;

  bool m_need_redraw;
  opt_rect<double> m_rect;

  bool m_use_units;
  units m_ux, m_uy;

private:
  bool m_pad_units;

  str m_title, m_xlabel, m_ylabel;
  double m_left_margin, m_right_margin;
  double m_bottom_margin, m_top_margin;

  bool m_sync_mode;
  double m_xlabels_angle, m_ylabels_angle;
  double m_xlabels_hjustif, m_xlabels_vjustif;
  double m_ylabels_hjustif, m_ylabels_vjustif;
};

static double compute_scale(agg::trans_affine& m)
{
  return m.scale() / 480.0;
}

static double
std_line_width(double scale, double w = 1.0)
{
#if 0
  const double dsf = M_LN10;
  double ls = log(scale) / dsf;
  return exp(round(ls) * dsf) * w * 1.5;
#else
  return w * 1.5;
#endif
}

template <class VS, class RM>
void plot<VS,RM>::commit_pending_draw()
{
  push_drawing_queue();
  m_need_redraw = false;
}

template <class VS, class RM>
void plot<VS,RM>::add(VS* vs, agg::rgba8& color, bool outline)
{
  item d(vs, color, outline);
  pod_list<item> *new_node = new pod_list<item>(d);
  m_drawing_queue = pod_list<item>::push_back(m_drawing_queue, new_node);
  RM::acquire(vs);
}

template <class VS, class RM>
void plot<VS,RM>::push_drawing_queue()
{
  for (pod_list<item> *c = m_drawing_queue; c != 0; c = c->next())
    {
      m_current_layer->add(c->content());
    }

  while (m_drawing_queue)
    m_drawing_queue = list::pop(m_drawing_queue);
}

template <class VS, class RM>
void plot<VS,RM>::clear_drawing_queue()
{
  while (m_drawing_queue)
    {
      item& d = m_drawing_queue->content();
      RM::dispose(d.vs);
      m_drawing_queue = list::pop(m_drawing_queue);
    }
}

template <class VS, class RM>
template <class Canvas> void plot<VS,RM>::draw(Canvas& _canvas, agg::trans_affine& canvas_mtx)
{
  canvas_adapter<Canvas, VS>  canvas(&_canvas);
  before_draw();
  draw_axis(canvas, canvas_mtx);
  draw_elements(canvas, canvas_mtx);
};

template <class VS, class RM>
void plot<VS,RM>::draw_element(item& c, canvas_type& canvas, agg::trans_affine& m)
{
  VS& vs = c.content();
  vs.apply_transform(m, 1.0);

  if (c.outline)
    canvas.draw_outline(vs, c.color);
  else
    canvas.draw(vs, c.color);
}

template <class VS, class RM>
agg::trans_affine plot<VS,RM>::get_scaled_matrix(agg::trans_affine& canvas_mtx)
{
  agg::trans_affine mvp = viewport_scale(canvas_mtx);
  agg::trans_affine m = m_trans;
  trans_affine_compose (m, mvp);
  return m;
}

template<class VS, class RM>
void plot<VS,RM>::clip_plot_area(canvas_type& canvas, agg::trans_affine& canvas_mtx)
{
  if (this->clip_is_active())
    {
      agg::trans_affine mvp = viewport_scale(canvas_mtx);
      agg::rect_base<int> clip = rect_of_slot_matrix<int>(mvp);
      canvas.clip_box(clip);
    }
}

template <class VS, class RM>
void plot<VS,RM>::draw_elements(canvas_type& canvas, agg::trans_affine& canvas_mtx)
{
  agg::trans_affine m = get_scaled_matrix(canvas_mtx);

  this->clip_plot_area(canvas, canvas_mtx);

  for (unsigned j = 0; j < m_root_layer.size(); j++)
    {
      draw_element(m_root_layer[j], canvas, m);
    }

  for (unsigned k = 0; k < m_layers.size(); k++)
    {
      item_list& layer = *(m_layers[k]);
      for (unsigned j = 0; j < layer.size(); j++)
	{
	  draw_element(layer[j], canvas, m);
	}
    }

  canvas.reset_clipping();
}

template <class VS, class RM>
template <class Canvas> void plot<VS,RM>::draw_queue(Canvas& _canvas, agg::trans_affine& canvas_mtx, opt_rect<double>& bb)
{
  canvas_adapter<Canvas, VS>  canvas(&_canvas);

  typedef typename plot<VS,RM>::iterator iter_type;

  before_draw();

  this->clip_plot_area(canvas, canvas_mtx);

  iter_type *c0 = m_drawing_queue;
  for (iter_type *c = c0; c != 0; c = c->next())
    {
      item& d = c->content();
      agg::trans_affine m = get_scaled_matrix(canvas_mtx);
      draw_element(d, canvas, m);

      agg::rect_base<double> ebb;
      bool not_empty = agg::bounding_rect_single(d.content(), 0, &ebb.x1, &ebb.y1, &ebb.x2, &ebb.y2);

      if (not_empty)
	bb.add<rect_union>(ebb);
    }

  canvas.reset_clipping();
}

template <class VS, class RM>
void plot<VS,RM>::compute_user_trans()
{
  agg::rect_base<double> r;

  if (m_use_units && m_pad_units)
    {
      int ixi, ixs, iyi, iys;
      double xd, yd;
      m_ux.limits(ixi, ixs, xd);
      r.x1 = ixi * xd;
      r.x2 = ixs * xd;

      m_uy.limits(iyi, iys, yd);
      r.y1 = iyi * yd;
      r.y2 = iys * yd;
    }
  else
    {
      r = m_rect.is_defined() ? m_rect.rect() : agg::rect_base<double>(0.0, 0.0, 1.0, 1.0);
    }

  double dx = r.x2 - r.x1, dy = r.y2 - r.y1;
  double fx = (dx == 0 ? 1.0 : 1/dx), fy = (dy == 0 ? 1.0 : 1/dy);
  this->m_trans = agg::trans_affine(fx, 0.0, 0.0, fy, -r.x1 * fx, -r.y1 * fy);
}

template <class VS, class RM>
void plot<VS,RM>::draw_axis(canvas_type& canvas, agg::trans_affine& canvas_mtx)
{
  if (!m_use_units)
    {
      const double pad = 10.0;
      m_left_margin   = pad;
      m_right_margin  = pad;
      m_bottom_margin = pad;
      m_top_margin    = pad;
      return;
    }

  agg::trans_affine m;

  double scale = compute_scale(canvas_mtx);

  agg::rect_base<int> clip = rect_of_slot_matrix<int>(canvas_mtx);
  canvas.clip_box(clip);

  agg::path_storage mark;
  sg_object_gen<agg::conv_transform<agg::path_storage> > mark_tr(mark, m);
  trans::stroke_a mark_stroke(&mark_tr);

  agg::path_storage ln;
  sg_object_gen<agg::conv_transform<agg::path_storage> > ln_tr(ln, m);
  trans::dash_a lndash(&ln_tr);
  trans::stroke_a lns(&lndash);

  const double yeps = 1.0e-3;
  const double xeps = 1.0e-3;

  const double line_width = std_line_width(scale);
  const double label_text_size = 11.0 * scale;
  const double title_text_size = 12.0 * scale;
  const double ppad = 0.02, fpad = 4;

  agg::pod_bvector<draw::text*> y_labels, x_labels;
  double dx_label, dy_label;

  {
    int jinf = m_uy.begin(), jsup = m_uy.end();

    opt_rect<double> ybox;
    agg::rect_base<double> r;

    for (int j = jinf; j <= jsup; j++)
      {
	double x = 0.0, y = m_uy.mark_value(j);
	this->m_trans.transform(&x, &y);
	if (y >= - yeps && y <= 1.0 + yeps)
	  {
	    char lab_text[32];
	    m_uy.mark_label(lab_text, 32, j);

	    draw::text* label = new draw::text(lab_text, 10.0 * scale,
					       line_width,
					       m_ylabels_hjustif,
					       m_ylabels_vjustif);
	    label->angle(m_ylabels_angle);
	    label->set_point(-ppad, y);

	    mark.move_to(0.0, y);
	    mark.line_to(-0.01, y);

	    if (j > jinf && j < jsup) {
	      ln.move_to(0.0, y);
	      ln.line_to(1.0, y);
	    }

	    agg::bounding_rect_single(*label, 0, &r.x1, &r.y1, &r.x2, &r.y2);
	    ybox.add<rect_union>(r);

	    y_labels.add(label);
	  }
      }

    const agg::rect_base<double>& ybr = ybox.rect();
    dx_label = ybr.x2 - ybr.x1;
  }

  {
    int jinf = m_ux.begin(), jsup = m_ux.end();

    opt_rect<double> xbox;
    agg::rect_base<double> r;
    agg::pod_bvector<draw::text*> labels;

    for (int j = jinf; j <= jsup; j++)
      {
	double x = m_ux.mark_value(j), y = 0.0;
	this->m_trans.transform(&x, &y);
	if (x >= - xeps && x <= 1.0 + xeps)
	  {
	    char lab_text[32];
	    m_ux.mark_label(lab_text, 32, j);

	    draw::text* label = new draw::text(lab_text, 10.0 * scale,
					       line_width,
					       m_xlabels_hjustif,
					       m_xlabels_vjustif);
	    label->angle(m_xlabels_angle);
	    label->set_point(x, -ppad);

	    mark.move_to(x, 0.0);
	    mark.line_to(x, -0.01);

	    if (j > jinf && j < jsup) {
	      ln.move_to(x, 0.0);
	      ln.line_to(x, 1.0);
	    }

	    agg::bounding_rect_single(*label, 0, &r.x1, &r.y1, &r.x2, &r.y2);
	    xbox.add<rect_union>(r);

	    x_labels.add(label);
	  }
      }

    const agg::rect_base<double>& xbr = xbox.rect();
    dy_label = xbr.y2 - xbr.y1;
  }

  const double sx = fabs(canvas_mtx.sx), sy = fabs(canvas_mtx.sy);
  const int ysign = (canvas_mtx.sy < 0.0 ? -1 : 1);
  const double lsx = (dx_label + 4 * ppad * sx + 2 * fpad) / (1 + 4 * ppad);
  const double lsy = (dy_label + 4 * ppad * sy + 2 * fpad) / (1 + 4 * ppad);

  m_left_margin   = (lsx + dx_label) / 2.0;
  m_right_margin  = (lsx - dx_label) / 2.0;
  m_bottom_margin = (lsy + dy_label) / 2.0;
  m_top_margin    = (lsy - dy_label) / 2.0;

  const double sxr = sx - lsx;
  const double syr = sy - lsy;

  if (!str_is_null(&m_ylabel))
    m_left_margin += label_text_size + 2*sxr*ppad;

  if (!str_is_null(&m_xlabel))
    m_bottom_margin += label_text_size + 2*syr*ppad;

  if (!str_is_null(&m_title))
    m_top_margin += title_text_size + 2*syr*ppad;

  m = this->viewport_scale(canvas_mtx);

  for (unsigned j = 0; j < x_labels.size(); j++)
    {
      draw::text* label = x_labels[j];
      label->apply_transform(m, 1.0);
      canvas.draw(*label, agg::rgba(0, 0, 0));
      delete label;
    }

  for (unsigned j = 0; j < y_labels.size(); j++)
    {
      draw::text* label = y_labels[j];
      label->apply_transform(m, 1.0);
      canvas.draw(*label, agg::rgba(0, 0, 0));
      delete label;
    }

  lndash.add_dash(7.0, 3.0);

  lns.width(std_line_width(scale, 0.25));
  canvas.draw(lns, colors::black);

  mark_stroke.width(std_line_width(scale, 0.75));
  canvas.draw(mark_stroke, colors::black);

  agg::path_storage box;
  sg_object_gen<agg::conv_transform<agg::path_storage> > boxtr(box, m);
  trans::stroke_a boxvs(&boxtr);

  box.move_to(0.0, 0.0);
  box.line_to(0.0, 1.0);
  box.line_to(1.0, 1.0);
  box.line_to(1.0, 0.0);
  box.close_polygon();

  boxvs.width(std_line_width(scale));
  canvas.draw(boxvs, colors::black);

  if (!str_is_null(&m_xlabel))
    {
      double labx = 0.5, _laby = 0.0;
      m.transform(&labx, &_laby);

      double _labx = 0.5, laby = 0.0;
      canvas_mtx.transform(&_labx, &laby);

      draw::text xlabel(m_xlabel.cstr(), label_text_size, line_width, 0.5, 0.0);
      xlabel.set_point(labx, laby + ysign * (syr*ppad + fpad));
      xlabel.apply_transform(identity_matrix, 1.0);

      canvas.draw(xlabel, colors::black);
    }

  if (!str_is_null(&m_ylabel))
    {
      double labx = 0.0, laby = 0.5;
      m.transform(&labx, &laby);

      draw::text ylabel(m_ylabel.cstr(), label_text_size, line_width, 0.5, 1.0);
      ylabel.set_point(sxr*ppad + fpad, laby);
      ylabel.angle(M_PI/2.0);
      ylabel.apply_transform(identity_matrix, 1.0);

      canvas.draw(ylabel, colors::black);
    }

  if (!str_is_null(&m_title))
    {
      double labx = 0.5, laby = 1.0;
      m.transform(&labx, &laby);

      draw::text title(m_title.cstr(), title_text_size, line_width, 0.5, 0.0);
      title.set_point(labx, laby + ysign * (2*syr*ppad));
      title.apply_transform(identity_matrix, 1.0);

      canvas.draw(title, colors::black);
    }

  canvas.reset_clipping();
};

template<class VS, class RM>
agg::trans_affine plot<VS,RM>::viewport_scale(agg::trans_affine& canvas_mtx)
{
  agg::trans_affine m;
  int ysign = (canvas_mtx.sy < 0.0 ? -1 : 1);
  m.sx = canvas_mtx.sx - (m_left_margin + m_right_margin);
  m.sy = canvas_mtx.sy - ysign * (m_top_margin + m_bottom_margin);
  m.shx = 0.0;
  m.shy = 0.0;
  m.tx = m_left_margin + canvas_mtx.tx;
  m.ty = ysign * m_bottom_margin + canvas_mtx.ty;
  return m;
}

template<class VS, class RM>
void plot<VS,RM>::set_axis_labels_angle(enum axis_e axis, double angle)
{
  if (axis == x_axis)
    {
      double c = sin(angle), s = cos(angle);
      m_xlabels_hjustif = round(c + 1.0) / 2.0;
      m_xlabels_vjustif = round(s + 1.0) / 2.0;
      m_xlabels_angle = angle;
    }
  else
    {
      double c = cos(angle), s = -sin(angle);
      m_ylabels_hjustif = round(c + 1.0) / 2.0;
      m_ylabels_vjustif = round(s + 1.0) / 2.0;
      m_ylabels_angle = angle;
    }

  m_need_redraw = true;
  compute_user_trans();
}

template<class VS, class RM>
void plot<VS,RM>::set_units(bool use_units)
{
  if (m_use_units != use_units)
    {
      m_use_units = use_units;
      m_need_redraw = true;
      compute_user_trans();
    }
}

template<class VS, class RM>
void plot<VS,RM>::set_limits(const agg::rect_base<double>& r)
{
  m_rect.set(r);
  m_ux = units(r.x1, r.x2);
  m_uy = units(r.y1, r.y2);
  m_need_redraw = true;
  compute_user_trans();
}

template<class VS, class RM>
void plot<VS,RM>::layer_dispose_elements(plot<VS,RM>::item_list& layer)
{
  unsigned n = layer.size();
  for (unsigned k = 0; k < n; k++)
    {
      RM::dispose(layer[k].vs);
    }
}

template<class VS, class RM>
bool plot<VS,RM>::push_layer()
{
  if (m_layers.size() >= max_layers)
    return false;

  item_list *new_layer = new(std::nothrow) item_list();
  if (new_layer == 0)
    return false;

  commit_pending_draw();
  m_layers.add(new_layer);
  m_current_layer = new_layer;

  return true;
}

template<class VS, class RM>
bool plot<VS,RM>::pop_layer()
{
  unsigned n = m_layers.size();

  if (n == 0)
    return false;

  item_list *layer = m_layers[n-1];
  layer_dispose_elements (*layer);
  delete layer;

  m_layers.inc_size(-1);
  n--;

  clear_drawing_queue();
  m_need_redraw = true;
  m_current_layer = (n > 0 ? m_layers[n-1] : &m_root_layer);

  return true;
}

template <class VS, class RM>
void plot<VS,RM>::clear_current_layer()
{
  clear_drawing_queue();
  layer_dispose_elements (current_layer());
  m_current_layer->clear();
}

template <class VS, class RM>
int plot<VS,RM>::current_layer_index()
{
  return m_layers.size();
}

#endif
