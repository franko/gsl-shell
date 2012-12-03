#ifndef AGGPLOT_CANVAS_H
#define AGGPLOT_CANVAS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "pixel_fmt.h"
#include "sg_object.h"

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_primitives.h"
#include "agg_rasterizer_outline.h"
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
        pixel_buffer(ren_buf), renderer_base(pixel_buffer), m_ren_solid(renderer_base),
        m_bgcol(bg_color)
    { }

    typedef Pixel pixfmt_type;

    double width()  const {
        return pixel_buffer.width();
    };
    double height() const {
        return pixel_buffer.height();
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
        renderer_base.clear(c);
    }

    void clear_box(const agg::rect_base<int>& r)
    {
        for (int y = r.y1; y < r.y2; y++)
            renderer_base.copy_hline (r.x1, y, r.x2, m_bgcol);
    }

    void clip_box(const agg::rect_base<int>& clip)
    {
        renderer_base.clip_box_naked(clip.x1, clip.y1, clip.x2, clip.y2);
    }

    void reset_clipping() {
        renderer_base.reset_clipping(true);
    }

    template <class Rasterizer, class Scanline>
    void render_scanlines(Rasterizer& ras, Scanline& sl)
    {
        agg::render_scanlines(ras, sl, m_ren_solid);
    }

    Pixel pixel_buffer;
    agg::renderer_base<Pixel> renderer_base;

private:
    agg::renderer_scanline_aa_solid<agg::renderer_base<Pixel> > m_ren_solid;
    agg::rgba8 m_bgcol;
};

template <class Pixel, class PixelNoSub>
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
        pixel_buffer(ren_buf), renderer_base(pixel_buffer),
        m_pixbuf(ren_buf), m_ren_base(m_pixbuf), m_ren_solid(m_ren_base),
        m_bgcol(bg_color)
    { }

    typedef Pixel pixfmt_type;

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

    PixelNoSub pixel_buffer;
    agg::renderer_base<PixelNoSub> renderer_base;

private:
    pixfmt_type m_pixbuf;
    agg::renderer_base<pixfmt_type> m_ren_base;
    agg::renderer_scanline_aa_solid<agg::renderer_base<pixfmt_type> > m_ren_solid;
    agg::rgba8 m_bgcol;
};

template <class Renderer, class PixelNoSub>
class canvas_gen : public Renderer {

    typedef typename Renderer::pixfmt_type pixfmt_type;

    enum { line_width = 120 };

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;

    typedef agg::renderer_primitives<agg::renderer_base<PixelNoSub> > renderer_prim;
    renderer_prim m_ren_prim;
    agg::rasterizer_outline<renderer_prim> m_ras_outline;

public:
    canvas_gen(agg::rendering_buffer& ren_buf, double width, double height,
               agg::rgba8 bgcol):
        Renderer(ren_buf, bgcol), ras(), sl(),
        m_ren_prim(Renderer::renderer_base), m_ras_outline(m_ren_prim)
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

    void draw_outline_noaa(sg_object& vs, agg::rgba8 c)
    {
        m_ren_prim.line_color(c);
        m_ras_outline.add_path(vs);
    }
};

struct virtual_canvas {
    virtual void draw(sg_object& vs, agg::rgba8 c) = 0;
    virtual void draw_outline(sg_object& vs, agg::rgba8 c) = 0;
    virtual void draw_outline_noaa(sg_object& vs, agg::rgba8 c) = 0;

    virtual void clip_box(const agg::rect_base<int>& clip) = 0;
    virtual void reset_clipping() = 0;

    virtual ~virtual_canvas() { }
};

#ifdef DISABLE_SUBPIXEL_AA
typedef canvas_gen<renderer_gray_aa<pixel_type>, pixel_type> canvas;
#else
typedef canvas_gen<renderer_subpixel_aa<pixel_lcd_type, pixel_type>, pixel_type> canvas;
#endif

#endif
