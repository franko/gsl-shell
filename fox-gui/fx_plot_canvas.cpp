#include "util/agg_color_conv_rgb8.h"

#include "fx_plot_canvas.h"
#include "rendering_buffer_utils.h"
#include "fatal.h"
#include "lua-graph.h"
#include "window_surface.h"

FXDEFMAP(fx_plot_canvas) fx_plot_canvas_map[]=
{
    FXMAPFUNC(SEL_PAINT,     0, fx_plot_canvas::on_cmd_paint),
};

FXIMPLEMENT(fx_plot_canvas,FXCanvas,fx_plot_canvas_map,ARRAYNUMBER(fx_plot_canvas_map));

fx_plot_canvas::fx_plot_canvas(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h):
    FXCanvas(p, tgt, sel, opts, x, y, w, h)
{
}

void fx_plot_canvas::update_region(const agg::rect_i& r)
{
    FXshort ww = r.x2 - r.x1, hh= r.y2 - r.y1;
    if (ww <= 0 || hh <= 0) return;

    const window_surface::image& src_img = m_surface->get_image();

    FXImage img(getApp(), NULL, IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, ww, hh);

    const unsigned fox_pixel_size = 4;

    agg::rendering_buffer dest;
    dest.attach((agg::int8u*) img.getData(), ww, hh, -ww * fox_pixel_size);

    rendering_buffer_ro src;
    rendering_buffer_get_const_view(src, src_img, r, window_surface::image_pixel_width);

    my_color_conv(&dest, &src, color_conv_rgb24_to_bgra32());

    img.create();

    FXDCWindow dc(this);
    dc.drawImage(&img, r.x1, getHeight() - r.y2);
}

long fx_plot_canvas::on_cmd_paint(FXObject *, FXSelector, void *ptr)
{
    int ww = getWidth(), hh = getHeight();

    if (!m_surface->canvas_size_match(ww, hh))
    {
        m_surface->resize(ww, hh);
        m_surface->draw_image_buffer();
    }

    agg::rect_i r(0, 0, ww, hh);
    update_region(r);
    return 1;
}
