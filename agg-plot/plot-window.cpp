
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "plot-window.h"
#include "canvas-window-cpp.h"
#include "gs-types.h"
#include "lua-utils.h"
#include "lua-cpp-utils.h"
#include "lua-draw.h"
#include "colors.h"
#include "plot.h"
#include "vertex-source.h"
#include "resource-manager.h"
#include "agg-parse-trans.h"

__BEGIN_DECLS

static int plot_window_new        (lua_State *L);
static int plot_window_add        (lua_State *L);
static int plot_window_update     (lua_State *L);
static int plot_window_add_line   (lua_State *L);
static int plot_window_index      (lua_State *L);
static int plot_window_newindex   (lua_State *L);
static int plot_window_free       (lua_State *L);
static int plot_window_title_set  (lua_State *L);
static int plot_window_title_get  (lua_State *L);
static int plot_window_units_set  (lua_State *L);
static int plot_window_units_get  (lua_State *L);

static int plot_window_add_gener (lua_State *L, bool as_line);

static const struct luaL_Reg plot_window_functions[] = {
  {"plot",        plot_window_new},
  {NULL, NULL}
};

static const struct luaL_Reg plot_window_methods[] = {
  {"add",         plot_window_add        },
  {"addline",     plot_window_add_line   },
  {"update",      plot_window_update     },
  {"__index",     plot_window_index      },
  {"__newindex",  plot_window_newindex   },
  {"__gc",        plot_window_free       },
  {NULL, NULL}
};

static const struct luaL_Reg plot_window_properties_get[] = {
  {"title",        plot_window_title_get  },
  {"units",        plot_window_units_get  },
  {NULL, NULL}
};

static const struct luaL_Reg plot_window_properties_set[] = {
  {"title",        plot_window_title_set  },
  {"units",        plot_window_units_set  },
  {NULL, NULL}
};

__END_DECLS

class plot_window : public canvas_window {
public:
  typedef plot<vertex_source, lua_management> plot_type;

  plot_window(): canvas_window(colors::white), m_plot() {};

  plot_type& get_plot() { return m_plot; };

  void on_draw_unprotected();
  virtual void on_draw();

  virtual void user_transform(agg::trans_affine& m);

  // this method should be used only when AGG is locked
  void plot_update() 
  {
    this->lock();
    this->on_draw_unprotected();
    if (this->status == plot_window::running)
      this->update_window();
    this->unlock();
  };

  static plot_window *check(lua_State *L, int index);

private:
  plot_type m_plot;
};

plot_window *
plot_window::check(lua_State *L, int index)
{
  return (plot_window *) gs_check_userdata (L, index, GS_PLOT_WINDOW);
}

void
plot_window::on_draw_unprotected()
{
  canvas& canvas = *this->m_canvas;
  canvas.clear();
  m_plot.draw(canvas);
}

void
plot_window::on_draw()
{
  AGG_PROTECT(this->on_draw_unprotected());
}


void
plot_window::user_transform(agg::trans_affine& m)
{
  m_plot.user_transform(m);
}

int
plot_window_new (lua_State *L)
{
  plot_window *p = new(L, GS_PLOT_WINDOW) plot_window();

  lua_newtable (L);
  lua_setfenv (L, -2);

  if (lua_isstring (L, 1))
    {
      const char *title = lua_tostring (L, 1);
      if (title)
	{
	  plot_window::plot_type& plt = p->get_plot();
	  plt.set_title(title);
	}
    }

  p->start_new_thread (L);

  return 1;
}

int
plot_window_free (lua_State *L)
{
  plot_window *p = plot_window::check(L, 1);
  p->~plot_window();
  return 0;
}

int
plot_window_index (lua_State *L)
{
  return mlua_index_with_properties (L, plot_window_properties_get, false);
}

int
plot_window_add_gener (lua_State *L, bool as_line)
{
  plot_window *p = plot_window::check(L, 1);
  int narg = lua_gettop (L);
  agg::rgba8 *color;

  if (narg <= 2)
    color = rgba8_push_default (L);
  else
    color = color_arg_lookup (L, 3);
      
  if (narg > 5)
    return luaL_error (L, "too much arguments if add or addline plot method");

  vertex_source *curr = check_agg_obj (L, 2);

  if (narg > 4)
    {
      curr = parse_spec_pipeline (L, 5, curr);
      lua_pop (L, 1);
    }
    
  if (curr->need_resize())
    {
      curr = new trans::resize(curr);
    }

  if (narg > 3)
    {
      curr = parse_spec_pipeline (L, 4, curr);
      lua_pop (L, 1);
    }

  lua_pushvalue (L, 1);
  mlua_fenv_addref (L, 2);
  lua_pop (L, 1);

  AGG_LOCK();

  plot_window::plot_type& plt = p->get_plot();
  plt.add(curr, color, as_line);

  if (p->status == plot_window::running)
    {
      p->on_draw_unprotected();
      p->plot_update();
    }

  AGG_UNLOCK();

  return 0;
}
 
int
plot_window_add (lua_State *L)
{
  return plot_window_add_gener (L, false);
}
 
int
plot_window_add_line (lua_State *L)
{
  return plot_window_add_gener (L, true);
}

int
plot_window_title_set (lua_State *L)
{
  plot_window *p = plot_window::check(L, 1);
  const char *title = lua_tostring (L, 2);

  if (title == NULL)
    return gs_type_error (L, 2, "string");
	  
  AGG_LOCK();

  plot_window::plot_type& plt = p->get_plot();
  plt.set_title(title);
 
  p->plot_update();

  AGG_UNLOCK();
	  
  return 0;
}

int
plot_window_title_get (lua_State *L)
{
  plot_window *p = plot_window::check(L, 1);

  AGG_LOCK();
  plot_window::plot_type& plt = p->get_plot();
  const char *title = plt.get_title();
  lua_pushstring (L, title);
  AGG_UNLOCK();
  
  return 1;
}

int
plot_window_units_set (lua_State *L)
{
  plot_window *p = plot_window::check(L, 1);
  bool request = (bool) lua_toboolean (L, 2);

  AGG_LOCK();
  
  plot_window::plot_type& plt = p->get_plot();
  bool current = plt.use_units();

  if (current != request)
    {
      plt.set_units(request);
      p->plot_update();
    }

  AGG_UNLOCK();
	  
  return 0;
}

int
plot_window_units_get (lua_State *L)
{
  plot_window *p = plot_window::check(L, 1);

  AGG_LOCK();
  plot_window::plot_type& plt = p->get_plot();
  lua_pushboolean (L, plt.use_units());
  AGG_UNLOCK();

  return 1;
}

int
plot_window_newindex (lua_State *L)
{
  return mlua_newindex_with_properties (L, plot_window_properties_set);
}

int
plot_window_update (lua_State *L)
{
  plot_window *p = plot_window::check(L, 1);
  AGG_PROTECT(p->plot_update());
  return 0;
}

int
plot_window_show (lua_State *L)
{
  plot_window *p = plot_window::check(L, 1);
  p->start_new_thread (L);
  return 1;
}

void
plot_window_register (lua_State *L)
{
  /* plot declaration */
  luaL_newmetatable (L, GS_METATABLE(GS_PLOT_WINDOW));
  lua_pushstring (L, "__superindex");
  lua_pushcfunction (L, canvas_window_index);
  lua_rawset (L, -3);
  luaL_register (L, NULL, plot_window_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plot_window_functions);
}
