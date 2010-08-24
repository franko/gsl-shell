#ifndef AGGPLOT_PLATFORM_SUPPORT_H
#define AGGPLOT_PLATFORM_SUPPORT_H

#include "agg_basics.h"
#include "platform/agg_platform_support.h"

extern void platform_support_prepare       ();
extern void platform_support_lock          (agg::platform_support *app);
extern void platform_support_unlock        (agg::platform_support *app);
extern bool platform_support_is_mapped     (agg::platform_support *app);
extern void platform_support_close_window  (agg::platform_support *app);
extern void platform_support_update_region (agg::platform_support *app, 
					    const agg::rect_base<int>& r);

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

template<class T> class row_accessor_ro
{
public:
  //--------------------------------------------------------------------
  row_accessor_ro(const T* buf, unsigned width, unsigned height, int stride) :
    m_buf(buf), m_width(width), m_height(height), m_stride(stride)
  {
    if(stride < 0) 
      m_start = m_buf - int(height - 1) * stride;
    else
      m_start = m_buf;
  }

  //--------------------------------------------------------------------
  const T* buf()    const { return m_buf;    }
  unsigned width()  const { return m_width;  }
  unsigned height() const { return m_height; }
  int      stride() const { return m_stride; }
  unsigned stride_abs() const 
  {
    return (m_stride < 0) ? unsigned(-m_stride) : unsigned(m_stride); 
  }

  //--------------------------------------------------------------------
  const T* row_ptr(int, int y, unsigned) const { return m_start + y * m_stride; }
  const T* row_ptr(int y) const { return m_start + y * m_stride; }

private:
  //--------------------------------------------------------------------
  const T*      m_buf;    // Pointer to renrdering buffer
  unsigned      m_width;  // Width in pixels
  unsigned      m_height; // Height in pixels
  int           m_stride; // Number of bytes per row. Can be < 0
  const T*      m_start;  // Pointer to first pixel depending on stride 
};

#endif
