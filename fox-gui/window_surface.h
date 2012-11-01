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

struct display_window {
    virtual void update_region(const agg::rect_i& r) = 0;
};

struct plot_ref {
    plot_ref(): plot(NULL), have_save_img(false) {}

    sg_plot* plot;
    plot_render_info inf;
    bool have_save_img;
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

    window_surface(display_window* window, const char* split);
    ~window_surface();

    int attach(sg_plot* p, const char* slot_str);
    void split(const char* split_str);

    bool canvas_size_match(unsigned ww, unsigned hh)
    {
        return (m_img.width() == ww && m_img.height() == hh);
    }

    bool resize(unsigned ww, unsigned hh);

    // redraw all the image buffer for the current plots
    void draw_image_buffer();

    int get_width()  const { return m_img.width(); }
    int get_height() const { return m_img.height(); }

    void render(unsigned index);
    opt_rect<int> render_drawing_queue(unsigned index);

    void draw_all();

    sg_plot* plot(unsigned index) const { return m_plots[index].plot; }

    agg::rect_i get_plot_area(unsigned index) const;
    agg::rect_i get_plot_area(unsigned index, int width, int height) const;
    unsigned plot_number() const { return m_plots.size(); }

    bool is_ready() const { return (m_canvas != 0); }

    bool save_plot_image(unsigned index);
    bool restore_plot_image(unsigned index);

    const image& get_image() { return m_img; }

    void slot_refresh(unsigned index);
    void slot_update(unsigned index);
    void save_slot_image(unsigned index);
    void restore_slot_image(unsigned index);

private:
    void render(plot_ref& ref, const agg::rect_i& r);

    opt_rect<int> render_drawing_queue(plot_ref& ref, const agg::rect_i& r);

    bool plot_is_defined(unsigned index) const
    {
        return (m_plots[index].plot != NULL);
    }

    image m_img;
    image m_save_img;
    window_part m_part;
    agg::pod_bvector<plot_ref> m_plots;
    display_window* m_window;
    canvas* m_canvas;
};

#endif
