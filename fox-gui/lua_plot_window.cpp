
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua_plot_window.h"
#include "gsl_shell_app.h"
#include "window_registry.h"
#include "fx_plot_window.h"
#include "lua-cpp-utils.h"
#include "lua-graph.h"
#include "gs-types.h"
#include "plot.h"

__BEGIN_DECLS

static int fox_window_new             (lua_State *L);
static int fox_window_free            (lua_State *L);
static int fox_window_close           (lua_State *L);
static int fox_window_attach          (lua_State *L);
static int fox_window_slot_refresh    (lua_State *L);

static const struct luaL_Reg fox_window_functions[] = {
  {"window",         fox_window_new},
  {NULL, NULL}
};

static const struct luaL_Reg fox_window_methods[] = {
  {"attach",         fox_window_attach        },
  {"close",          fox_window_close        },
  {"refresh",        fox_window_slot_refresh        },
  {"__gc",           fox_window_free       },
  {NULL, NULL}
};

__END_DECLS

typedef plot<sg_object, manage_owner> sg_plot;

int
fox_window_new (lua_State *L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, "__fox_app");
  gsl_shell_app* app = (gsl_shell_app*) lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (unlikely(app == NULL))
    return luaL_error(L, "cannot create window: FOX application not found");

  fx_plot_window* win = new(L, GS_FOX_WINDOW) fx_plot_window(app, "GSL Shell FX plot", NULL, NULL, 640, 480);

  win->lua_id = window_index_add (L, -1);

  win->setTarget(app);
  app->window_create_request(win);

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
fox_window_slot_refresh (lua_State *L)
{
  fx_plot_window *win = object_check<fx_plot_window>(L, 1, GS_FOX_WINDOW);
  fx_plot_canvas& canvas = win->canvas();

  gsl_shell_app* app = (gsl_shell_app*) win->getApp();

  bool interrupted = app->interrupt();

  if (canvas.is_ready())
    {
      int ww = canvas.getWidth(), hh = canvas.getHeight();
      agg::trans_affine m(double(ww), 0.0, 0.0, double(hh), 0.0, 0.0);

      AGG_LOCK();
      opt_rect<double> rect = canvas.incremental_draw(m);
      AGG_UNLOCK();

      if (rect.is_defined())
	{
	  const int m = 4;
	  const agg::rect_base<double>& r = rect.rect();
	  const agg::rect_base<int> ri(r.x1 - m, r.y1 - m, r.x2 + m, r.y2 + m);
	  canvas.update_region(ri);
	}
    }

  app->resume(interrupted);

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
