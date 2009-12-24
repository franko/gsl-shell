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
  int lua_is_owner;
  int is_shown;
  void *window;
};

enum trans_type {
  trans_end = -1,
  trans_stroke = 0,
  trans_curve,
  trans_dash,
  trans_marker,
  trans_rotate,
  trans_translate,
};

struct property_reg {
  int id;
  const char *name;
};

struct stroke_spec {
  double width;
  int line_cap;
  int line_join;
};

struct dash_spec {
  double len[2];
};

struct marker_spec {
  double size;
};

struct rotate_spec {
  double angle;
};


struct translate_spec {
  double x, y;
};

struct trans_spec {
  enum trans_type tag;
  union {
    struct stroke_spec    stroke;
    struct dash_spec      dash;
    struct marker_spec    marker;
    struct translate_spec translate;
    struct rotate_spec    rotate;
  } content;
};

extern struct property_reg line_cap_properties[];
extern struct property_reg line_join_properties[];

extern void agg_plot_destroy (struct agg_plot *cp);
extern void update_callback (void *_app);

extern pthread_mutex_t agg_mutex[1];

__END_DECLS

#endif
