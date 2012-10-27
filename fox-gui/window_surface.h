#ifndef FOXGUI_WINDOW_SURFACE_H
#define FOXGUI_WINDOW_SURFACE_H

#include <fx.h>

#include <agg_basics.h>
#include <agg_rendering_buffer.h>
#include <agg_trans_affine.h>

#include "lua-graph.h"
#include "image_buf.h"
#include "window_part.h"
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

struct graph_mutex {
    static void lock()   { AGG_LOCK();   }
    static void unlock() { AGG_UNLOCK(); }
};

class window_surface
{
public:
    enum { image_pixel_width = 3 };
    typedef image_gen<image_pixel_width, true> image;

    window_surface(const char* split);
    ~window_surface();

    int attach(sg_plot* p, const char* slot_str);
    void split(const char* split_str);

    bool ensure_canvas_size(unsigned ww, unsigned hh);

    int get_width()  const { return m_img.width(); }
    int get_height() const { return m_img.height(); }

    // draw plot identified by index in its slot and
    // returns the rectangle area that needs to be updated on screen
    agg::rect_i plot_draw(unsigned index);

    // draw plot's drawing queue and returns the rectangle area
    // that needs to be updated on screen
    agg::rect_i plot_draw_queue(unsigned index, bool draw_all);

    sg_plot* plot(unsigned index) const { return m_plots[index].plot; }

    agg::rect_i get_plot_area(unsigned index, int canvas_width, int canvas_height);
    unsigned plot_number() const { return m_plots.size(); }

    bool is_ready() const { return (m_canvas != 0); }

    bool save_plot_image(unsigned index);
    bool restore_plot_image(unsigned index);

    const image& get_image() { return m_img; }
    bool plot_is_dirty(unsigned k) const { return m_plots[k].is_dirty; }

private:
    bool resize(unsigned ww, unsigned hh);
    void plots_set_to_dirty();

    // render the plot in the given rectangle area of image
    void render(plot_ref& ref, const agg::rect_i& r);

    agg::rect_i plot_draw(unsigned index, int canvas_width, int canvas_height);
    opt_rect<double> plot_render_queue(plot_ref& ref, const agg::rect_i& r);

    bool plot_is_defined(unsigned index) const
    {
        return (m_plots[index].plot != NULL);
    }

    image m_img;
    image m_save_img;
    window_part m_part;
    agg::pod_bvector<plot_ref> m_plots;
    canvas* m_canvas;
};

#endif
