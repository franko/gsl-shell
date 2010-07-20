
#include <pthread.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "agg_color_rgba.h"

#include "lua-plot.h"
#include "lua-draw.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "gs-types.h"
#include "vertex-source.h"
#include "trans.h"
#include "colors.h"
#include "xwin-show.h"
#include "agg-parse-trans.h"

__BEGIN_DECLS

static int agg_plot_new        (lua_State *L);
static int agg_plot_show       (lua_State *L);
static int agg_plot_add        (lua_State *L);
static int agg_plot_update     (lua_State *L);
static int agg_plot_add_line   (lua_State *L);
static int agg_plot_index      (lua_State *L);
static int agg_plot_newindex   (lua_State *L);
static int agg_plot_free       (lua_State *L);
static int agg_plot_title_set  (lua_State *L);
static int agg_plot_title_get  (lua_State *L);
static int agg_plot_units_set  (lua_State *L);
static int agg_plot_units_get  (lua_State *L);

static const struct luaL_Reg plot_functions[] = {
  {"plot",     agg_plot_new},
  {NULL, NULL}
};

static const struct luaL_Reg agg_plot_methods[] = {
  {"show",        agg_plot_show       },
  {"add",         agg_plot_add        },
  {"addline",     agg_plot_add_line   },
  {"update",      agg_plot_update     },
  {"__index",     agg_plot_index      },
  {"__newindex",  agg_plot_newindex   },
  {"__gc",        agg_plot_free       },
  {NULL, NULL}
};

static const struct luaL_Reg agg_plot_properties_get[] = {
  {"title",        agg_plot_title_get  },
  {"units",        agg_plot_units_get  },
  {NULL, NULL}
};

static const struct luaL_Reg agg_plot_properties_set[] = {
  {"title",        agg_plot_title_set  },
  {"units",        agg_plot_units_set  },
  {NULL, NULL}
};

__END_DECLS

void agg_plot::wait_update()
{
  if (this->window)
    update_callback (this->window);
};

agg_plot* agg_plot::arg_check(lua_State *L, int index)
{
  return (agg_plot *) gs_check_userdata (L, index, GS_DRAW_PLOT);
}

int
agg_plot_new (lua_State *L)
{
  agg_plot *p = new(L, GS_DRAW_PLOT) agg_plot();

  lua_newtable (L);
  lua_setfenv (L, -2);

  if (lua_isstring (L, 1))
    {
      const char *title = lua_tostring (L, 1);
      if (title)
	p->set_title(title);
    }

  return 1;
}

int
agg_plot_free (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  p->~agg_plot();
  return 0;
}

int
agg_plot_index (lua_State *L)
{
  return mlua_index_with_properties (L, agg_plot_properties_get, false);
}

int
agg_plot_title_set (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  const char *title = lua_tostring (L, 2);

  if (title == NULL)
    return gs_type_error (L, 2, "string");
	  
  AGG_LOCK();

  p->set_title(title);
  p->wait_update();

  AGG_UNLOCK();
	  
  return 0;
}

int
agg_plot_title_get (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  const char *title = p->get_title();
  lua_pushstring (L, title);
  return 1;
}

int
agg_plot_units_set (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  bool request = (bool) lua_toboolean (L, 2);
  bool current = p->use_units();

  if (current != request)
    {
      AGG_LOCK();
      p->set_units(request);
      p->wait_update();
      AGG_UNLOCK();
    }
	  
  return 0;
}

int
agg_plot_units_get (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  lua_pushboolean (L, p->use_units());
  return 1;
}

int
agg_plot_newindex (lua_State *L)
{
  return mlua_newindex_with_properties (L, agg_plot_properties_set);
}

static int
agg_plot_add_gener (lua_State *L, bool as_line)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
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
  p->add(curr, color, as_line);
  p->wait_update();
  AGG_UNLOCK();

  return 0;
}
 
int
agg_plot_add (lua_State *L)
{
  return agg_plot_add_gener (L, false);
}
 
int
agg_plot_add_line (lua_State *L)
{
  return agg_plot_add_gener (L, true);
}
 
int
agg_plot_update (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  AGG_PROTECT(p->wait_update());
  return 0;
}

int
agg_plot_show (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  pthread_t xwin_thread[1];
  pthread_attr_t attr[1];

  AGG_LOCK();

  if (! p->is_shown)
    {
      p->id = mlua_window_ref(L, 1);

      pthread_attr_init (attr);
      pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

      if (pthread_create(xwin_thread, attr, xwin_thread_function, (void*) p))
	{
	  pthread_attr_destroy (attr);
	  mlua_window_unref(L, p->id);
	  p->id = -1;
	  AGG_UNLOCK();
	  return luaL_error(L, "error creating thread.");
	}
      
      p->is_shown = 1;
      pthread_attr_destroy (attr);
    }

  AGG_UNLOCK();

  return 0;
}

void
plot_register (lua_State *L)
{
  /* plot declaration */
  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_PLOT));
  luaL_register (L, NULL, agg_plot_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plot_functions);
}
