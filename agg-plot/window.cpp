
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "window-cpp.h"
#include "lua-draw.h"
#include "lua-cpp-utils.h"
#include "gs-types.h"
#include "object-refs.h"
#include "object-index.h"
#include "colors.h"
#include "lua-plot-cpp.h"
#include "split-spec-parser.h"

__BEGIN_DECLS

static int window_new        (lua_State *L);
static int window_free       (lua_State *L);
static int window_split      (lua_State *L);
static int window_attach     (lua_State *L);

static const struct luaL_Reg window_functions[] = {
  {"window",        window_new},
  {NULL, NULL}
};

static const struct luaL_Reg window_methods[] = {
  {"attach",         window_attach        },
  {"split",          window_split         },
  {"update",         window_update        },
  {"__gc",           window_free       },
  {NULL, NULL}
};

__END_DECLS

static void remove_plot_ref (lua_State *L, int window_index, int plot_id);

void
window::draw_rec(split::node<ref> *n)
{
  split::node<ref>::list *ls;
  for (ls = n->tree(); ls != NULL; ls = ls->next())
    draw_rec(ls->content());

  ref *ref = n->content();
  matrix* m = n->get_matrix();
  if (ref && m)
    {
      if (ref->plot)
	{
	  agg::trans_affine mtx(*m);
	  this->scale(mtx);
	  ref->plot->draw(*m_canvas, mtx);
	}
    }
}

void
window::on_draw_unprotected()
{
  if (! m_canvas)
    return;

  m_canvas->clear();
  draw_rec(m_tree);
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

void
window::cleanup_tree_rec (lua_State *L, int window_index, split::node<ref>* n)
{
  split::node<ref>::list *ls;
  for (ls = n->tree(); ls != NULL; ls = ls->next())
    cleanup_tree_rec(L, window_index, ls->content());

  ref *ref = n->content();
  if (ref)
    {
      if (ref->plot)
	remove_plot_ref (L, window_index, ref->id);
    }
}

void
window::split(const char *spec)
{
  if (m_tree)
    delete m_tree;
  split::string_lexer lexbuf(spec);
  m_tree = split::parse<ref, split::string_lexer>(lexbuf);
  split::node<ref>::init(m_tree);
}

static const char *
next_int (const char *str, int& val)
{
  while (*str == ' ')
    str++;
  if (*str == '\0')
    return NULL;

  char *eptr;
  val = strtol (str, &eptr, 10);

  if (eptr == str)
    return NULL;

  while (*eptr == ' ')
    eptr++;
  if (*eptr == ',')
    eptr++;
  return eptr;
}

/* Returns the existing plot ref id, 0 if there isn't any.
   It does return -1 in case of error.*/
int window::attach(lua_plot *plot, const char *spec, int id)
{
  split::node<ref> *n = m_tree;
  const char *ptr;
  int k;

  for (ptr = next_int (spec, k); ptr; ptr = next_int (ptr, k))
    {
      split::node<ref>::list* list = n->tree();

      if (! list)
	return -1;

      for (int j = 1; j < k; j++)
	{
	  list = list->next();
	  if (! list)
	    return -1;
	}

      n = list->content();
    }

  ref* ex_ref = n->content();
  if (! ex_ref)
    return -1;
  int ex_id = ex_ref->id;

  ref new_ref(& plot->self(), id);
  n->content(new_ref);

  /* NB: here ex_ref content will be the same of new_ref */

  return (ex_id > 0 ? ex_id : 0);
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
window_split (lua_State *L)
{
  window *win = window::check (L, 1);
  const char *spec = luaL_checkstring (L, 2);

  win->lock();

  win->cleanup_refs(L, 1);
  win->split(spec);

  win->on_draw();
  win->update_window();

  win->unlock();
  return 0;
}

void 
remove_plot_ref (lua_State *L, int window_index, int plot_id)
{
  object_index_get (L, OBJECT_PLOT, plot_id);

  int plot_index = lua_gettop (L);
  if (gs_is_userdata (L, plot_index, GS_PLOT))
    object_ref_remove (L, window_index, plot_index);
}

int
window_attach (lua_State *L)
{
  window *win = window::check (L, 1);
  lua_plot *plot = lua_plot::check (L, 2);
  const char *spec = luaL_checkstring (L, 3);

  win->lock();

  int ex_plot_id = win->attach (plot, spec, plot->id);

  if (ex_plot_id >= 0)
    {
      plot->window_id = win->id;

      win->on_draw();
      win->update_window();

      win->unlock();

      object_ref_add (L, 1, 2);

      if (ex_plot_id > 0)
	remove_plot_ref (L, 1, ex_plot_id);
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

  win->lock();
  win->on_draw_unprotected();
  win->update_window();
  win->unlock();

  return 0;
}

int
window_update (lua_State *L)
{
  window *win = window::check (L, 1);

  win->lock();
  win->on_draw();
  win->update_window();
  win->unlock();

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
