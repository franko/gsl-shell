#ifndef AGGPLOT_PATH_H
#define AGGPLOT_PATH_H

#include "sg_object.h"

#include "agg_path_storage.h"
#include "agg_ellipse.h"

namespace draw {

typedef sg_object_gen<agg::path_storage, no_approx_scale> path;
typedef sg_object_gen<agg::ellipse, approx_scale> ellipse;
}

#endif
