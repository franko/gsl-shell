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

    sg_plot* plot;
    bool is_dirty;
    bool is_image_dirty;
    opt_rect<double> m_dirty_rect;
};

class fx_plot_canvas : public FXCanvas
{
    FXDECLARE(fx_plot_canvas)

    enum { image_pixel_width = 3 };

    typedef image_gen<image_pixel_width, true> image;

public:
    fx_plot_canvas(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0,
                   FXuint opts=FRAME_NORMAL,
                   FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    ~fx_plot_canvas();

    void attach(sg_plot* p);
    void update_region(const agg::rect_base<int>& r);

    // sg_plot* get_plot(unsigned index)
    // {
    //     return m_plot;
    // }

    void plot_render(plot_ref& ref, const agg::trans_affine& m);
    void plot_draw(unsigned index, int canvas_width, int canvas_height);
    opt_rect<double> plot_render_queue(const agg::trans_affine& m);
    void plot_draw_queue(const agg::trans_affine& m, bool draw_all);

#if 0
    agg::trans_affine plot_matrix(unsigned index) const
    {
        const agg::rect_i& r = m_part.rect(index);
        double dx = r.x2 - r.x1, dy = r.y2 - r.y1;
        double tx = r.x1, ty = r.y1;
        return agg::trans_affine(dx, 0.0, 0.0, dy, tx, ty);
    }
#endif

#if 0    
    bool is_ready() const
    {
        return m_canvas && m_plot;
    }
#endif

    bool save_image();
    bool restore_image();

    long on_cmd_paint(FXObject *, FXSelector, void *);
    long on_update(FXObject *, FXSelector, void *);

protected:
    fx_plot_canvas() {}

private:
    void prepare_image_buffer(unsigned ww, unsigned hh);
    void ensure_canvas_size(unsigned ww, unsigned hh);
    void plots_set_to_dirty();

    bool plot_is_defined(unsigned index)
    {
        plot_ref& ref = m_plots[index];
        return (ref.plot != NULL);
    }

//    my_array<image> m_img;
//    my_array<image> m_save_img;
    image m_img;
    image m_save_img;
    window_part m_part;
    agg::pod_bvector<plot_ref> m_plots;
    canvas* m_canvas;
//    bool m_dirty_flag, m_dirty_img;
//    opt_rect<double> m_dirty_rect;
//    agg::trans_affine m_area_mtx;
};

#endif
