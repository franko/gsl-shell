#include "window_surface.h"

#include "util/agg_color_conv_rgb8.h"

#include "fatal.h"
#include "lua-graph.h"

void plot_ref::attach(sg_plot* p)
{
    plot = p;
    is_image_dirty = true;
    dirty_rect.clear();
}

window_surface::window_surface(const char* split_str):
m_img(), m_save_img(), m_canvas(0)
{
    split(split_str ? split_str : ".");
}

window_surface::~window_surface()
{
    delete m_canvas;
}

void window_surface::split(const char* split_str)
{
    m_part.parse(split_str);
    m_part.split();

    m_plots.clear();
    plot_ref empty;
    for (unsigned k = 0; k < m_part.get_slot_number(); k++)
        m_plots.add(empty);
}

bool window_surface::resize(unsigned ww, unsigned hh)
{
    fprintf(stderr, "window_surface::resize to size: %u %u\n", ww, hh);

    m_save_img.clear();

    if (likely(m_img.resize(ww, hh)))
    {
        m_canvas = new(std::nothrow) canvas(m_img, ww, hh, colors::white);
        plots_set_to_dirty();
        return (m_canvas != NULL);
    }
    return false;
}

bool window_surface::ensure_canvas_size(unsigned ww, unsigned hh)
{
    if (unlikely(m_img.width() != ww || m_img.height() != hh))
    {
        return resize(ww, hh);
    }
    return true;
}

void window_surface::render(plot_ref& ref, const agg::rect_i& r)
{
    fprintf(stderr, "window_surface::render rendering using area: %i %i %i %i\n", r.x1, r.y1, r.x2, r.y2);
    m_canvas->clear_box(r);
    if (ref.plot)
    {
        graph_mutex::lock();
        ref.plot->draw(*m_canvas, r, &ref.inf);
        graph_mutex::unlock();
    }
    if (!ref.plot)
        fprintf(stderr, "window_surface::render WARNING: undefined plot\n");
}

agg::rect_i window_surface::plot_draw(unsigned index, int canvas_width, int canvas_height)
{
    fprintf(stderr, "window_surface::plot_draw plot %i, ww: %i, hh: %i\n", index, canvas_width, canvas_height);

    plot_ref& ref = m_plots[index];
    agg::rect_i r = m_part.rect(index, canvas_width, canvas_height);

    if (ref.is_image_dirty)
    {
        render(ref, r);
        ref.is_image_dirty = false;
        fprintf(stderr, "window_surface::plot_draw drawing done.\n");
    }
    else
        fprintf(stderr, "window_surface::plot_draw drawing not needed.\n");
    return r;
}

agg::rect_i window_surface::plot_draw(unsigned index)
{
    return plot_draw(index, get_width(), get_height());
}

opt_rect<double>
window_surface::plot_render_queue(plot_ref& ref, const agg::rect_i& box)
{
    fprintf(stderr, "window_surface::plot_render_queue rect: %i %i %i %i\n", box.x1, box.y1, box.x2, box.y2);

    const agg::trans_affine m = affine_matrix(box);
    opt_rect<double> r, draw_rect;

    graph_mutex::lock();
    ref.plot->draw_queue(*m_canvas, m, ref.inf, draw_rect);
    graph_mutex::unlock();

    r.add<rect_union>(draw_rect);
    r.add<rect_union>(ref.dirty_rect);
    ref.dirty_rect = draw_rect;

    if (r.is_defined())
    {
        const agg::rect_d& rx = r.rect();
        fprintf(stderr, "window_surface::plot_render_queue Update RECT: %g %g %g %g\n", rx.x1, rx.y1, rx.x2, rx.y2);
    }
    else
    {
        fprintf(stderr, "window_surface::plot_render_queue Update rect: EMPTY\n");
    }
    return r;
}

agg::rect_i
window_surface::plot_draw_queue(unsigned index, bool draw_all)
{
    int canvas_width = get_width(), canvas_height = get_height();

    plot_ref& ref = m_plots[index];

    if (!ref.plot)
        fatal_exception("call to plot_draw_queue for undefined plot");

    agg::rect_i r = m_part.rect(index, canvas_width, canvas_height);
    opt_rect<double> rect = plot_render_queue(ref, r);

    if (draw_all)
    {
        fprintf(stderr, "window_surface::plot_draw_queue UPDATE PLOT RECTANGLE.\n");
        return r;
    }

    if (rect.is_defined())
    {
        fprintf(stderr, "window_surface::plot_draw_queue UPDATE ONLY RECTANGLE.\n");
        const int pd = 4;
        const agg::rect_d& ur = rect.rect();
        const agg::rect_i box(0, 0, canvas_width, canvas_height);
        r = agg::rect_i(ur.x1 - pd, ur.y1 - pd, ur.x2 + pd, ur.y2 + pd);
        r.clip(box);
    }
    else
    {
        fprintf(stderr, "window_surface::plot_draw_queue EMPTY UPDATE RECT.\n");
        r.x2 = r.x1;
        r.y2 = r.y1;
    }

    return r;
}

int window_surface::attach(sg_plot* p, const char* slot_str)
{
    int index = m_part.get_slot_index(slot_str);
    if (index >= 0)
        m_plots[index].attach(p);
    return index;
}

bool window_surface::save_plot_image(unsigned index)
{
    int ww = get_width(), hh = get_height();

    if (!m_save_img.ensure_size(ww, hh)) return false;

    fprintf(stderr, "window_surface::save_plot_image saving: %i\n", index);

    agg::rect_i r = plot_draw(index, ww, hh);
    image::copy_region(m_save_img, m_img, r);
    return true;
}

bool window_surface::restore_plot_image(unsigned index)
{
    if (unlikely(!m_save_img.defined()))
        return false;

    fprintf(stderr, "window_surface::restore_plot_image restoring: %i\n", index);

    int ww = get_width(), hh = get_height();
    agg::rect_i r = m_part.rect(index, ww, hh);
    image::copy_region(m_img, m_save_img, r);
    return true;
}

agg::rect_i window_surface::get_plot_area(unsigned index, int canvas_width, int canvas_height)
{
    return m_part.rect(index, canvas_width, canvas_height);
}

void window_surface::plots_set_to_dirty()
{
    for (unsigned k = 0; k < m_plots.size(); k++)
    {
        plot_ref& ref = m_plots[k];
        ref.is_image_dirty = true;
    }    
}
