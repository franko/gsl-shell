#ifndef FOXGUI_FX_PLOT_CANVAS_H
#define FOXGUI_FX_PLOT_CANVAS_H

#include <new>
#include <fx.h>
#include <agg_rendering_buffer.h>

#include "image_buf.h"
#include "window_part.h"
#include "sg_object.h"
#include "lua-plot-cpp.h"
#include "canvas.h"
#include "rect.h"

#if 0
template <class T>
class my_array {
public:
    my_array(): m_data(0) {}
    ~my_array() { delete[] m_data; }

    void resize(unsigned n)
    {
        delete[] m_data;
        m_data = new T[n];
    }

    const T& operator[](unsigned k) const { return m_data[k]; }
          T& operator[](unsigned k)       { return m_data[k]; }

private:
    T* m_data;
};
#endif

struct plot_ref {
    plot_area(): plot(NULL) {}

    sg_plot* plot;
    bool is_dirty;
    bool is_image_dirty;
    opt_rect<double> m_dirty_rect;
};

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

    agg::trans_affine plot_matrix(unsigned slot_id) const
    {
        const agg::rect_i& r = m_part.rect(k);
        double dx = r.x2 - r.x1, dy = r.y2 - r.y1;
        double tx = r.x1, ty = r.y1;
        return agg::trans_affine(dx, 0.0, 0.0, dy, tx, ty);
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
