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

    bool is_ready() const { return m_surface.is_ready(); }

    bool plot_need_redraw(unsigned index) const
    {
        return m_surface.plot(index)->need_redraw();
    }

    void plot_render(unsigned index);
    opt_rect<int> plot_render_queue(unsigned index);

    void plot_draw(unsigned index);

    void update_region(const agg::rect_i& r);
    void update_plot_region(unsigned index);

    sg_plot* get_plot(unsigned index) const { return m_surface.plot(index); }
    unsigned get_plot_number() const { return m_surface.plot_number(); }
    agg::rect_i get_plot_area(unsigned index) const { return m_surface.get_plot_area(index); }
    agg::rect_i get_plot_area(unsigned index, int w, int h) const { return m_surface.get_plot_area(index, w, h); }

    void draw(unsigned index);

    bool plot_have_saved_image(unsigned index) const { return m_surface.have_saved_image(index); }

    bool save_plot_image(unsigned index) { return m_surface.save_plot_image(index); }
    void restore_plot_image(unsigned index) { m_surface.restore_plot_image(index); }

    long on_cmd_paint(FXObject *, FXSelector, void *);

protected:
    fx_plot_canvas(): m_surface(NULL) {}

private:
    void plot_set_dirty(unsigned k, bool flag) { m_dirty_flags[k] = flag; }
    bool plot_is_dirty(unsigned k) const { return m_dirty_flags[k]; }

    agg::pod_array<bool> m_dirty_flags;
    window_surface m_surface;
};

#endif
