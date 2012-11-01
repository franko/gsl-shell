#ifndef FOXGUI_FX_PLOT_CANVAS_H
#define FOXGUI_FX_PLOT_CANVAS_H

#include <fx.h>

#include <agg_basics.h>
#include <agg_rendering_buffer.h>
#include <agg_trans_affine.h>

#include "window_surface.h"

class fx_plot_canvas : public FXCanvas
{
    FXDECLARE(fx_plot_canvas)

    typedef window_surface::image image;

    class fox_display_window : public display_window {
    public:
        fox_display_window(fx_plot_canvas* can): m_fox_canvas(can) {}
        virtual void update_region(const agg::rect_i& r) { m_fox_canvas->update_region(r); }
    private:
        fx_plot_canvas* m_fox_canvas;
    };

public:
    fx_plot_canvas(FXComposite* p, const char* split, FXObject* tgt=NULL, FXSelector sel=0,
                   FXuint opts=FRAME_NORMAL,
                   FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    void update_region(const agg::rect_i& r);

    window_surface& surface() { return m_surface; }

    long on_cmd_paint(FXObject *, FXSelector, void *);

protected:
    fx_plot_canvas(): m_window(this), m_surface(&m_window, NULL) {}

private:
    fox_display_window m_window;
    window_surface m_surface;
};

#endif
