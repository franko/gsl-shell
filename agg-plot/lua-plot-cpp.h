#ifndef AGGPLOT_LUA_PLOT_CPP_H
#define AGGPLOT_LUA_PLOT_CPP_H

#include "lua-plot.h"

extern "C" {
#include "lua.h"
}

#include "plot.h"
#include "resource-manager.h"
#include "drawable.h"

class lua_plot {
public:
  typedef plot<drawable, lua_management> plot_type;

private:
  plot_type m_plot;

public:
  lua_plot() : m_plot(), id(-1) { };

  void update_window(lua_State *L);

  plot_type& self() { return m_plot; };

  static lua_plot *check(lua_State *L, int index);

  int id;
};

#endif
