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
}

void fx_plot_canvas::plot_render(unsigned index)
{
    plot_ref& ref = m_plots[index];
    int ww = getWidth(), hh = getHeight();
    agg::rect_i r = m_part.rect(index, ww, hh);
    plot_render(ref, r);
}

void fx_plot_canvas::update_region(const agg::rect_i& r)
{
    FXshort ww = r.x2 - r.x1, hh= r.y2 - r.y1;
    FXImage img(getApp(), NULL, IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, ww, hh);

    const unsigned fox_pixel_size = 4;

    agg::rendering_buffer dest;
    dest.attach((agg::int8u*) img.getData(), ww, hh, -ww * fox_pixel_size);

    rendering_buffer_ro src;
    rendering_buffer_get_const_view(src, m_img, r, image_pixel_width);

    my_color_conv(&dest, &src, color_conv_rgb24_to_rgba32());

    img.create();

    FXDCWindow dc(this);
    dc.drawImage(&img, r.x1, getHeight() - r.y2);
}

void fx_plot_canvas::plot_draw(unsigned index, int canvas_width, int canvas_height)
{
    plot_ref& ref = m_plots[index];
    agg::rect_i r = m_part.rect(index, canvas_width, canvas_height);
    if (ref.is_image_dirty)
        plot_render(ref, r);
    update_region(r);
    ref.is_dirty = false;
}

sg_plot* fx_plot_canvas::get_plot(unsigned index, int canvas_width, int canvas_height, agg::rect_i& area)
{
    plot_ref& ref = m_plots[index];
    if (ref.plot)
        area = m_part.rect(index, canvas_width, canvas_height);
    return ref.plot;
}

void fx_plot_canvas::plot_draw(unsigned index)
{
    int ww = getWidth(), hh = getHeight();
    plot_draw(index, ww, hh);
}

void
fx_plot_canvas::plot_draw_queue(unsigned index, int canvas_width, int canvas_height, bool draw_all)
{
    plot_ref& ref = m_plots[index];

    if (!ref.plot) return;

    agg::rect_i r = m_part.rect(index, canvas_width, canvas_height);
    opt_rect<double> rect = plot_render_queue(ref, r);

    if (draw_all)
    {
        update_region(r);
    }
    else if (rect.is_defined())
    {
        const int pd = 4;
        const agg::rect_d& ur = rect.rect();
        const agg::rect_i box(0, 0, canvas_width, canvas_height);
        agg::rect_i ri(ur.x1 - pd, ur.y1 - pd, ur.x2 + pd, ur.y2 + pd);
        ri.clip(box);
        update_region(ri);
    }
}

void fx_plot_canvas::plot_draw_queue(unsigned index, bool draw_all)
{
    int ww = getWidth(), hh = getHeight();
    plot_draw_queue(index, ww, hh, draw_all);
}

void plot_ref::attach(sg_plot* p)
{
    plot = p;
    is_dirty = true;
    is_image_dirty = true;
    dirty_rect.clear();
}

int fx_plot_canvas::attach(sg_plot* p, const char* slot_str)
{
    int index = m_part.get_slot_index(slot_str);
    if (index >= 0)
        m_plots[index].attach(p);
    return index;
}

bool fx_plot_canvas::save_plot_image(unsigned index)
{
    int ww = getWidth(), hh = getHeight();

    if (!m_save_img.ensure_size(ww, hh)) return false;

    plot_ref& ref = m_plots[index];
    agg::rect_i r = m_part.rect(index, ww, hh);

    if (ref.is_image_dirty)
        plot_render(ref, r);

    rendering_buffer_ro src;
    rendering_buffer_get_const_view(src, m_img, r, image_pixel_width);

    agg::rendering_buffer dest;
    rendering_buffer_get_view(dest, m_save_img, r, image_pixel_width);

    dest.copy_from(src);
    return true;
}

bool fx_plot_canvas::restore_plot_image(unsigned index)
{
    int ww = getWidth(), hh = getHeight();

    if (!image::match(m_img, m_save_img))
        return false;

    agg::rect_i r = m_part.rect(index, ww, hh);

    rendering_buffer_ro src;
    rendering_buffer_get_const_view(src, m_save_img, r, image_pixel_width);

    agg::rendering_buffer dest;
    rendering_buffer_get_view(dest, m_img, r, image_pixel_width);

    dest.copy_from(src);
    return true;
}

long fx_plot_canvas::on_cmd_paint(FXObject *, FXSelector, void *ptr)
{
    int ww = getWidth(), hh = getHeight();
    if (unlikely(!ensure_canvas_size(ww, hh)))
        return 1;
    for (unsigned k = 0; k < m_plots.size(); k++)
    {
        plot_draw(k, ww, hh);
    }
    return 1;
}

long fx_plot_canvas::on_update(FXObject *, FXSelector, void *)
{
    int ww = getWidth(), hh = getHeight();
    if (unlikely(!ensure_canvas_size(ww, hh)))
        return 1;
    for (unsigned k = 0; k < m_plots.size(); k++)
    {
        plot_ref& ref = m_plots[k];
        if (ref.is_dirty)
        {
            plot_draw(k, ww, hh);
        }
    }
    return 1;
}

void fx_plot_canvas::plots_set_to_dirty()
{
    for (unsigned k = 0; k < m_plots.size(); k++)
    {
        plot_ref& ref = m_plots[k];
        ref.is_image_dirty = true;
    }    
}
