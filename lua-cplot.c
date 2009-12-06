
#include <assert.h>
#include <pthread.h>

#include "lua.h"
#include "lauxlib.h"

#include "common.h"
#include "agg-cplot.h"
#include "lua-cplot-priv.h"
#include "xwin-show.h"

extern void cplot_register (lua_State *L);

struct lline {
  line *element;
  bool is_owner;
};

static const char * const cplot_mt_name = "GSL.pl.cplot";
static const char * const line_mt_name = "GSL.pl.draw";

static int lline_new     (lua_State *L);
static int lpoly_new     (lua_State *L);
static int lline_move_to (lua_State *L);
static int lline_line_to (lua_State *L);
static int lline_close   (lua_State *L);
static int lline_free    (lua_State *L);

static int lcplot_new     (lua_State *L);
static int lcplot_show    (lua_State *L);
static int lcplot_add     (lua_State *L);
static int lcplot_free    (lua_State *L);

struct lline* check_drawable (lua_State *L, int index);
struct lcplot*    check_lcplot    (lua_State *L, int index);

static const struct luaL_Reg cplot_functions[] = {
  {"line",      lline_new},
  {"poly",      lpoly_new},
  {"cplot",     lcplot_new},
  {NULL, NULL}
};

static const struct luaL_Reg lline_methods[] = {
  {"move_to",     lline_move_to},
  {"line_to",     lline_line_to},
  {"close",       lline_close},
  {"__gc",        lline_free},
  {NULL, NULL}
};

static const struct luaL_Reg cplot_methods[] = {
  {"show",        lcplot_show},
  {"add",         lcplot_add},
  {"__gc",        lcplot_free},
  {NULL, NULL}
};

int
lline_new (lua_State *L)
{
  const char *color_str = luaL_optstring (L, 1, "black");
  struct lline *d;

  d = (struct lline *) lua_newuserdata (L, sizeof (struct lline));

  if (lua_gettop (L) > 1)
    {
      double ll[2] = {10.0, 10.0};
      size_t j, n;

      if (! lua_istable (L, 2))
	return luaL_error (L, "second argument of line should be a list of numbers"
			   "to define dashed lines.");

      n = lua_objlen (L, 2);
      for (j = 1; j <= n && j <= 2; j++)
	{
	  lua_rawgeti (L, 2, j);
	  if (lua_isnumber (L, -1))
	    ll[j-1] = lua_tonumber (L, -1);
	  lua_pop (L, 1);
	}

      d->element = dashed_line_new (color_str, ll[0], ll[1]);
    }
  else
    {
      d->element = line_new (color_str);
    }

  d->is_owner = true;

  luaL_getmetatable (L, line_mt_name);
  lua_setmetatable (L, -2);

  printf ("created lline: %p\n", d);

  return 1;
}

int
lpoly_new (lua_State *L)
{
  const char *color_str = luaL_checkstring (L, 1);
  const char *outline_color_str = luaL_optstring (L, 2, NULL);
  struct lline *d = (struct lline *) lua_newuserdata (L, sizeof (struct lline));

  d->element = poly_new (color_str, outline_color_str);
  d->is_owner = true;

  luaL_getmetatable (L, line_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

struct lline *
check_drawable (lua_State *L, int index)
{
  return (struct lline*) luaL_checkudata (L, index, line_mt_name);
}

int
lline_free (lua_State *L)
{
  struct lline *d = check_drawable (L, 1);
  printf ("%p, line free... ", d);
  if (d->is_owner)
    {
      printf ("actually\n");
      line_free (d->element);
    }
  else
    {
      printf ("not really\n");
    }
  return 0;
}

int
lline_move_to (lua_State *L)
{
  struct lline *d = check_drawable (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  line_move_to (d->element, x, y);
  return 0;
}

int
lline_line_to (lua_State *L)
{
  struct lline *d = check_drawable (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  /*  printf ("%la %la\n", x, y); */
  line_line_to (d->element, x, y);
  return 0;
}

int
lline_close (lua_State *L)
{
  struct lline *d = check_drawable (L, 1);
  line_close (d->element);
  return 0;
}

struct lcplot *
check_lcplot (lua_State *L, int index)
{
  struct lcplot **ptr = luaL_checkudata (L, index, cplot_mt_name);
  return *ptr;
}

int
lcplot_new (lua_State *L)
{
  lua_Integer use_units = 1;
  struct lcplot **cpptr = lua_newuserdata (L, sizeof(void *));
  struct lcplot *cp;

  if (lua_isboolean (L, 1))
    use_units = lua_toboolean (L, 1);

  cp = emalloc (sizeof(struct lcplot));
  *cpptr = cp;

  cp->plot = cplot_new (use_units);
  pthread_mutex_init (cp->mutex, NULL);

  cp->lua_is_owner = 1;
  cp->is_shown = 0;
  cp->window = NULL;

  luaL_getmetatable (L, cplot_mt_name);
  lua_setmetatable (L, -2);

  printf ("created lcplot: %p\n", cp);

  return 1;
}

void
lcplot_destroy (struct lcplot *cp)
{
  printf ("lcplot destroy %p\n", cp);
  cplot_free (cp->plot);
  pthread_mutex_destroy (cp->mutex);
  free (cp);
}

int
lcplot_free (lua_State *L)
{
  struct lcplot *cp = check_lcplot (L, 1);

  printf ("%p, cplot free... ", cp);
  pthread_mutex_lock (cp->mutex);

  assert (cp->lua_is_owner);

  if (cp->is_shown)
    {
      printf ("thread taking ownership of lcplot %p\n", cp);
      cp->lua_is_owner = 0;
    }
  else
    {
      printf ("really\n");
      pthread_mutex_unlock (cp->mutex);
      lcplot_destroy (cp);
      return 0;
    }

  pthread_mutex_unlock (cp->mutex);
  return 0;
}

int
lcplot_add (lua_State *L)
{
  struct lcplot *cp = check_lcplot (L, 1);
  struct lline *d = check_drawable (L, 2);
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

  if (cp->window)
    update_callback (cp->window);

  pthread_mutex_unlock (cp->mutex);

  return 0;
}

int
lcplot_show (lua_State *L)
{
  struct lcplot *cp = check_lcplot (L, 1);
  pthread_t xwin_thread[1];
  pthread_attr_t attr[1];

  pthread_mutex_lock (cp->mutex);

  if (! cp->is_shown)
    {
      pthread_attr_init (attr);
      pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

      if (pthread_create(xwin_thread, attr, xwin_thread_function, (void*) cp))
	{
	  pthread_attr_destroy (attr);
	  return luaL_error(L, "error creating thread.");
	}
      
      cp->is_shown = 1;
      pthread_attr_destroy (attr);
    }

  pthread_mutex_unlock (cp->mutex);

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
  luaL_newmetatable (L, line_mt_name);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, lline_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, cplot_functions);
}
