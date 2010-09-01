
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua-defs.h"
#include "window-cpp.h"
#include "lua-draw.h"
#include "lua-cpp-utils.h"
#include "gs-types.h"
#include "win-plot-refs.h"
#include "object-index.h"
#include "colors.h"
#include "lua-plot-cpp.h"
#include "split-parser.h"

__BEGIN_DECLS

static int window_free       (lua_State *L);
static int window_split      (lua_State *L);

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

agg::rect_base<int> rect_of_slot_matrix (const agg::trans_affine& mtx)
{
  return agg::rect_base<int>(mtx.tx, mtx.ty, mtx.sx + mtx.tx, mtx.sy + mtx.ty);
}

void window::ref::compose(bmatrix& a, const bmatrix& b)
{
  trans_affine_compose (a, b);
};

int window::ref::calculate(window::ref::node* t, const bmatrix& m, int id)
{
  ref *r = t->content();
  if (r)
    {
      r->slot_id = id++;
      r->matrix = m;
    }

  int nb = list::length(t->tree());

  if (nb > 0)
    {
      double frac = 1 / (double) nb;

      direction_e dir;
      ref::node::list *ls = t->tree(dir);
      if (ls)
	{
	  bmatrix lm;

	  double* p = (dir == along_x ? &lm.tx : &lm.ty);
	  double* s = (dir == along_x ? &lm.sx : &lm.sy);

	  *s = frac;

	  for ( ; ls; ls = ls->next(), *p += frac)
	    {
	      bmatrix sm(lm);
	      window::ref::compose(sm, m);
	      id = window::ref::calculate (ls->content(), sm, id);
	    }
	}
    }

  return id;
}

void
window::draw_rec(ref::node *n)
{
  ref::node::list *ls;
  for (ls = n->tree(); ls != NULL; ls = ls->next())
    draw_rec(ls->content());

  ref *ref = n->content();
  if (ref)
    {
      draw_slot_by_ref (*ref, false);
    }
}

window::ref* window::ref_lookup (ref::node *p, int slot_id)
{
  ref::node::list *t = p->tree();
  for (/* */; t; t = t->next())
    {
      ref *ref = window::ref_lookup(t->content(), slot_id);
      if (ref)
	return ref;
    }

  ref *ref = p->content();
  if (ref)
    {
      if (ref->slot_id == slot_id)
	return ref;
    }

  return NULL;
}

void window::draw_slot_by_ref(window::ref& ref, bool dirty)
{
  if (! ref.plot)
    return;

  agg::trans_affine mtx(ref.matrix);
  this->scale(mtx);

  if (dirty)
    {
      agg::rect_base<int> r = rect_of_slot_matrix(mtx);
      m_canvas->clear_box(r);
      AGG_LOCK();
      try 
	{
	  ref.plot->draw(*m_canvas, mtx);
	} 
      catch (std::bad_alloc&) { }
      AGG_UNLOCK();
      platform_support_update_region (this, r);
    }
  else
    {
      ref.plot->draw(*m_canvas, mtx);
    }
}

void
window::draw_slot(int slot_id, bool update_req)
{
  ref *ref = window::ref_lookup (this->m_tree, slot_id);
  if (ref && m_canvas)
    {
      draw_slot_by_ref(*ref, update_req);
    }
}

void
window::on_draw()
{
  if (! m_canvas)
    return;

  m_canvas->clear();
  draw_rec(m_tree);
}

window *
window::check (lua_State *L, int index)
{
  return (window *) gs_check_userdata (L, index, GS_WINDOW);
}

void
window::cleanup_tree_rec (lua_State *L, int window_index, ref::node* n)
{
  for (ref::node::list *ls = n->tree(); ls != NULL; ls = ls->next())
    cleanup_tree_rec(L, window_index, ls->content());

  ref *ref = n->content();
  if (ref)
    {
      if (ref->plot)
	window_plot_ref_remove (L, ref->slot_id, window_index);
    }
}

void
window::split(const char *spec)
{
  if (m_tree)
    delete m_tree;

  ::split<ref>::lexer lexbuf(spec);
  m_tree = ::split<ref>::parse(lexbuf);

  bmatrix m0;
  ref::calculate(m_tree, m0, 0);
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
int window::attach(lua_plot *plot, const char *spec)
{
  ref::node *n = m_tree;
  const char *ptr;
  int k;

  for (ptr = next_int (spec, k); ptr; ptr = next_int (ptr, k))
    {
      ref::node::list* list = n->tree();

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

  ref* r = n->content();
  if (! r)
    return -1;

  r->plot = & plot->self();

  return r->slot_id;
}

int
window_new (lua_State *L)
{
  window *win = push_new_object<window>(L, GS_WINDOW, colors::white);
  win->start_new_thread (L);
  return 1;
}

int
window_free (lua_State *L)
{
  return object_free<window>(L, 1, GS_WINDOW);
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

int
window_attach (lua_State *L)
{
  window *win = window::check (L, 1);
  lua_plot *plot = lua_plot::check (L, 2);
  const char *spec = luaL_checkstring (L, 3);

  win->lock();

  int slot_id = win->attach (plot, spec);

  if (slot_id >= 0)
    {
      win->draw_slot(slot_id, true);
      win->unlock();
      window_plot_ref_add (L, slot_id, 1, 2);
    }
  else
    {
      win->unlock();
      luaL_error (L, "invalid slot");
    }

  return 0;
}

int
window_slot_update (lua_State *L)
{
  window *win = window::check (L, 1);
  int slot_id = luaL_checkinteger (L, 2);

  win->lock();
  if (win->status == canvas_window::running)
    {
      win->draw_slot(slot_id, true);
    }
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
