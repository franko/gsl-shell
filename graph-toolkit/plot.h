
/* plot.h
 *
 * Copyright (C) 2009-2013 Francesco Abbate
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
#include "factor_labels.h"

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

    virtual void draw(sg_object& vs, agg::rgba8 c) {
        m_canvas->draw(vs, c);
    }
    virtual void draw_outline(sg_object& vs, agg::rgba8 c) {
        m_canvas->draw_outline(vs, c);
    }

    virtual void clip_box(const agg::rect_base<int>& clip) {
        m_canvas->clip_box(clip);
    }
    virtual void reset_clipping() {
        m_canvas->reset_clipping();
    }

private:
    Canvas* m_canvas;
};

struct plot_layout {
    struct point {
        point(double _x, double _y): x(_x), y(_y) {}
        point() {}
        double x, y;
    };

    void set_plot_active_area(double sx, double sy, double tx, double ty)
    {
        plot_active_area.sx = sx;
        plot_active_area.sy = sy;
        plot_active_area.tx = tx;
        plot_active_area.ty = ty;
    }

    static void set_area_undefined(agg::trans_affine& m) {
        m.sx = -1.0;
    }

    static bool is_area_defined(const agg::trans_affine& m)
    {
        return (m.sx > 0.0);
    }

    point title_pos;
    double title_font_size;

    agg::trans_affine legend_area[4];
    agg::trans_affine plot_area;
    agg::trans_affine plot_active_area;
};

struct plot_render_info {
    agg::trans_affine active_area;
};

struct plot_item {
    sg_object* vs;
    agg::rgba8 color;
    bool outline;

    plot_item() : vs(0) {};

    plot_item(sg_object* vs, agg::rgba8& c, bool as_outline):
        vs(vs), color(c), outline(as_outline)
    {};

    sg_object& content() {
        return *vs;
    };
};

typedef manage_owner RM;

class plot {

    static const unsigned max_layers = 8;

    enum {
        axis_label_prop_space = 20,
        axis_title_prop_space = 30,
        canvas_margin_prop_space = 15,
        canvas_margin_fixed_space = 4,
    };

protected:
    enum text_class_e { text_axis_title, text_axis_labels, text_plot_title };

    double get_default_font_size(text_class_e tc, double scale)
    {
        double base_size;
        if (tc == text_axis_title)
            base_size = 15.0;
        else if (tc == text_axis_labels)
            base_size = 14.0;
        else /* text_axis_title */
            base_size = 18.0;

        const double cscale = max(scale, 0.75);
        return base_size * cscale;
    }

    typedef plot_item item;

    class item_list : public agg::pod_bvector<item>
    {
    public:
        item_list(): agg::pod_bvector<item>() { }

        const opt_rect<double>& bounding_box() const {
            return m_bbox;
        }
        void set_bounding_box(const agg::rect_base<double>& r) {
            m_bbox.set(r);
        }
        void clear_bounding_box() {
            m_bbox.clear();
        }

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

        void clear_label_format() {
            format_tag = units::format_invalid;
        }

        void set_labels_angle(double angle)
        {
            double a = (dir == x_axis ? -angle + M_PI/2 : -angle);
            double c = cos(a), s = sin(a);
            m_labels_hjustif = round(c + 1.0) / 2.0;
            m_labels_vjustif = round(s + 1.0) / 2.0;
            m_labels_angle = angle;
        }

        double labels_angle()   const {
            return m_labels_angle;
        }
        double labels_hjustif() const {
            return m_labels_hjustif;
        }
        double labels_vjustif() const {
            return m_labels_vjustif;
        }

    private:
        double m_labels_angle;
        double m_labels_hjustif, m_labels_vjustif;
        char m_label_format[units::label_format_max_size];
    };

    plot(bool use_units = true) :
        m_drawing_queue(0), m_clip_flag(true),
        m_need_redraw(true), m_rect(),
        m_use_units(use_units), m_pad_units(false), m_title(),
        m_sync_mode(true), m_x_axis(x_axis), m_y_axis(y_axis),
        m_xaxis_hol(0)
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

        delete m_xaxis_hol;
    };

    str& title() {
        return m_title;
    }
    str& x_axis_title() {
        return m_x_axis.title;
    }
    str& y_axis_title() {
        return m_y_axis.title;
    }

    void add_legend(plot* p, placement_e where) {
        m_legend[where] = p;
    }
    plot* get_legend(placement_e where) {
        return m_legend[where];
    }

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
    bool use_units() const {
        return m_use_units;
    };

    void update_units();
    void set_limits(const agg::rect_base<double>& r);
    void unset_limits();

    ptr_list<factor_labels>* get_xaxis_hol() { return m_xaxis_hol; }

    void set_xaxis_hol(ptr_list<factor_labels>* hol)
    {
        delete m_xaxis_hol;
        m_xaxis_hol = hol;
    }

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
    void draw(Canvas& canvas, const agg::trans_affine& m, plot_render_info* inf)
    {
        canvas_adapter<Canvas> vc(&canvas);
        agg::rect_i clip = rect_of_slot_matrix<int>(m);
        plot_layout layout = compute_plot_layout(m);
        draw_virtual_canvas(vc, layout, &clip);
        if (inf)
            inf->active_area = layout.plot_active_area;
    }

    template <class Canvas>
    void draw(Canvas& canvas, const agg::rect_i& r, plot_render_info* inf)
    {
        canvas_adapter<Canvas> vc(&canvas);
        agg::trans_affine mtx = affine_matrix(r);
        plot_layout layout = compute_plot_layout(mtx);
        draw_virtual_canvas(vc, layout, &r);
        if (inf)
            inf->active_area = layout.plot_active_area;
    }

    virtual bool push_layer();
    virtual bool pop_layer();
    virtual void clear_current_layer();

    /* drawing queue related methods */
    void push_drawing_queue();
    void clear_drawing_queue();
    int current_layer_index();

    bool clip_is_active() const {
        return m_clip_flag;
    };
    void set_clip_mode(bool flag) {
        m_clip_flag = flag;
    };

    bool need_redraw() const {
        return m_need_redraw;
    };
    void commit_pending_draw();

    template <class Canvas>
    void draw_queue(Canvas& canvas, const agg::trans_affine& m, const plot_render_info& inf, opt_rect<double>& bbox);

    void sync_mode(bool req_mode) {
        m_sync_mode = req_mode;
    };
    bool sync_mode() const {
        return m_sync_mode;
    };

    void pad_mode(bool req)
    {
        if (req != m_pad_units)
        {
            m_pad_units = req;
            m_need_redraw = true;
            compute_user_trans();
        }
    };

    bool pad_mode() const {
        return m_pad_units;
    }

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

    void enable_categories(axis_e dir) {
        get_axis(dir).use_categories = true;
    }

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
    void draw_virtual_canvas(canvas_type& canvas, plot_layout& layout, const agg::rect_i* r);
    void draw_simple(canvas_type& canvas, plot_layout& layout, const agg::rect_i* r);

    void draw_grid(const axis_e dir, const units& u,
                   const agg::trans_affine& user_mtx,
                   agg::path_storage& ln);

    double draw_axis_m(axis_e dir, units& u, const agg::trans_affine& user_mtx,
                       ptr_list<draw::text>& labels, double scale,
                       agg::path_storage& mark, agg::path_storage& ln);

    double draw_xaxis_factors(units& u, const agg::trans_affine& user_mtx,
                             ptr_list<draw::text>& labels,
                             ptr_list<factor_labels>* f_labels, double scale,
                             agg::path_storage& mark, agg::path_storage& ln);

    void draw_elements(canvas_type &canvas, const plot_layout& layout);
    void draw_element(item& c, canvas_type &canvas, const agg::trans_affine& m);
    void draw_axis(canvas_type& can, plot_layout& layout, const agg::rect_i* clip = 0);

    void draw_legends(canvas_type& canvas, const plot_layout& layout);

    plot_layout compute_plot_layout(const agg::trans_affine& canvas_mtx, bool do_legends = true);

    // return the matrix that map from plot coordinates to screen
    // coordinates
    agg::trans_affine get_model_matrix(const plot_layout& layout);

    void clip_plot_area(canvas_type& canvas, const agg::trans_affine& canvas_mtx);

    void compute_user_trans();

    bool fit_inside(sg_object *obj) const;

    void layer_dispose_elements (item_list* layer);

    unsigned nb_layers() const {
        return m_layers.size();
    }
    item_list* get_layer(unsigned j) {
        return m_layers[j];
    }

    item_list* current_layer() {
        return m_layers[m_layers.size() - 1];
    };

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

    // keep trace of the region where changes happened since
    // the last pushlayer or clear
    opt_rect<double> m_changes_accu;
    opt_rect<double> m_changes_pending;

    bool m_use_units;
    units m_ux, m_uy;

