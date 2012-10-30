#include "util/agg_color_conv_rgb8.h"

#include "fx_plot_canvas.h"
#include "rendering_buffer_utils.h"
#include "fatal.h"
#include "lua-graph.h"

FXDEFMAP(fx_plot_canvas) fx_plot_canvas_map[]=
{
    FXMAPFUNC(SEL_PAINT,     0, fx_plot_canvas::on_cmd_paint),
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

void fx_plot_canvas::update_region(const agg::rect_i& r)
{
    FXshort ww = r.x2 - r.x1, hh= r.y2 - r.y1;
    if (ww <= 0 || hh <= 0) return;

    const image& src_img = m_surface.get_image();

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

void fx_plot_canvas::update_plot_region(unsigned index)
{
    agg::rect_i area = get_plot_area(index);
    update_region(area);
}

int fx_plot_canvas::attach(sg_plot* p, const char* slot_str)
{
    return m_surface.attach(p, slot_str);
}

void fx_plot_canvas::plot_render(unsigned index)
{
    m_surface.render(index);
}

void fx_plot_canvas::plot_draw(unsigned index)
{
    m_surface.render(index);
    agg::rect_i r = get_plot_area(index);
    update_region(r);
    plot_set_dirty(index, false);
}

opt_rect<int> fx_plot_canvas::plot_render_queue(unsigned index)
{
    return m_surface.render_drawing_queue(index);
}

long fx_plot_canvas::on_cmd_paint(FXObject *, FXSelector, void *ptr)
{
    fprintf(stderr, "fx_plot_canvas::on_cmd_paint\n");
    int ww = getWidth(), hh = getHeight();

    if (!m_surface.canvas_size_match(ww, hh))
    {
        m_surface.resize(ww, hh);
        m_surface.draw_image_buffer();
    }

    agg::rect_i r(0, 0, ww, hh);
    update_region(r);
    return 1;
}
