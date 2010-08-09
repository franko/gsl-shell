
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "window-cpp.h"
#include "lua-draw.h"
#include "lua-cpp-utils.h"
#include "gs-types.h"
#include "object-refs.h"
#include "colors.h"
#include "lua-plot-cpp.h"

__BEGIN_DECLS

static int window_new        (lua_State *L);
static int window_free       (lua_State *L);
static int window_split3     (lua_State *L);
static int window_attach     (lua_State *L);

static const struct luaL_Reg window_functions[] = {
  {"window",        window_new},
  {NULL, NULL}
};

static const struct luaL_Reg window_methods[] = {
  {"attach",         window_attach        },
  {"split3",         window_split3        },
  {"update",         window_update        },
  {"__gc",           window_free       },
  {NULL, NULL}
};

__END_DECLS

void
window::on_draw_unprotected()
{
  if (! m_canvas)
    return;

  m_canvas->clear();

  pod_list<plot_matrix> *ls;

  for (ls = m_plot_matrix; ls != NULL; ls = ls->next())
    {
      plot_matrix& pm = ls->content();

      if (pm.plot)
	{
	  agg::trans_affine mtx(pm.matrix);
	  m_canvas->premultiply(mtx);
	  pm.plot->draw(*m_canvas, mtx);
	}
    }
}

void
window::on_draw()
{
  AGG_LOCK();
  on_draw_unprotected();
  AGG_UNLOCK();
}

window *
window::check (lua_State *L, int index)
{
  return (window *) gs_check_userdata (L, index, GS_WINDOW);
}

static void
set_matrix(agg::trans_affine& m, double x, double y, double sx, double sy)
{
  m.tx = x;
  m.ty = y;
  m.sx = sx;
  m.sy = sy;
}

void
window::split3()
{
  plot_matrix empty(NULL);

  pod_list<plot_matrix> *p = new pod_list<plot_matrix>(empty);
  set_matrix (p->content().matrix, 0.0, 0.5, 0.5, 0.5);

  p = new pod_list<plot_matrix>(empty, p);
  set_matrix (p->content().matrix, 0.5, 0.5, 0.5, 0.5);

  p = new pod_list<plot_matrix>(empty, p);
  set_matrix (p->content().matrix, 0.0, 0.0, 1.0, 0.5);

  m_plot_matrix = p;
}

bool
window::attach(lua_plot *plot, int slot)
{
  pod_list<plot_matrix> *ls;
  for (ls = m_plot_matrix; ls != NULL; ls = ls->next(), slot--)
    {
      if (slot == 0)
	break;
    }

  if (! ls)
    return false;

  ls->content().plot = & plot->self();
  return true;
}

int
window_new (lua_State *L)
{
  window *win = new(L, GS_WINDOW) window(colors::white);

  win->start_new_thread (L);

  return 1;
}

int
window_free (lua_State *L)
{
  window *win = window::check (L, 1);
  win->~window();
  return 0;
}


int
window_split3 (lua_State *L)
{
  window *win = window::check (L, 1);
  win->split3();
  return 0;
}

int
window_attach (lua_State *L)
{
  window *win = window::check (L, 1);
  lua_plot *plot = lua_plot::check (L, 2);
  int slot = luaL_checkinteger (L, 3);

  win->lock();

  if (win->attach (plot, slot))
    {
      plot->id = win->id;

      win->on_draw();
      win->update_window();

      win->unlock();

      object_ref_add (L, 1, 2);
    }
  else
    {
      win->unlock();
      luaL_error (L, "invalid slot");
    }

  return 0;
}

int
window_update_unprotected (lua_State *L)
{
  window *win = window::check (L, 1);

  win->on_draw_unprotected();
  win->update_window();

  return 0;
}

int
window_update (lua_State *L)
{
  window *win = window::check (L, 1);

  AGG_LOCK();
  win->on_draw_unprotected();
  win->update_window();
  AGG_UNLOCK();

  return 0;
}

void
window_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_WINDOW));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, window_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, window_functions);
}
