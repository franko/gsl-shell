#ifndef AGGPLOT_CANVAS_H
#define AGGPLOT_CANVAS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "pixel_fmt.h"
#include "sg_object.h"

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_trans_viewport.h"
#include "agg_conv_stroke.h"

template <class Pixel>
class renderer_gray_aa
{
public:
    renderer_gray_aa(agg::rendering_buffer& ren_buf, agg::rgba8 bg_color):
        m_pixbuf(ren_buf), m_ren_base(m_pixbuf), m_ren_solid(m_ren_base),
        m_bgcol(bg_color)
    { }

    typedef Pixel pixfmt_type;

    agg::renderer_base<Pixel>& renderer_base() {
        return m_ren_base;
    }

    double width()  const {
        return m_pixbuf.width();
    };
    double height() const {
        return m_pixbuf.height();
    };

    template <class Rasterizer, class VertexSource>
    static void add_path(Rasterizer& ras, VertexSource& vs)
    {
        ras.add_path(vs);
    }

    void color(agg::rgba8 c) {
        m_ren_solid.color(c);
    }

    void clear(agg::rgba8 c) {
        m_ren_base.clear(c);
    }

    void clear_box(const agg::rect_base<int>& r)
    {
        for (int y = r.y1; y < r.y2; y++)
            m_ren_base.copy_hline (r.x1, y, r.x2, m_bgcol);
    }

    void clip_box(const agg::rect_base<int>& clip)
    {
        m_ren_base.clip_box_naked(clip.x1, clip.y1, clip.x2, clip.y2);
    }

    void reset_clipping() {
        m_ren_base.reset_clipping(true);
    }

    template <class Rasterizer, class Scanline>
    void render_scanlines(Rasterizer& ras, Scanline& sl)
    {
        agg::render_scanlines(ras, sl, m_ren_solid);
    }
private:
    Pixel m_pixbuf;
    agg::renderer_base<Pixel> m_ren_base;
    agg::renderer_scanline_aa_solid<agg::renderer_base<Pixel> > m_ren_solid;
    agg::rgba8 m_bgcol;
};

template <class Pixel>
class renderer_subpixel_aa
{
    enum { subpixel_scale = 3 };

    struct subpixel_scale_trans
    {
        void transform(double* x, double* y) const {
            *x = subpixel_scale * (*x);
        }
    };

public:
    renderer_subpixel_aa(agg::rendering_buffer& ren_buf, agg::rgba8 bg_color):
        m_pixbuf(ren_buf), m_ren_base(m_pixbuf), m_ren_solid(m_ren_base),
        m_bgcol(bg_color)
    { }

    typedef Pixel pixfmt_type;

    agg::renderer_base<pixfmt_type>& renderer_base() {
        return m_ren_base;
    }

    double width()  const {
        return m_pixbuf.width() / subpixel_scale;
    };
    double height() const {
        return m_pixbuf.height();
    };

    void clear(agg::rgba8 c) {
        m_ren_base.clear(c);
    }

    void clear_box(const agg::rect_base<int>& r)
    {
        int x1 = subpixel_scale * r.x1, x2 = subpixel_scale * r.x2;
        for (int y = r.y1; y < r.y2; y++)
            m_ren_base.copy_hline (x1, y, x2, m_bgcol);
    }

    void clip_box(const agg::rect_base<int>& clip)
    {
        int x1 = subpixel_scale * clip.x1, x2 = subpixel_scale * clip.x2;
        m_ren_base.clip_box_naked(x1, clip.y1, x2, clip.y2);
    }

    void reset_clipping() {
        m_ren_base.reset_clipping(true);
    }

    template <class Rasterizer, class VertexSource>
    static void add_path(Rasterizer& ras, VertexSource& vs)
    {
        subpixel_scale_trans tr;
        agg::conv_transform<VertexSource, subpixel_scale_trans> scaled_vs(vs, tr);
        ras.add_path(scaled_vs);
    }

    void color(agg::rgba8 c) {
        m_ren_solid.color(c);
    }

    template <class Rasterizer, class Scanline>
    void render_scanlines(Rasterizer& ras, Scanline& sl)
    {
        agg::render_scanlines(ras, sl, m_ren_solid);
    }

private:
    pixfmt_type m_pixbuf;
    agg::renderer_base<pixfmt_type> m_ren_base;
    agg::renderer_scanline_aa_solid<agg::renderer_base<pixfmt_type> > m_ren_solid;
    agg::rgba8 m_bgcol;
};

template <class Renderer>
class canvas_gen : public Renderer {

    typedef typename Renderer::pixfmt_type pixfmt_type;

    enum { line_width = 120 };

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;

public:
    canvas_gen(agg::rendering_buffer& ren_buf, double width, double height,
               agg::rgba8 bgcol):
        Renderer(ren_buf, bgcol), ras(), sl()
    { }

    void draw(sg_object& vs, agg::rgba8 c)
    {
        this->add_path(this->ras, vs);
        this->color(c);
        this->render_scanlines(this->ras, this->sl);
    }

    void draw_outline(sg_object& vs, agg::rgba8 c)
    {
        agg::conv_stroke<sg_object> line(vs);
        line.width(line_width / 100.0);
        line.line_cap(agg::round_cap);
        this->add_path(this->ras, line);
        this->color(c);
        this->render_scanlines(this->ras, this->sl);
    }
};

struct virtual_canvas {
    virtual void draw(sg_object& vs, agg::rgba8 c) = 0;
    virtual void draw_outline(sg_object& vs, agg::rgba8 c) = 0;

    virtual void clip_box(const agg::rect_base<int>& clip) = 0;
    virtual void reset_clipping() = 0;

    virtual ~virtual_canvas() { }
};

#ifdef DISABLE_SUBPIXEL_AA
typedef canvas_gen<renderer_gray_aa<pixel_type> > canvas;
#else
typedef canvas_gen<renderer_subpixel_aa<pixel_lcd_type> > canvas;
#endif

#endif
