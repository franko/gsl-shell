#ifndef LUA_CPLOT_H
#define LUA_CPLOT_H

#include "defs.h"

__BEGIN_DECLS
#include "lua.h"
__END_DECLS

#ifdef __cplusplus

#include "plot.h"
#include "resource-manager.h"

typedef plot<vertex_source, lua_management> plot_type;

struct agg_plot : public plot_type {
  bool is_shown;
  void *window;
  int id;

  agg_plot() : plot_type(), is_shown(false), window(NULL), id(0) {};

  void wait_update();

  static agg_plot* arg_check(lua_State *L, int index);
};

#endif

__BEGIN_DECLS

extern void plot_register (lua_State *L);

__END_DECLS

#endif
