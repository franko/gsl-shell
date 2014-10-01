#include <string.h>

#include "image_write.h"

#include "platform/agg_platform_support.h"
#include "rendering_buffer_utils.h"
#include "platform/win32/agg_win32_bmp.h"
#include "util/agg_color_conv_rgb8.h"

static inline void pixel_map_attach (agg::pixel_map& pm, agg::rendering_buffer *rbuf,
                                     bool flip_y)
{
    int stride = pm.stride();
    rbuf->attach(pm.buf(), pm.width(), pm.height(), flip_y ? stride : -stride);
}

bool
save_image_file(agg::rendering_buffer& src, const char *fn, agg::pix_format_e src_pixfmt_tag)
{
    unsigned slen = strlen(fn);
    agg::pod_array<char> fnext(slen+5);
    sprintf (fnext.data(), "%s.bmp", fn);

    agg::pixel_map pmap;
    pmap.create(src.width(), src.height(), agg::org_e(24));

    agg::rendering_buffer rbuf_tmp;
    pixel_map_attach (pmap, &rbuf_tmp, true);

    if (src_pixfmt_tag == agg::pix_format_bgr24)
    {
        rbuf_tmp.copy_from(src);
    }
    else if (src_pixfmt_tag == agg::pix_format_rgb24)
    {
        my_color_conv(&rbuf_tmp, &src, agg::color_conv_rgb24_to_bgr24());
    }

    return pmap.save_as_bmp(fnext.data());
}
