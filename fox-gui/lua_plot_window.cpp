
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua_plot_window.h"
#include "fx_plot_window.h"
#include "lua-cpp-utils.h"
#include "gs-types.h"
#include "plot.h"

__BEGIN_DECLS

static int fox_window_new             (lua_State *L);
static int fox_window_free            (lua_State *L);
static int fox_window_close           (lua_State *L);
static int fox_window_attach          (lua_State *L);

static const struct luaL_Reg fox_window_functions[] = {
  {"window",         fox_window_new},
  {NULL, NULL}
};

static const struct luaL_Reg fox_window_methods[] = {
  {"attach",         fox_window_attach        },
  {"close",          fox_window_close        },
  {"__gc",           fox_window_free       },
  {NULL, NULL}
};

__END_DECLS

typedef plot<sg_object, manage_owner> sg_plot;

int
fox_window_new (lua_State *L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, "__fox_app");
  FXApp* app = (FXApp*) lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (unlikely(app == NULL))
    return luaL_error(L, "cannot create window: FOX application not found");

  fx_plot_window* win = new(L, GS_FOX_WINDOW) fx_plot_window(app, "GSL Shell FX plot", NULL, NULL, 640, 480);

  win->create();
  win->show(PLACEMENT_SCREEN);

  return 1;
}

int
fox_window_free (lua_State *L)
{
  fx_plot_window *win = object_check<fx_plot_window>(L, 1, GS_FOX_WINDOW);
  win->~fx_plot_window();
  return 0;
}

int
fox_window_attach (lua_State *L)
{
  fx_plot_window *win = object_check<fx_plot_window>(L, 1, GS_FOX_WINDOW);
  sg_plot* p = object_check<sg_plot>(L, 2, GS_PLOT);
  win->canvas().attach(p);
  return 0;
}

int
fox_window_close (lua_State *L)
{
  return 0;
}

int
fox_window_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_FOX_WINDOW));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, fox_window_methods);
  lua_pop (L, 1);

  luaL_register (L, "fox", fox_window_functions);
  lua_pop (L, 1);
  return 0;
}
