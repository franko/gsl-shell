
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

void
window::draw_rec(split::node<plot_type*> *n)
{
  split::node<plot_type*>::list *ls;
  for (ls = n->tree(); ls != NULL; ls = ls->next())
    draw_rec(ls->content());

  plot_type **p = n->content();
  matrix* m = n->get_matrix();
  if (p && m)
    {
      if (*p)
	{
	  agg::trans_affine mtx(*m);
	  m_canvas->premultiply(mtx);
	  (*p)->draw(*m_canvas, mtx);
	}
    }
}

void
window::on_draw_unprotected()
{
  if (! m_canvas)
    return;

  if (m_tree)
    {
      m_canvas->clear();
      draw_rec(m_tree);
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

/*
static void
set_matrix(agg::trans_affine& m, double x, double y, double sx, double sy)
{
  m.tx = x;
  m.ty = y;
  m.sx = sx;
  m.sy = sy;
}

typedef pod_list<plot_matrix> pm_list;

pm_list * build_list(node *tree)
{
  pm_list *ls = NULL;

  node_list *childs = tree->get_tree();
  for ( ; childs != NULL; childs = childs->next())
    {
      pm_list *sub = build_list(childs->content());
      ls = pm_list::push_back(ls, sub);
    }
  
  matrix *m = tree->get_matrix();
  if (m)
    {
      plot_matrix pm(NULL, *m);
      ls = new pm_list(pm, ls);
    }

  return ls;
}
*/

void
window::split(const char *spec)
{
  split::string_lexer lexbuf(spec);
  m_tree = split::parse<plot_type*, split::string_lexer>(lexbuf, (plot_type*) 0);

  agg::trans_affine m;
  m_tree->transform(m);
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

bool
window::attach(lua_plot *plot, const char *spec)
{
  split::node<plot_type*> *n = m_tree;
  const char *ptr;
  int k;

  for (ptr = next_int (spec, k); ptr; ptr = next_int (ptr, k))
    {
      split::node<plot_type*>::list* list = n->tree();

      if (! list)
	break;

      for (int j = 1; j < k && list; j++)
	list = list->next();

      n = list->content();
    }

  n->content(& plot->self());
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
window_split (lua_State *L)
{
  window *win = window::check (L, 1);
  const char *spec = luaL_checkstring (L, 2);
  win->split(spec);
  return 0;
}

int
window_attach (lua_State *L)
{
  window *win = window::check (L, 1);
  lua_plot *plot = lua_plot::check (L, 2);
  const char *spec = luaL_checkstring (L, 3);

  win->lock();

  if (win->attach (plot, spec))
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
