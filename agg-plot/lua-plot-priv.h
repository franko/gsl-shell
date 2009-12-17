#ifndef LUA_CPLOT_PRIV_H
#define LUA_CPLOT_PRIV_H

#include <pthread.h>

#include "defs.h"
#include "c-drawables.h"

__BEGIN_DECLS

enum path_cmd_e {
  CMD_ERROR = -1,
  CMD_MOVE_TO = 0,
  CMD_LINE_TO,
  CMD_CLOSE,
  CMD_ARC_TO,
  CMD_CURVE3,
  CMD_CURVE4,
};

struct cmd_call_stack {
  double f[6];
  int b[2];
};

struct agg_plot {
  CPLOT *plot;
  pthread_mutex_t mutex[1];
  int lua_is_owner;
  int is_shown;
  void *window;
};

enum trans_type {
  trans_end = -1,
  trans_stroke = 0,
  trans_curve,
  trans_resize,
};

struct stroke_spec {
  double width;
};

struct curve_spec {
  int stub;
};

struct trans_spec {
  enum trans_type tag;
  union {
    struct stroke_spec stroke;
    struct curve_spec  curve;
  } content;
};

extern void agg_plot_destroy (struct agg_plot *cp);
extern void update_callback (void *_app);

__END_DECLS

#endif
