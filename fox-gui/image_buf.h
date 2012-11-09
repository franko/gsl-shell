#ifndef FOXGUI_IMAGE_BUF_H
#define FOXGUI_IMAGE_BUF_H

#include <new>

#include "defs.h"

#include "agg_rendering_buffer.h"
#include "rendering_buffer_utils.h"

template <unsigned PixelSize, bool FlipY>
struct image_gen : agg::rendering_buffer
{
    image_gen() { }
    image_gen(unsigned w, unsigned h)
    {
        init(w, h);
    }

    ~image_gen()
    {
        dispose();
    }

    bool defined() const
    {
        return (buf() != 0);
    }

    bool resize(unsigned w, unsigned h)
    {
        dispose();
        return init(w, h);
    }

    void clear()
    {
        dispose();
        attach(NULL, 0, 0, 0);
    }

    bool ensure_size(unsigned w, unsigned h)
    {
        if (!defined() || width() != w || height() != h)
            return resize(w, h);
        return true;
    }

    static bool match(const image_gen& a, const image_gen& b)
    {
        if (!a.defined() || !b.defined())
            return false;
        return (a.width() == b.width() && a.height() == b.height());
    }

    static void copy_region(image_gen& dest_img, const image_gen& src_img, const agg::rect_i r)
    {
        rendering_buffer_ro src;
        rendering_buffer_get_const_view(src, src_img, r, PixelSize);

        agg::rendering_buffer dest;
        rendering_buffer_get_view(dest, dest_img, r, PixelSize);

        dest.copy_from(src);
    }

private:
    bool init(unsigned w, unsigned h)
    {
        agg::int8u* data = new(std::nothrow) agg::int8u[w * h * PixelSize];
        if (likely(data))
        {
            int stride = (FlipY ? - w * PixelSize : w * PixelSize);
            attach(data, w, h, stride);
        }
        return (data != 0);
    }

    void dispose()
    {
        agg::int8u* data = buf();
        delete[] data;
    }
};

#endif
