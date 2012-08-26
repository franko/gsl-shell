
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
#include "list.h"
#include "strpp.h"
#include "canvas.h"
#include "units.h"
#include "resource-manager.h"
#include "colors.h"
#include "rect.h"
#include "canvas_svg.h"
#include "trans.h"
#include "text.h"
#include "categories.h"
#include "sg_object.h"

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

template <class Canvas>
class canvas_adapter : public virtual_canvas {
public:
  canvas_adapter(Canvas* c) : m_canvas(c) {}

  virtual void draw(sg_object& vs, agg::rgba8 c) { m_canvas->draw(vs, c); }
  virtual void draw_outline(sg_object& vs, agg::rgba8 c) { m_canvas->draw_outline(vs, c); }

  virtual void clip_box(const agg::rect_base<int>& clip) { m_canvas->clip_box(clip);  }
  virtual void reset_clipping() { m_canvas->reset_clipping(); }

private:
  Canvas* m_canvas;
};

struct plot_item {
  sg_object* vs;
  agg::rgba8 color;
  bool outline;

  plot_item() : vs(0) {};

  plot_item(sg_object* vs, agg::rgba8& c, bool as_outline):
    vs(vs), color(c), outline(as_outline)
  {};

  sg_object& content() { return *vs; };
};

template<class ResourceManager>
class plot {

  static const unsigned max_layers = 8;

  enum {
    axis_label_prop_space = 20,
    axis_title_prop_space = 30,
    canvas_margin_prop_space = 15,
    canvas_margin_fixed_space = 4,
  };

protected:
  typedef plot_item item;

  class item_list : public agg::pod_bvector<item>
  {
  public:
    item_list(): agg::pod_bvector<item>() { }

    const opt_rect<double>& bounding_box() const { return m_bbox; }
    void set_bounding_box(const agg::rect_base<double>& r) { m_bbox.set(r); }
    void clear_bounding_box() { m_bbox.clear(); }

  private:
    opt_rect<double> m_bbox;
  };

public:
  typedef list<item> iterator;
  typedef virtual_canvas canvas_type;

  enum axis_e { x_axis, y_axis };
  enum placement_e { right = 0, left = 1, bottom = 2, top = 3 };

  struct axis {
    str title;
    axis_e dir;
    bool use_categories;
    category_map categories;
    units::format_e format_tag;

    axis(axis_e _dir, const char* _title = 0):
      title(_title ? _title : ""), dir(_dir), use_categories(false),
      format_tag(units::format_invalid),
      m_labels_angle(0.0),
      m_labels_hjustif(_dir == x_axis ? 0.5 : 1.0),
      m_labels_vjustif(_dir == x_axis ? 1.0 : 0.5)
    { }

    const char* label_format() const
    {
      return (format_tag == units::format_invalid ? 0 : m_label_format);
    }

    void set_label_format(units::format_e tag, const char* fmt)
    {
      format_tag = tag;
      memcpy(m_label_format, fmt, strlen(fmt) + 1);
    }

    void clear_label_format() { format_tag = units::format_invalid; }

    void set_labels_angle(double angle)
    {
      double a = (dir == x_axis ? -angle + M_PI/2 : -angle);
      double c = cos(a), s = sin(a);
      m_labels_hjustif = round(c + 1.0) / 2.0;
      m_labels_vjustif = round(s + 1.0) / 2.0;
      m_labels_angle = angle;
    }

    double labels_angle()   const { return m_labels_angle; }
    double labels_hjustif() const { return m_labels_hjustif; }
    double labels_vjustif() const { return m_labels_vjustif; }

  private:
    double m_labels_angle;
    double m_labels_hjustif, m_labels_vjustif;
    char m_label_format[units::label_format_max_size];
  };

  plot(bool use_units = true) :
    m_drawing_queue(0), m_clip_flag(true),
    m_need_redraw(true), m_rect(),
    m_use_units(use_units), m_pad_units(false), m_title(),
    m_sync_mode(true), m_x_axis(x_axis), m_y_axis(y_axis)
  {
    m_layers.add(&m_root_layer);
    compute_user_trans();
    for (unsigned k = 0; k < 4; k++)
      m_legend[k] = 0;
  };

