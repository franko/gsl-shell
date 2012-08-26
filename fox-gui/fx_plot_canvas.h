#ifndef FOXGUI_FX_PLOT_CANVAS_H
#define FOXGUI_FX_PLOT_CANVAS_H

#include <new>
#include <fx.h>
#include <agg_rendering_buffer.h>

#include "image_buf.h"
#include "sg_object.h"
#include "lua-plot-cpp.h"
#include "canvas.h"
#include "rect.h"

class fx_plot_canvas : public FXCanvas
{
    FXDECLARE(fx_plot_canvas)

    typedef image_gen<3, true> image;

public:
    fx_plot_canvas(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0,
                   FXuint opts=FRAME_NORMAL,
                   FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    ~fx_plot_canvas();

    void attach(sg_plot* p);
    void update_region(const agg::rect_base<int>& r);

    sg_plot* get_plot()
    {
        return m_plot;
    }

    void plot_render(const agg::trans_affine& m);
    void plot_draw(const agg::trans_affine& m);
    opt_rect<double> plot_render_queue(const agg::trans_affine& m);
    void plot_draw_queue(const agg::trans_affine& m, bool draw_all);

    const agg::trans_affine& plot_matrix() const
    {
        return m_area_mtx;
    }

    bool is_ready() const
    {
        return m_canvas && m_plot;
    }

    bool save_image();
    bool restore_image();

    long on_cmd_paint(FXObject *, FXSelector, void *);
    long on_update(FXObject *, FXSelector, void *);

protected:
    fx_plot_canvas() {}

private:
    void prepare_image_buffer(unsigned ww, unsigned hh);
    void ensure_canvas_size(unsigned ww, unsigned hh);

    image m_img;
    image m_save_img;
    sg_plot* m_plot;
    canvas* m_canvas;
    bool m_dirty_flag, m_dirty_img;
    opt_rect<double> m_dirty_rect;
    agg::trans_affine m_area_mtx;
};

#endif
