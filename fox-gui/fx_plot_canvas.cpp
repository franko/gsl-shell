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
    agg::rect_i area = m_surface.get_plot_area(index);
    update_region(area);
}

int fx_plot_canvas::attach(sg_plot* p, const char* slot_str)
{
    return m_surface.attach(p, slot_str);
}

void fx_plot_canvas::slot_refresh(unsigned index)
{
    bool redraw = m_surface.plot(index)->need_redraw();
    if (redraw)
    {
        m_surface.render(index);
    }

    opt_rect<int> r = m_surface.render_drawing_queue(index);
    agg::rect_i area = m_surface.get_plot_area(index);
    if (redraw)
    {
        update_region(area);
    }
    else
    {
        if (r.is_defined())
        {
            const int pad = 4;
            const agg::rect_i& ri = r.rect();
            agg::rect_i r_pad(ri.x1 - pad, ri.y1 - pad, ri.x2 + pad, ri.y2 + pad);
            r_pad.clip(area);
            update_region(r_pad);
        }
    }
}
void
fx_plot_canvas::slot_update(unsigned index)
{
    m_surface.render(index);
    m_surface.render_drawing_queue(index);
    update_plot_region(index);
}

void
fx_plot_canvas::save_slot_image(unsigned index)
{
    m_surface.save_plot_image(index);
}

void
fx_plot_canvas::restore_slot_image(unsigned index)
{
    if (m_surface.have_saved_image(index))
    {
        m_surface.restore_plot_image(index);
    }
    else
    {
        m_surface.render(index);
        m_surface.save_plot_image(index);
    }
}

long fx_plot_canvas::on_cmd_paint(FXObject *, FXSelector, void *ptr)
{
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
