#ifndef FOXGUI_FX_PLOT_CANVAS_H
#define FOXGUI_FX_PLOT_CANVAS_H

#include <new>
#include <fx.h>

#include <agg_basics.h>
#include <agg_rendering_buffer.h>
#include <agg_trans_affine.h>

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
    bool is_dirty;
    bool is_image_dirty;
    opt_rect<double> dirty_rect;
};

class fx_plot_canvas : public FXCanvas
{
    FXDECLARE(fx_plot_canvas)

    enum { image_pixel_width = 3 };

    typedef image_gen<image_pixel_width, true> image;

public:
    fx_plot_canvas(FXComposite* p, const char* split, FXObject* tgt=NULL, FXSelector sel=0,
                   FXuint opts=FRAME_NORMAL,
                   FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    ~fx_plot_canvas();

    int attach(sg_plot* p, const char* slot_str);
    void split(const char* split_str);

    void update_region(const agg::rect_base<int>& r);

    void plot_draw(unsigned index);
    void plot_draw_queue(unsigned index, bool draw_all);
    void plot_render(unsigned index);

    bool need_redraw(unsigned index)
    {
        return m_plots[index].plot->need_redraw();
    }

    bool is_ready() const { return (m_canvas != 0); }

    bool save_plot_image(unsigned index);
    bool restore_plot_image(unsigned index);

    long on_cmd_paint(FXObject *, FXSelector, void *);
    long on_update(FXObject *, FXSelector, void *);

protected:
    fx_plot_canvas() {}

private:
    void prepare_image_buffer(unsigned ww, unsigned hh);
    void ensure_canvas_size(unsigned ww, unsigned hh);
    void plots_set_to_dirty();

    void plot_render(plot_ref& ref, const agg::trans_affine& m);
    void plot_draw(unsigned index, int canvas_width, int canvas_height);
    opt_rect<double> plot_render_queue(plot_ref& ref, const agg::trans_affine& m);
    void plot_draw_queue(unsigned index, int canvas_width, int canvas_height, bool draw_all);

    bool plot_is_defined(unsigned index)
    {
        plot_ref& ref = m_plots[index];
        return (ref.plot != NULL);
    }

    image m_img;
    image m_save_img;
    window_part m_part;
    agg::pod_bvector<plot_ref> m_plots;
    canvas* m_canvas;
};

#endif
