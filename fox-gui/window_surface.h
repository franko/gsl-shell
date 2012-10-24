#ifndef FOXGUI_WINDOW_SURFACE_H
#define FOXGUI_WINDOW_SURFACE_H

#include <fx.h>

#include <agg_basics.h>
#include <agg_rendering_buffer.h>
#include <agg_trans_affine.h>

#include "image_buf.h"
#include "window_part.h"
#include "pthreadpp.h"
#include "sg_object.h"
#include "lua-plot-cpp.h"
#include "canvas.h"
#include "rect.h"

struct plot_ref {
    plot_ref(): plot(NULL) {}

    void attach(sg_plot* p);

    sg_plot* plot;
    plot_render_info inf;
    bool is_dirty;
    bool is_image_dirty;
    opt_rect<double> dirty_rect;
};

class window_surface
{
    enum { image_pixel_width = 3 };

public:
    typedef image_gen<image_pixel_width, true> image;

    window_surface(const char* split, pthread::mutex& sg_mut);
    ~window_surface();

    int attach(sg_plot* p, const char* slot_str);
    void split(const char* split_str);

    virtual void update_region(const agg::rect_base<int>& r) = 0;
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;

    void plot_draw(unsigned index);
    void plot_draw_queue(unsigned index, bool draw_all);
    void plot_render(unsigned index);

    sg_plot* get_plot(unsigned index, int canvas_width, int canvas_height, agg::rect_i& area);
    unsigned get_plot_number() const { return m_plots.size(); }

    bool need_redraw(unsigned index) const
    {
        return m_plots[index].plot->need_redraw();
    }

    bool is_ready() const { return (m_canvas != 0); }

    bool save_plot_image(unsigned index);
    bool restore_plot_image(unsigned index);

private:
    bool prepare_image_buffer(unsigned ww, unsigned hh);
    bool ensure_canvas_size(unsigned ww, unsigned hh);
    void plots_set_to_dirty();

    void plot_render(plot_ref& ref, const agg::rect_i& r);
    void plot_draw(unsigned index, int canvas_width, int canvas_height);
    opt_rect<double> plot_render_queue(plot_ref& ref, const agg::rect_i& r);
    void plot_draw_queue(unsigned index, int canvas_width, int canvas_height, bool draw_all);

    bool plot_is_defined(unsigned index) const
    {
        return (m_plots[index].plot != NULL);
    }

    image m_img;
    image m_save_img;
    window_part m_part;
    agg::pod_bvector<plot_ref> m_plots;
    canvas* m_canvas;
    pthread::mutex& m_graph_mutex;
};

#endif
