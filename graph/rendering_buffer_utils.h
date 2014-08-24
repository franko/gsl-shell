#ifndef AGGPLOT_RBUF_UTILS_H
#define AGGPLOT_RBUF_UTILS_H

template<class RenBufDst, class RenBufSrc, class CopyRow>
void my_color_conv(RenBufDst* dst, const RenBufSrc* src, CopyRow copy_row_functor)
{
    unsigned int width  = src->width();
    unsigned int height = src->height();

    for(unsigned int y = 0; y < height; y++)
    {
        copy_row_functor(dst->row_ptr(0, y, width), src->row_ptr(y), width);
    }
}

template<class RenBufDst, class RenBufSrc>
void rendering_buffer_get_region (RenBufDst& dst, RenBufSrc& src, agg::rect_base<int>& r,
                                  unsigned pixel_width)
{
    int w = r.x2 - r.x1, h = r.y2 - r.y1;
    for (int y = 0; y < h; y++)
    {
        unsigned char *drow = dst.row_ptr(y);
        unsigned char *srow = src.row_ptr(r.y1 + y);
        srow += r.x1 * pixel_width;
        memcpy (drow, srow, pixel_width * w);
    }
}

template<class RenBufDst, class RenBufSrc>
void rendering_buffer_get_const_view (RenBufDst& view, const RenBufSrc& src,
                                      const agg::rect_base<int>& r, unsigned pixel_width)
{
    int x = r.x1, y = r.y1, w = r.x2 - r.x1, h = r.y2 - r.y1;
    const unsigned char *buf_start = src.row_ptr(y);
    if (src.stride() < 0)
        buf_start += src.stride() * (h - 1);
    view.attach(buf_start + pixel_width * x, w, h, src.stride());
}

template<class RenBufDst, class RenBufSrc>
void rendering_buffer_get_view (RenBufDst& view, RenBufSrc& src,
                                const agg::rect_base<int>& r, unsigned pixel_width)
{
    int x = r.x1, y = r.y1, w = r.x2 - r.x1, h = r.y2 - r.y1;
    unsigned char *buf_start = src.row_ptr(y);
    if (src.stride() < 0)
        buf_start += src.stride() * (h - 1);
    view.attach(buf_start + pixel_width * x, w, h, src.stride());
}

template<class RenBufDst, class RenBufSrc>
void rendering_buffer_put_region (RenBufDst& dst, RenBufSrc& src, agg::rect_base<int>& r,
                                  unsigned pixel_width)
{
    int w = r.x2 - r.x1, h = r.y2 - r.y1;
    for (int y = 0; y < h; y++)
    {
        unsigned char *drow = dst.row_ptr(r.y1 + y);
        unsigned char *srow = src.row_ptr(y);
        drow += r.x1 * pixel_width;
        memcpy (drow, srow, pixel_width * w);
    }
}

template<class T> class row_accessor_ro
{
public:
    //--------------------------------------------------------------------
    row_accessor_ro() : m_buf(0), m_width(0), m_height(0), m_stride(0), m_start(0) {};

    row_accessor_ro(const T* buf, unsigned width, unsigned height, int stride) :
        m_buf(buf), m_width(width), m_height(height), m_stride(stride)
    {
        if(stride < 0)
            m_start = m_buf - int(height - 1) * stride;
        else
            m_start = m_buf;
    }

    void attach(const T* buf, unsigned width, unsigned height, int stride)
    {
        m_buf = m_start = buf;
        m_width = width;
        m_height = height;
        m_stride = stride;
        if(stride < 0)
        {
            m_start = m_buf - int(height - 1) * stride;
        }
    };

    //--------------------------------------------------------------------
    const T* buf()    const {
        return m_buf;
    }
    unsigned width()  const {
        return m_width;
    }
    unsigned height() const {
        return m_height;
    }
    int      stride() const {
        return m_stride;
    }
    unsigned stride_abs() const
    {
        return (m_stride < 0) ? unsigned(-m_stride) : unsigned(m_stride);
    }

    //--------------------------------------------------------------------
    const T* row_ptr(int, int y, unsigned) const {
        return m_start + y * m_stride;
    }
    const T* row_ptr(int y) const {
        return m_start + y * m_stride;
    }

private:
    //--------------------------------------------------------------------
    const T*      m_buf;    // Pointer to renrdering buffer
    unsigned      m_width;  // Width in pixels
    unsigned      m_height; // Height in pixels
    int           m_stride; // Number of bytes per row. Can be < 0
    const T*      m_start;  // Pointer to first pixel depending on stride
};

typedef row_accessor_ro<unsigned char> rendering_buffer_ro;

#endif
