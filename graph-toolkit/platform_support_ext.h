#ifndef AGGPLOT_PLATFORM_SUPPORT_H
#define AGGPLOT_PLATFORM_SUPPORT_H

#include "agg_basics.h"
#include "platform/agg_platform_support.h"
#include "rendering_buffer_utils.h"

class platform_support_ext : public agg::platform_support {
public:
    platform_support_ext (agg::pix_format_e format, bool flip_y)
        : agg::platform_support(format, flip_y)
    { };

    void lock();
    void unlock();

    bool is_mapped();
    void close_request();
    void update_region (const agg::rect_base<int>& r);
    void do_window_update ();

    static void prepare();
};

#endif
