#ifndef FOXGUI_FX_PLOT_CANVAS_H
#define FOXGUI_FX_PLOT_CANVAS_H

#include <fx.h>

#include <agg_basics.h>
#include <agg_rendering_buffer.h>
#include <agg_trans_affine.h>

#include "window_surface.h"

class fox_window_surface : public window_surface
{
public:
    fox_window_surface(fx_plot_canvas& fxcan, const char* split):
    window_surface(split), m_fx_canvas(fxcan)
    { }

    virtual void update_region(const agg::rect_base<int>& r);
    virtual get_width()  const { return m_fx_canvas.getWidth(); }
    virtual get_height() const { return m_fx_canvas.getHeight(); }
};

class fx_plot_canvas : public FXCanvas
{
    FXDECLARE(fx_plot_canvas)

public:
    fx_plot_canvas(FXComposite* p, const char* split, FXObject* tgt=NULL, FXSelector sel=0,
                   FXuint opts=FRAME_NORMAL,
                   FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    void plot_draw(unsigned index);
    void plot_draw_queue(unsigned index, bool draw_all);
    void plot_render(unsigned index);

    sg_plot* get_plot(unsigned index, int canvas_width, int canvas_height, agg::rect_i& area);
    unsigned get_plot_number() const { return m_plots.size(); }

#if 0
    bool need_redraw(unsigned index) const
    {
        return m_plots[index].plot->need_redraw();
    }

    bool is_ready() const { return (m_canvas != 0); }
#endif

    bool save_plot_image(unsigned index);
    bool restore_plot_image(unsigned index);

    long on_cmd_paint(FXObject *, FXSelector, void *);
    long on_update(FXObject *, FXSelector, void *);

protected:
    fx_plot_canvas() {}

private:
    fox_window_surface m_surface;
};

#endif