private:
    item_list m_root_layer;
    agg::pod_auto_vector<item_list*, max_layers> m_layers;

    bool m_pad_units;

    str m_title;

    bool m_sync_mode;

    axis m_x_axis, m_y_axis;
    plot* m_legend[4];

    ptr_list<factor_labels>* m_xaxis_hol;
};

template <class Canvas>
void plot::draw_queue(Canvas& _canvas, const agg::trans_affine& canvas_mtx, const plot_render_info& inf, opt_rect<double>& bb)
{
    canvas_adapter<Canvas> canvas(&_canvas);
    before_draw();

    plot_layout layout = compute_plot_layout(canvas_mtx);
    layout.plot_active_area = inf.active_area;

    this->clip_plot_area(canvas, layout.plot_active_area);

    typedef typename plot::iterator iter_type;
    iter_type *c0 = m_drawing_queue;
    for (iter_type *c = c0; c != 0; c = c->next())
    {
        item& d = c->content();
        agg::trans_affine m = get_model_matrix(layout);
        draw_element(d, canvas, m);

        agg::rect_base<double> ebb;
        bool not_empty = agg::bounding_rect_single(d.content(), 0, &ebb.x1, &ebb.y1, &ebb.x2, &ebb.y2);

        if (not_empty)
            bb.add<rect_union>(ebb);
    }

    m_changes_accu.add<rect_union>(bb);

    if (m_changes_pending.is_defined())
    {
        bb.add<rect_union>(m_changes_pending);
    }

    canvas.reset_clipping();
}

#endif
