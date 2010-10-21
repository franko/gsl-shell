#ifndef AGGPLOT_PATH_H
#define AGGPLOT_PATH_H

#include "scalable.h"

#include "agg_path_storage.h"
#include "agg_ellipse.h"

namespace draw {

  typedef vs_proxy<agg::path_storage, false> path;
  typedef vs_proxy<agg::ellipse, true> ellipse;
}

#endif
