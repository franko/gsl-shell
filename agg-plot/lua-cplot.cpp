
#include <pthread.h>

#include "lua.hpp"
#include "units_cplot.h"
#include "cplot.h"
#include "lua-cplot-priv.h"

#include "xwin-show.h"

extern int agg_main (int argc, char *argv[]);

extern "C" {

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

  static int cplot_new     (lua_State *L);
  static int cplot_show    (lua_State *L);
  static int cplot_add     (lua_State *L);
  static int cplot_free    (lua_State *L);

  struct ldrawable* check_drawable (lua_State *L, int index);
  struct lcplot*    check_cplot    (lua_State *L, int index);
}

static const struct luaL_Reg cplot_functions[] = {
  {"line",      drawable_new},
  {"cplot",     cplot_new},
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
  {"show",        cplot_show},
  {"add",         cplot_add},
  {"__gc",        cplot_free},
  {NULL, NULL}
};

static agg::rgba8
color_lookup (const char *color_str)
{
  const char *p = color_str;
  agg::rgba8 c;
  int val = 180;

  if (strncmp (p, "light", 5) == 0)
    {
      val = 255;
      p += 5;
    }
  else if (strncmp (p, "dark", 4) == 0)
    {
      val = 80;
      p += 4;
    }

  if (strcmp (p, "red") == 0)
    c = agg::rgba8(val, 0, 0);
  else if (strcmp (p, "green") == 0)
    c = agg::rgba8(0, val, 0);
  else if (strcmp (p, "blue") == 0)
    c = agg::rgba8(0, 0, val);
  else
    c = agg::rgba8(0, 0, 0);

  return c;
}

int
drawable_new (lua_State *L)
{
  struct ldrawable *d = (struct ldrawable *) lua_newuserdata (L, sizeof (struct ldrawable));
  const char *color_str = luaL_optstring (L, 1, "black");
  agg::rgba8 color = color_lookup (color_str);

  d->element  = new line(color);
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
check_cplot (lua_State *L, int index)
{
  return (struct lcplot *) luaL_checkudata (L, index, cplot_mt_name);
}

int
drawable_free (lua_State *L)
{
  struct ldrawable *d = check_drawable (L, 1);
  if (d->is_owner)
    delete d->element;
  return 0;
}

int
drawable_move_to (lua_State *L)
{
  struct ldrawable *d = check_drawable (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  agg::path_storage& p = d->element->path;
  p.move_to(x, y);
  return 0;
}

int
drawable_line_to (lua_State *L)
{
  struct ldrawable *d = check_drawable (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  agg::path_storage& p = d->element->path;
  p.line_to(x, y);
  return 0;
}

int
drawable_close (lua_State *L)
{
  struct ldrawable *d = check_drawable (L, 1);
  agg::path_storage& p = d->element->path;
  p.close_polygon();
  return 0;
}

int
cplot_new (lua_State *L)
{
  lua_Integer use_units = luaL_optinteger (L, 1, 0);
  struct lcplot *cp = (struct lcplot *) lua_newuserdata (L, sizeof (struct lcplot));

  if (use_units)
    {
      cp->plot = new units_cplot();
    }
  else
    {
      cp->plot = new cplot();
    }

  cp->lua_is_owner = 1;
  cp->is_shown = 0;
  cp->x_app = NULL;

  cp->mutex = new pthread_mutex_t;
  pthread_mutex_init (cp->mutex, NULL);

  luaL_getmetatable (L, cplot_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

void
lcplot_destroy (struct lcplot *cp)
{
  delete cp->plot;
  pthread_mutex_destroy (cp->mutex);
  free (cp->mutex);
}

int
cplot_free (lua_State *L)
{
  struct lcplot *cp = check_cplot (L, 1);
  if (cp->lua_is_owner && cp->is_shown)
    {
      cp->lua_is_owner = 0;
    }
  else
    {
      lcplot_destroy (cp);
    }
}

int
cplot_add (lua_State *L)
{
  struct lcplot *cp = check_cplot (L, 1);
  struct ldrawable *d = check_drawable (L, 2);
  cplot *p = cp->plot;

  pthread_mutex_lock (cp->mutex);

  if (d->is_owner)
    {
      p->add(d->element);
      d->is_owner = false;
    }
  else
    {
      line* ln_copy = new line(*d->element);
      p->add(ln_copy);
    }

  pthread_mutex_unlock (cp->mutex);

  if (cp->x_app)
    update_callback (cp->x_app);

  return 0;
}

int
cplot_show (lua_State *L)
{
  struct lcplot *cp = check_cplot (L, 1);
  cplot *p = cp->plot;
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

int
agg_main (int argc, char *argv[])
{
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
