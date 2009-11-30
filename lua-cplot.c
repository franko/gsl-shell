
#include <pthread.h>

#include "lua.h"
#include "lauxlib.h"

#include "common.h"
#include "cplot-cintfc.h"
#include "lua-cplot-priv.h"
#include "xwin-show.h"

extern void cplot_register (lua_State *L);

struct ldrawable {
  line *element;
  bool is_owner;
};

static const char * const cplot_mt_name = "GSL.pl.cplot";
static const char * const drawable_mt_name = "GSL.pl.draw";

static int drawable_new     (lua_State *L);
static int drawable_move_to (lua_State *L);
static int drawable_line_to (lua_State *L);
static int drawable_close   (lua_State *L);
static int drawable_free    (lua_State *L);

static int lcplot_new     (lua_State *L);
static int lcplot_show    (lua_State *L);
static int lcplot_add     (lua_State *L);
static int lcplot_free    (lua_State *L);

struct ldrawable* check_drawable (lua_State *L, int index);
struct lcplot*    check_lcplot    (lua_State *L, int index);

static const struct luaL_Reg cplot_functions[] = {
  {"line",      drawable_new},
  {"cplot",     lcplot_new},
  {NULL, NULL}
};

static const struct luaL_Reg drawable_methods[] = {
  {"move_to",     drawable_move_to},
  {"line_to",     drawable_line_to},
  {"close",       drawable_close},
  {"__gc",        drawable_free},
  {NULL, NULL}
};

static const struct luaL_Reg cplot_methods[] = {
  {"show",        lcplot_show},
  {"add",         lcplot_add},
  {"__gc",        lcplot_free},
  {NULL, NULL}
};

int
drawable_new (lua_State *L)
{
  struct ldrawable *d = (struct ldrawable *) lua_newuserdata (L, sizeof (struct ldrawable));
  const char *color_str = luaL_optstring (L, 1, "black");

  d->element = line_new (color_str);
  d->is_owner = true;

  luaL_getmetatable (L, drawable_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

struct ldrawable *
check_drawable (lua_State *L, int index)
{
  return (struct ldrawable*) luaL_checkudata (L, index, drawable_mt_name);
}

struct lcplot *
check_lcplot (lua_State *L, int index)
{
  return (struct lcplot *) luaL_checkudata (L, index, cplot_mt_name);
}

int
drawable_free (lua_State *L)
{
  struct ldrawable *d = check_drawable (L, 1);
  if (d->is_owner)
    line_free (d->element);
  return 0;
}

int
drawable_move_to (lua_State *L)
{
  struct ldrawable *d = check_drawable (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  line_move_to (d->element, x, y);
  return 0;
}

int
drawable_line_to (lua_State *L)
{
  struct ldrawable *d = check_drawable (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  line_line_to (d->element, x, y);
  return 0;
}

int
drawable_close (lua_State *L)
{
  struct ldrawable *d = check_drawable (L, 1);
  line_close (d->element);
  return 0;
}

int
lcplot_new (lua_State *L)
{
  lua_Integer use_units = luaL_optinteger (L, 1, 0);
  struct lcplot *cp = (struct lcplot *) lua_newuserdata (L, sizeof (struct lcplot));

  cp->plot = cplot_new (use_units);

  cp->lua_is_owner = 1;
  cp->is_shown = 0;
  cp->x_app = NULL;

  cp->mutex = emalloc (sizeof(pthread_mutex_t));
  pthread_mutex_init (cp->mutex, NULL);

  luaL_getmetatable (L, cplot_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

void
lcplot_destroy (struct lcplot *cp)
{
  cplot_free (cp->plot);
  pthread_mutex_destroy (cp->mutex);
  free (cp->mutex);
}

int
lcplot_free (lua_State *L)
{
  struct lcplot *cp = check_lcplot (L, 1);
  if (cp->lua_is_owner && cp->is_shown)
    {
      cp->lua_is_owner = 0;
    }
  else
    {
      lcplot_destroy (cp);
    }
  return 0;
}

int
lcplot_add (lua_State *L)
{
  struct lcplot *cp = check_lcplot (L, 1);
  struct ldrawable *d = check_drawable (L, 2);
  cplot *p = cp->plot;

  pthread_mutex_lock (cp->mutex);

  if (d->is_owner)
    {
      cplot_add (p, d->element);
      d->is_owner = false;
    }
  else
    {
      line *ln_copy = line_copy (d->element);
      cplot_add (p, ln_copy);
    }

  pthread_mutex_unlock (cp->mutex);

  if (cp->x_app)
    update_callback (cp->x_app);

  return 0;
}

int
lcplot_show (lua_State *L)
{
  struct lcplot *cp = check_lcplot (L, 1);
  pthread_t xwin_thread[1];
  pthread_attr_t attr[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

  if (pthread_create(xwin_thread, attr, xwin_thread_function, (void*) cp))
    {
      pthread_attr_destroy (attr);
      return luaL_error(L, "error creating thread.");
    }

  pthread_attr_destroy (attr);
  return 0;
}

void
cplot_register (lua_State *L)
{
  /* cplot declaration */
  luaL_newmetatable (L, cplot_mt_name);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, cplot_methods);
  lua_pop (L, 1);

  /* drawable declaration */
  luaL_newmetatable (L, drawable_mt_name);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, drawable_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, cplot_functions);
}
