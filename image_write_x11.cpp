#include <string.h>
#include <stdio.h>

#include "image_write.h"

#include "platform/agg_platform_support.h"
#include "rendering_buffer_utils.h"
#include "util/agg_color_conv_rgb8.h"

bool
save_image_file(agg::rendering_buffer& rbuf, const char *fn, agg::pix_format_e src_pixfmt_tag)
{
    unsigned slen = strlen(fn);
    agg::pod_array<char> fnext(slen+5);
    sprintf(fnext.data(), "%s.ppm", fn);

    FILE* fd = fopen(fnext.data(), "wb");
    if(fd == 0) return false;

    unsigned w = rbuf.width();
    unsigned h = rbuf.height();

    fprintf(fd, "P6\n%d %d\n255\n", w, h);

    unsigned y;
    agg::pod_array<unsigned char> row_buf(w * 3);
    unsigned char *tmp_buf = row_buf.data();

    for(y = 0; y < rbuf.height(); y++)
    {
        const unsigned char* src = rbuf.row_ptr(h - 1 - y);
        switch(src_pixfmt_tag)
        {
        default:
            break;
        case agg::pix_format_rgb555:
            agg::color_conv_row(tmp_buf, src, w, agg::color_conv_rgb555_to_rgb24());
            break;

        case agg::pix_format_rgb565:
            agg::color_conv_row(tmp_buf, src, w, agg::color_conv_rgb565_to_rgb24());
            break;

        case agg::pix_format_bgr24:
            agg::color_conv_row(tmp_buf, src, w, agg::color_conv_bgr24_to_rgb24());
            break;

        case agg::pix_format_rgb24:
            agg::color_conv_row(tmp_buf, src, w, agg::color_conv_rgb24_to_rgb24());
            break;

        case agg::pix_format_rgba32:
            agg::color_conv_row(tmp_buf, src, w, agg::color_conv_rgba32_to_rgb24());
            break;

        case agg::pix_format_argb32:
            agg::color_conv_row(tmp_buf, src, w, agg::color_conv_argb32_to_rgb24());
            break;

        case agg::pix_format_bgra32:
            agg::color_conv_row(tmp_buf, src, w, agg::color_conv_bgra32_to_rgb24());
            break;

        case agg::pix_format_abgr32:
            agg::color_conv_row(tmp_buf, src, w, agg::color_conv_abgr32_to_rgb24());
            break;
        }
        fwrite(tmp_buf, 1, w * 3, fd);
    }

    fclose(fd);
    return true;
}
