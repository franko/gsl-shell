#ifndef FOXGUI_FX_PLOT_CANVAS_H
#define FOXGUI_FX_PLOT_CANVAS_H

#include <fx.h>

#include <agg_basics.h>
#include <agg_rendering_buffer.h>
#include <agg_trans_affine.h>

class window_surface;

class fx_plot_canvas : public FXCanvas
{
    FXDECLARE(fx_plot_canvas)
public:
    fx_plot_canvas(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0, FXuint opts=FRAME_NORMAL,
                   FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    void update_region(const agg::rect_i& r);

    void attach_surface(window_surface* surf) { m_surface = surf; }

    long on_cmd_paint(FXObject *, FXSelector, void *);

protected:
    fx_plot_canvas() {}

private:
    window_surface* m_surface;
};

#endif
