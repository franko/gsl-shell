#ifndef AGGPLOT_MARKERS_H
#define AGGPLOT_MARKERS_H

#include "sg_object.h"

extern sg_object* new_marker_symbol(const char *name);
extern sg_object* new_marker_symbol(int n);
extern sg_object* new_marker_symbol_raw(const char *req_name, bool& stroke);

#endif