  virtual ~plot()
  {
    for (unsigned k = 0; k < m_layers.size(); k++)
      {
        item_list *layer = m_layers[k];
        layer_dispose_elements(layer);
        if (k > 0)
          delete layer;
      }
  };

  str& title() { return m_title; }
  str& x_axis_title() { return m_x_axis.title; }
  str& y_axis_title() { return m_y_axis.title; }

  void add_legend(plot* p, placement_e where) { m_legend[where] = p; }
  plot* get_legend(placement_e where) { return m_legend[where]; }

  axis& get_axis(axis_e axis_dir)
  {
    return (axis_dir == x_axis ? m_x_axis : m_y_axis);
  }

  const axis& get_axis(axis_e axis_dir) const
  {
    return (axis_dir == x_axis ? m_x_axis : m_y_axis);
  }

  void set_axis_labels_angle(axis_e axis, double angle);

  double get_axis_labels_angle(axis_e axis_dir) const
  {
    return get_axis(axis_dir).labels_angle();
  }

  void set_units(bool use_units);
  bool use_units() const { return m_use_units; };

  void update_units();
  void set_limits(const agg::rect_base<double>& r);
  void unset_limits();

  virtual void add(sg_object* vs, agg::rgba8& color, bool outline);
  virtual void before_draw() { }

  void get_bounding_rect(agg::rect_base<double>& bb)
  {
    before_draw();

    if (m_rect.is_defined())
      bb = m_rect.rect();
    else
      bb = agg::rect_base<double>(0.0, 0.0, 0.0, 0.0);
  }

  template <class Canvas>
  void draw(Canvas& canvas, const agg::trans_affine& m);

  virtual bool push_layer();
  virtual bool pop_layer();
  virtual void clear_current_layer();

  /* drawing queue related methods */
  void push_drawing_queue();
  void clear_drawing_queue();
  int current_layer_index();

  bool clip_is_active() const { return m_clip_flag; };
  void set_clip_mode(bool flag) { m_clip_flag = flag; };

  bool need_redraw() const { return m_need_redraw; };
  void commit_pending_draw();

  template <class Canvas>
  void draw_queue(Canvas& canvas, const agg::trans_affine& m, opt_rect<double>& bbox);

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

  bool pad_mode() const { return m_pad_units; }

  bool enable_label_format(axis_e dir, const char* fmt)
  {
    if (!fmt)
    {
      get_axis(dir).clear_label_format();
      return true;
    }

    units::format_e tag = units::parse_label_format(fmt);
    if (tag == units::format_invalid)
      return false;
    get_axis(dir).set_label_format(tag, fmt);
    return true;
  }

  void enable_categories(axis_e dir) { get_axis(dir).use_categories = true; }

  void disable_categories(axis_e dir)
  {
    axis& ax = get_axis(dir);
    ax.use_categories = false;
    ax.categories.clear();
  }

  void add_category_entry(axis_e dir, double v, const char* text)
  {
    axis& ax = get_axis(dir);
    ax.categories.add_item(v, text);
  }

protected:
  double draw_axis_m(axis_e dir, units& u, const agg::trans_affine& user_mtx,
                     ptr_list<draw::text>& labels, double scale,
                     agg::path_storage& mark, agg::path_storage& ln);

  void draw_elements(canvas_type &canvas, const agg::trans_affine& m);
  void draw_element(item& c, canvas_type &canvas, const agg::trans_affine& m);
  void draw_axis(canvas_type& can, const agg::trans_affine& m, agg::rect_base<int>* clip = 0);

  agg::trans_affine draw_legends(canvas_type& canvas,
                                    const agg::trans_affine& canvas_mtx);

  agg::trans_affine get_scaled_matrix(const agg::trans_affine& canvas_mtx);

  void clip_plot_area(canvas_type& canvas, const agg::trans_affine& canvas_mtx);

  void compute_user_trans();

  bool fit_inside(sg_object *obj) const;

  void layer_dispose_elements (item_list* layer);

  unsigned nb_layers() const { return m_layers.size(); }
  item_list* get_layer(unsigned j) { return m_layers[j]; }

  item_list* current_layer() { return m_layers[m_layers.size() - 1]; };

