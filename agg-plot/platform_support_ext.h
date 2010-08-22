#ifndef AGGPLOT_PLATFORM_SUPPORT_H
#define AGGPLOT_PLATFORM_SUPPORT_H

#include "agg_basics.h"
#include "platform/agg_platform_support.h"

extern void platform_support_prepare      ();
extern void platform_support_lock         (agg::platform_support *app);
extern void platform_support_unlock       (agg::platform_support *app);
extern bool platform_support_is_mapped    (agg::platform_support *app);
extern void platform_support_close_window (agg::platform_support *app);
extern void platform_support_update_region (agg::platform_support *app, 
					    const agg::rect_base<int>& r);

#endif
