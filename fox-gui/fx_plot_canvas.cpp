#include "util/agg_color_conv_rgb8.h"

#include "fx_plot_canvas.h"
#include "rendering_buffer_utils.h"
#include "fatal.h"
#include "lua-graph.h"

FXDEFMAP(fx_plot_canvas) fx_plot_canvas_map[]=
{
    FXMAPFUNC(SEL_PAINT,     0, fx_plot_canvas::on_cmd_paint),
    FXMAPFUNC(SEL_UPDATE,    0, fx_plot_canvas::on_update),
};

FXIMPLEMENT(fx_plot_canvas,FXCanvas,fx_plot_canvas_map,ARRAYNUMBER(fx_plot_canvas_map));

fx_plot_canvas::fx_plot_canvas(FXComposite* p, const char* split_str, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h):
    FXCanvas(p, tgt, sel, opts, x, y, w, h), m_surface(split_str)
{
    unsigned n = m_surface.plot_number();
    m_dirty_flags.resize(n);
    for (unsigned k = 0; k < n; k++)
        m_dirty_flags[k] = true;
}

void fx_plot_canvas::update_region(const image& src_img, const agg::rect_i& r)
{
    FXshort ww = r.x2 - r.x1, hh= r.y2 - r.y1;
    if (ww <= 0 || hh <= 0) return;

    FXImage img(getApp(), NULL, IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, ww, hh);

    const unsigned fox_pixel_size = 4;

    agg::rendering_buffer dest;
    dest.attach((agg::int8u*) img.getData(), ww, hh, -ww * fox_pixel_size);

    rendering_buffer_ro src;
    rendering_buffer_get_const_view(src, src_img, r, window_surface::image_pixel_width);

    my_color_conv(&dest, &src, color_conv_rgb24_to_rgba32());

    img.create();

    FXDCWindow dc(this);
    dc.drawImage(&img, r.x1, getHeight() - r.y2);
}

int fx_plot_canvas::attach(sg_plot* p, const char* slot_str)
{
    return m_surface.attach(p, slot_str);
}

sg_plot*
fx_plot_canvas::get_plot(unsigned index, int canvas_width, int canvas_height, agg::rect_i& area)
{
    area = m_surface.get_plot_area(index, canvas_width, canvas_height);
    return m_surface.plot(index);
}

void fx_plot_canvas::plot_draw(unsigned index)
{
    agg::rect_i r = m_surface.plot_draw(index, true);
    update_region(m_surface.get_image(), r);
    plot_set_dirty(index, false);
}

void fx_plot_canvas::plot_draw_queue(unsigned index, bool draw_all)
{
    agg::rect_i r = m_surface.plot_draw_queue(index, draw_all);
    update_region(m_surface.get_image(), r);
}

long fx_plot_canvas::on_cmd_paint(FXObject *, FXSelector, void *ptr)
{
    fprintf(stderr, "fx_plot_canvas::on_cmd_paint\n");
    int ww = getWidth(), hh = getHeight();
    if (unlikely(!m_surface.ensure_canvas_size(ww, hh)))
        return 1;
    for (unsigned k = 0; k < m_surface.plot_number(); k++)
        plot_draw(k);
    return 1;
}

long fx_plot_canvas::on_update(FXObject *, FXSelector, void *)
{
    // fprintf(stderr, "fx_plot_canvas::on_update\n");
    int ww = getWidth(), hh = getHeight();
    if (unlikely(!m_surface.ensure_canvas_size(ww, hh)))
        return 1;
    for (unsigned k = 0; k < m_surface.plot_number(); k++)
    {
        if (plot_is_dirty(k))
            plot_draw(k);
    }
    return 1;
}
