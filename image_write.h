#ifndef IMAGE_WRITE_H
#define IMAGE_WRITE_H

#include "platform/agg_platform_support.h"

extern bool save_image_file(agg::rendering_buffer& src, const char *fn, agg::pix_format_e src_pixfmt_tag);

#endif
