#ifndef AGGPLOT_PATH_H
#define AGGPLOT_PATH_H

#include "scalable.h"

#include "agg_path_storage.h"
#include "agg_ellipse.h"

namespace draw {

  typedef vs_proxy<agg::path_storage, true> path;
  typedef vs_proxy_approx<agg::ellipse, true> ellipse;
}

#endif