  item_list* parent_layer()
  {
    unsigned n = m_layers.size();
    return (n > 1 ? m_layers[n-2] : 0);
  }

  agg::trans_affine m_trans;
  list<item> *m_drawing_queue;

  bool m_clip_flag;

  bool m_need_redraw;
  opt_rect<double> m_rect;

  bool m_use_units;
  units m_ux, m_uy;

private:
  item_list m_root_layer;
  agg::pod_auto_vector<item_list*, max_layers> m_layers;

  bool m_pad_units;

  str m_title;
  agg::trans_affine m_area_mtx;

  bool m_sync_mode;

  axis m_x_axis, m_y_axis;
  plot* m_legend[4];
};

static double compute_scale(const agg::trans_affine& m)
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

template <class RM>
void plot<RM>::commit_pending_draw()
{
  push_drawing_queue();
  m_need_redraw = false;
}

template <class RM>
void plot<RM>::add(sg_object* vs, agg::rgba8& color, bool outline)
{
  item d(vs, color, outline);
  list<item> *new_node = new list<item>(d);
  m_drawing_queue = list<item>::push_back(m_drawing_queue, new_node);
  RM::acquire(vs);
}

template <class RM>
void plot<RM>::push_drawing_queue()
{
  item_list* layer = current_layer();
  for (list<item> *c = m_drawing_queue; c != 0; c = c->next())
    {
      layer->add(c->content());
    }

  while (m_drawing_queue)
    m_drawing_queue = list<item>::pop(m_drawing_queue);
}

template <class RM>
void plot<RM>::clear_drawing_queue()
{
  while (m_drawing_queue)
    {
      item& d = m_drawing_queue->content();
      RM::dispose(d.vs);
      m_drawing_queue = list<item>::pop(m_drawing_queue);
    }
}

static bool area_is_valid(const agg::trans_affine& b)
{
  const double thresold = 40.0;
  return (b.sx > thresold && b.sy > thresold);
}

template <class RM>
template <class Canvas> void plot<RM>::draw(Canvas& _canvas, const agg::trans_affine& canvas_mtx)
{
  canvas_adapter<Canvas>  canvas(&_canvas);
  before_draw();

  agg::rect_base<int> clip = rect_of_slot_matrix<int>(canvas_mtx);

  agg::trans_affine area_mtx = draw_legends(canvas, canvas_mtx);

  if (area_is_valid(area_mtx))
    {
      draw_axis(canvas, area_mtx, &clip);
      draw_elements(canvas, area_mtx);
    }
};

template <class RM>
void plot<RM>::draw_element(item& c, canvas_type& canvas, const agg::trans_affine& m)
{
  sg_object& vs = c.content();
  vs.apply_transform(m, 1.0);

  if (c.outline)
    canvas.draw_outline(vs, c.color);
  else
    canvas.draw(vs, c.color);
}

template <class RM>
agg::trans_affine plot<RM>::get_scaled_matrix(const agg::trans_affine& canvas_mtx)
{
  agg::trans_affine m = m_trans;
  trans_affine_compose (m, m_area_mtx);
  return m;
}

template<class RM>
void plot<RM>::clip_plot_area(canvas_type& canvas, const agg::trans_affine& canvas_mtx)
{
  if (this->clip_is_active())
    {
      agg::rect_base<int> clip = rect_of_slot_matrix<int>(m_area_mtx);
      canvas.clip_box(clip);
    }
}

