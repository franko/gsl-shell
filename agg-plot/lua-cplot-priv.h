#ifndef LUA_CPLOT_PRIV_H
#define LUA_CPLOT_PRIV_H

#include <pthread.h>

#include "defs.h"
#include "agg-cplot.h"

__BEGIN_DECLS

enum line_cmd_e {
  CMD_ERROR = -1,
  CMD_MOVE_TO = 0,
  CMD_LINE_TO,
  CMD_CLOSE,
  CMD_SET_DASH,
  CMD_ADD_DASH,
  CMD_ARC_TO,
  CMD_CURVE3,
  CMD_CURVE4,
};

struct cmd_call_stack {
  enum line_cmd_e cmd;
  const char *signature;
  double f[6];
  int b[2];
};

struct lcplot {
  cplot *plot;
  pthread_mutex_t mutex[1];
  int lua_is_owner;
  int is_shown;
  void *window;
};

extern void lcplot_destroy (struct lcplot *cp);
extern void update_callback (void *_app);

__END_DECLS

#endif
