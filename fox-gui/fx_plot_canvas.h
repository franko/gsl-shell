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

public:
    fx_plot_canvas(FXComposite* p, const char* split, FXObject* tgt=NULL, FXSelector sel=0,
                   FXuint opts=FRAME_NORMAL,
                   FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    int attach(sg_plot* p, const char* slot_str);

    void plot_draw(unsigned index);
    void plot_draw_queue(unsigned index, bool draw_all);

    void update_region(const image& src_img, const agg::rect_i& r);

    sg_plot* get_plot(unsigned index, int canvas_width, int canvas_height, agg::rect_i& area);
    unsigned get_plot_number() const { return m_surface.plot_number(); }

    void draw(unsigned index);

    bool save_plot_image(unsigned index) { return m_surface.save_plot_image(index); }
    bool restore_plot_image(unsigned index) { return m_surface.restore_plot_image(index); }

    long on_cmd_paint(FXObject *, FXSelector, void *);
    long on_update(FXObject *, FXSelector, void *);

protected:
    fx_plot_canvas(): m_surface(NULL) {}

private:
    window_surface m_surface;
};

#endif