template <class RM>
void plot<RM>::draw_elements(canvas_type& canvas, const agg::trans_affine& canvas_mtx)
{
  const agg::trans_affine m = get_scaled_matrix(canvas_mtx);

  this->clip_plot_area(canvas, canvas_mtx);

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

template <class RM>
template <class Canvas> void plot<RM>::draw_queue(Canvas& _canvas, const agg::trans_affine& canvas_mtx, opt_rect<double>& bb)
{
  canvas_adapter<Canvas>  canvas(&_canvas);

  typedef typename plot<RM>::iterator iter_type;

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

template <class RM>
void plot<RM>::compute_user_trans()
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

template <class RM>
double plot<RM>::draw_axis_m(axis_e dir, units& u,
                             const agg::trans_affine& user_mtx,
                             ptr_list<draw::text>& labels, double scale,
                             agg::path_storage& mark, agg::path_storage& ln)
{
  const double ppad = double(axis_label_prop_space) / 1000.0;
  const double eps = 1.0e-3;

  opt_rect<double> bb;
  agg::rect_base<double> r;

  bool isx = (dir == x_axis);

  const axis& ax = get_axis(dir);
  double hj = ax.labels_hjustif(), vj = ax.labels_vjustif();
  double langle = ax.labels_angle();

  category_map::iterator clabels(ax.categories);
  units_iterator ulabels(u, ax.format_tag, ax.label_format());

  label_iterator* ilabels = (ax.use_categories ? (label_iterator*) &clabels : (label_iterator*) &ulabels);

  double uq;
  const char* text;
  while (ilabels->next(uq, text))
    {
      double x = (isx ? uq : 0.0), y = (isx ? 0.0 : uq);
      user_mtx.transform(&x, &y);

      double q = (isx ? x : y);

      if (q < -eps || q > 1.0 + eps)
        continue;

      draw::text* label = new draw::text(text, 14.0*scale, hj, vj);

      label->set_point(isx ? q : -ppad, isx ? -ppad : q);
      label->angle(langle);

      agg::bounding_rect_single(*label, 0, &r.x1, &r.y1, &r.x2, &r.y2);
      bb.add<rect_union>(r);

      labels.add(label);

      mark.move_to(isx ? q :  0.0 , isx ?  0.0  : q);
      mark.line_to(isx ? q : -0.01, isx ? -0.01 : q);
    }

  int jinf = u.begin(), jsup = u.end();
  for (int j = jinf+1; j < jsup; j++)
    {
      double uq = u.mark_value(j);
      double x = (isx ? uq : 0), y = (isx ? 0.0 : uq);
      user_mtx.transform(&x, &y);
      double q = (isx ? x : y);

      if (q >= -eps && q <= 1.0 + eps)
        {
          ln.move_to(isx ? q : 0.0, isx ? 0.0 : q);
          ln.line_to(isx ? q : 1.0, isx ? 1.0 : q);
        }
    }

  double label_size;
  if (bb.is_defined())
    {
      const agg::rect_base<double>& br = bb.rect();
      label_size = (isx ? br.y2 - br.y1 : br.x2 - br.x1);
    }
  else
    {
      label_size = 0.0;
    }

  return label_size;
}

static inline double approx_text_height(double text_size)
{
  return text_size * 1.5;
}

template <class RM>
agg::trans_affine plot<RM>::draw_legends(canvas_type& canvas,
                                         const agg::trans_affine& canvas_mtx)
{
  const double sx = canvas_mtx.sx, sy = canvas_mtx.sy;
  const double ppad = double(canvas_margin_prop_space) / 1000.0;
  const double fpad = double(canvas_margin_fixed_space);
  const double size_frac_x = 0.125, size_frac_y = 0.05;

  double dxl, dxr, dyb, dyt;

  dxl = dxr = fpad + ppad * sx;
  dyb = dyt = fpad + ppad * sy;

  if (!str_is_null(&m_title))
    {
      const double scale = compute_scale(canvas_mtx);
      const double ptpad = double(axis_title_prop_space) / 1000.0;
      const double title_text_size = 18.0 * scale;
      const double th = approx_text_height(title_text_size);

      double x = 0.5, y = 1.0;
      canvas_mtx.transform(&x, &y);
      y -= ptpad + dyt + title_text_size;

      draw::text title(m_title.cstr(), title_text_size, 0.5, 0.0);
      title.set_point(x, y);
      title.apply_transform(identity_matrix, 1.0);

      canvas.draw(title, colors::black);

      dyt += 2 * ptpad + th;
    }

  for (int k = 0; k < 4; k++)
    {
      plot* mp = m_legend[k];

      if (mp)
        {
          agg::rect_base<double> bb;
          mp->get_bounding_rect(bb);

          double bb_dx = bb.x2 - bb.x1, bb_dy = bb.y2 - bb.y1;
          double dx, dy;
          double px, py;
          switch (k)
            {
            case right:
              dx = max(sx * size_frac_x, bb_dx);
              dy = dx * bb_dy / bb_dx;
              px = sx - dx - ppad * sx - dxr;
              py = (sy - dy) / 2;
              dxr += dx + 2 * ppad * sx;
              break;
            case left:
              dx = max(sx * size_frac_x, bb_dx);
              dy = dx * bb_dy / bb_dx;
              px = ppad * sx + dxr;
              py = (sy - dy) / 2;
              dxl += dx + 2 * ppad * sx;
              break;
            case bottom:
              dy = sy * size_frac_y;
              dx = dy * bb_dx / bb_dy;
              py = ppad * sy + dyb;
              px = (sx - dx) / 2;
              dyb += dy + 2 * ppad * sy;
              break;
            case top:
              dy = sy * size_frac_y;
              dx = dy * bb_dx / bb_dy;
              py = sy - dy - ppad * sy - dyt;
              px = (sx - dx) / 2;
              dyt += dy + 2 * ppad * sy;
              break;
            default:
              /* */;
            }

          if (px >= 0 && py >= 0 && px + dx < sx && py + dy < sy)
            {
              const double tx = canvas_mtx.tx, ty = canvas_mtx.ty;
              agg::trans_affine mtx(dx, 0.0, 0.0, dy, tx + px, ty + py);
              mp->before_draw();
              mp->draw_axis(canvas, mtx);
              mp->draw_elements(canvas, mtx);
            }
        }
    }

  double cpx = canvas_mtx.tx, cpy = canvas_mtx.ty;
  double ssx = sx - (dxl + dxr), ssy = sy - (dyb + dyt);
  return agg::trans_affine(ssx, 0.0, 0.0, ssy, cpx + dxl, cpy + dyb);
}

template <class RM>
void plot<RM>::draw_axis(canvas_type& canvas, const agg::trans_affine& canvas_mtx,
                         agg::rect_base<int>* clip)
{
  agg::trans_affine& m = m_area_mtx;

  if (!m_use_units)
    {
      m = canvas_mtx;
      return;
    }

  double scale = compute_scale(canvas_mtx);

  if (clip)
    canvas.clip_box(*clip);

  agg::path_storage box;
  sg_object_gen<agg::conv_transform<agg::path_storage> > boxtr(box, m);
  trans::stroke_a boxvs(&boxtr);

  box.move_to(0.0, 0.0);
  box.line_to(0.0, 1.0);
  box.line_to(1.0, 1.0);
  box.line_to(1.0, 0.0);
  box.close_polygon();

  agg::path_storage mark;
  sg_object_gen<agg::conv_transform<agg::path_storage> > mark_tr(mark, m);
  trans::stroke_a mark_stroke(&mark_tr);

  agg::path_storage ln;
  sg_object_gen<agg::conv_transform<agg::path_storage> > ln_tr(ln, m);
  trans::dash_a lndash(&ln_tr);
  trans::stroke_a lns(&lndash);

  const double label_text_size = 15.0 * scale;
  const double plpad = double(axis_label_prop_space) / 1000.0;
  const double ptpad = double(axis_title_prop_space) / 1000.0;

  ptr_list<draw::text> labels;

  double dy_label = draw_axis_m(x_axis, m_ux, m_trans, labels, scale, mark, ln);
  double dx_label = draw_axis_m(y_axis, m_uy, m_trans, labels, scale, mark, ln);

  double ppad_left = plpad, ppad_right = plpad;
  double ppad_bottom = plpad, ppad_top = plpad;
  double dx_left = dx_label, dx_right = 0.0;
  double dy_bottom = dy_label, dy_top = 0.0;

  if (!str_is_null(&m_y_axis.title))
    {
      dx_left += approx_text_height(label_text_size);
      ppad_left += ptpad;
    }

  if (!str_is_null(&m_x_axis.title))
    {
      dy_bottom += approx_text_height(label_text_size);
      ppad_bottom += ptpad;
    }

  const double sx = canvas_mtx.sx, sy = canvas_mtx.sy;

  const double xppad = (ppad_left + ppad_right);
  const double lsx = (dx_left + dx_right + xppad * sx) / (1 + xppad);

  const double yppad = (ppad_bottom + ppad_top);
  const double lsy = (dy_bottom + dy_top + yppad * sy) / (1 + yppad);

  const double sxr = sx - lsx;
  const double syr = sy - lsy;

  m.sx = sxr;
  m.sy = syr;
  m.tx = canvas_mtx.tx + dx_left + ppad_left * sxr;
  m.ty = canvas_mtx.ty + dy_bottom + ppad_bottom * syr;

  for (unsigned j = 0; j < labels.size(); j++)
    {
      draw::text* label = labels[j];
      label->apply_transform(m, 1.0);
      canvas.draw(*label, colors::black);
    }

  lndash.add_dash(7.0, 3.0);

  lns.width(std_line_width(scale, 0.15));
  canvas.draw(lns, colors::black);

  mark_stroke.width(std_line_width(scale, 0.75));
  canvas.draw(mark_stroke, colors::black);

  boxvs.width(std_line_width(scale, 0.75));
  canvas.draw(boxvs, colors::black);

  if (!str_is_null(&m_x_axis.title))
    {
      double labx = m.sx * 0.5 + m.tx;
      double laby = canvas_mtx.ty;

      const char* text = m_x_axis.title.cstr();
      draw::text xlabel(text, label_text_size, 0.5, 0.0);
      xlabel.set_point(labx, laby);
      xlabel.apply_transform(identity_matrix, 1.0);

      canvas.draw(xlabel, colors::black);
    }

  if (!str_is_null(&m_y_axis.title))
    {
      double labx = canvas_mtx.tx;
      double laby = m.sy * 0.5 + m.ty;

      const char* text = m_y_axis.title.cstr();
      draw::text ylabel(text, label_text_size, 0.5, 1.0);
      ylabel.set_point(labx, laby);
      ylabel.angle(M_PI/2.0);
      ylabel.apply_transform(identity_matrix, 1.0);

      canvas.draw(ylabel, colors::black);
    }

  if (clip)
    canvas.reset_clipping();
}

template<class RM>
void plot<RM>::set_axis_labels_angle(axis_e axis_dir, double angle)
{
  get_axis(axis_dir).set_labels_angle(angle);
  m_need_redraw = true;
  compute_user_trans();
}

template<class RM>
void plot<RM>::set_units(bool use_units)
{
  if (m_use_units != use_units)
    {
      m_use_units = use_units;
      m_need_redraw = true;
      compute_user_trans();
    }
}

template<class RM>
void plot<RM>::update_units()
{
  if (m_rect.is_defined())
  {
    const rect_base<double>& r = m_rect.rect();
    m_ux = units(r.x1, r.x2);
    m_uy = units(r.y1, r.y2);
  }
  else
  {
    m_ux = units();
    m_uy = units();
  }

  compute_user_trans();
}

template<class RM>
void plot<RM>::set_limits(const agg::rect_base<double>& r)
{
  m_rect.set(r);
  update_units();
  m_need_redraw = true;
}

template<class RM>
void plot<RM>::unset_limits()
{
  m_rect.clear();
  update_units();
  m_need_redraw = true;
}

template<class RM>
void plot<RM>::layer_dispose_elements(plot<RM>::item_list* layer)
{
  unsigned n = layer->size();
  for (unsigned k = 0; k < n; k++)
    {
      RM::dispose(layer->at(k).vs);
    }
}

template<class RM>
bool plot<RM>::push_layer()
{
  if (m_layers.size() >= max_layers)
    return false;

  item_list *new_layer = new(std::nothrow) item_list();
  if (new_layer)
  {
    before_draw();
    push_drawing_queue();
    m_layers.add(new_layer);
    return true;
  }

  return false;
}

template<class RM>
bool plot<RM>::pop_layer()
{
  if (m_layers.size() <= 1)
    return false;

  unsigned n = m_layers.size();
  item_list* layer = m_layers[n-1];
  m_layers.inc_size(-1);
  layer_dispose_elements(layer);
  delete layer;

  clear_drawing_queue();
  m_need_redraw = true;

  return true;
}

template <class RM>
void plot<RM>::clear_current_layer()
{
  item_list* current = current_layer();
  clear_drawing_queue();
  layer_dispose_elements(current);
  current->clear();
}

template <class RM>
int plot<RM>::current_layer_index()
{
  return m_layers.size();
}

#endif
