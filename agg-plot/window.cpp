
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua-defs.h"
#include "window-cpp.h"
#include "lua-draw.h"
#include "lua-cpp-utils.h"
#include "gs-types.h"
#include "object-refs.h"
#include "object-index.h"
#include "colors.h"
#include "lua-plot-cpp.h"
#include "split-parser.h"
#include "platform_support_ext.h"

__BEGIN_DECLS

static int window_show            (lua_State *L);
static int window_free            (lua_State *L);
static int window_split           (lua_State *L);

static const struct luaL_Reg window_functions[] = {
  {"window",        window_new},
  {NULL, NULL}
};

static const struct luaL_Reg window_methods[] = {
  {"show",           window_show          },
  {"attach",         window_attach        },
  {"split",          window_split         },
  {"update",         window_update        },
  {"__gc",           window_free       },
  {NULL, NULL}
};

__END_DECLS

struct dispose_buffer {
  static void func (window::ref& ref)
  {
    if (ref.layer_buf)
      {
	delete [] ref.layer_buf;
	ref.layer_buf = 0;
      }
  }
};

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
window::ref::save_image (agg::rendering_buffer& win_buf, 
			 agg::rect_base<int>& r,
			 int img_bpp, bool flip_y)
{
  int w = r.x2 - r.x1, h = r.y2 - r.y1;
  int row_len = w * (img_bpp / 8);

  if (layer_buf == 0)
    {
      unsigned int bufsize = row_len * h;
      layer_buf = new(std::nothrow) unsigned char[bufsize];
    }

  if (layer_buf != 0)
    {
      layer_img.attach(layer_buf, w, h, flip_y ? -row_len : row_len);
      rendering_buffer_get_region (layer_img, win_buf, r, img_bpp / 8);
    }
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

void window::draw_slot_by_ref(window::ref& ref, bool update_req)
{
  if (! ref.plot)
    return;

  agg::rect_base<int> r;
  agg::trans_affine mtx(ref.matrix);
  this->scale(mtx);

  if (update_req)
    {
      r = rect_of_slot_matrix(mtx);
      m_canvas->clear_box(r);
    }

  AGG_LOCK();
  try 
    {
      ref.plot->draw(*m_canvas, mtx);
    } 
  catch (std::bad_alloc&) { }
  AGG_UNLOCK();

  if (update_req)
    {
      update_region(r);
    }
}

void
window::draw_slot(int slot_id, bool clean_req)
{
  ref *ref = window::ref_lookup (this->m_tree, slot_id);
  if (ref && m_canvas)
    {
      if (clean_req || ref->plot->need_redraw())
	{
	  draw_slot_by_ref(*ref, true);
	  dispose_buffer::func(*ref);
	}

      refresh_slot_by_ref(*ref);
    }
}

void
window::save_slot_image(int slot_id)
{
  ref *ref = window::ref_lookup (this->m_tree, slot_id);
  if (ref != 0)
    {
      agg::trans_affine mtx(ref->matrix);
      this->scale(mtx);

      agg::rect_base<int> r = rect_of_slot_matrix(mtx);
      ref->save_image(this->rbuf_window(), r, this->bpp(), this->flip_y());
    }
}

void
window::restore_slot_image(int slot_id)
{
  ref *ref = window::ref_lookup (this->m_tree, slot_id);
  if (ref != 0)
    {
      agg::trans_affine mtx(ref->matrix);
      this->scale(mtx);

      agg::rect_base<int> r = rect_of_slot_matrix(mtx);

      if (ref->layer_buf == 0)
	{
	  m_canvas->clear_box(r);
	  draw_slot_by_ref (*ref, false);
	  ref->save_image(this->rbuf_window(), r, this->bpp(), this->flip_y());
	  ref->dirty_rect.clear();
	}
      else
	{
	  agg::rendering_buffer& img = ref->layer_img;
	  agg::rendering_buffer& win = this->rbuf_window();
	  rendering_buffer_put_region (win, img, r, this->bpp() / 8);
	}
    }
}

void
window::refresh_slot_by_ref(ref& ref)
{
  agg::trans_affine mtx(ref.matrix);
  this->scale(mtx);

  AGG_LOCK();
  try {
    agg::rect_base<double> bb;
    if (ref.plot->draw_queue(*m_canvas, mtx, bb))
      {
	agg::rect_base<int> bbw(bb.x1 - 4, bb.y1 - 4, bb.x2 + 4, bb.y2 + 4);
	agg::rect_base<int> dbox;
	ref.dirty_rect.compose(dbox, bbw);
	update_region (dbox);
	ref.dirty_rect.set(bbw);
      }
  }
  catch (std::bad_alloc&) { }
  AGG_UNLOCK();
}

void
window::on_draw()
{
  if (! m_canvas)
    return;

  m_canvas->clear();
  draw_rec(m_tree);
}

void
window::on_resize(int sx, int sy)
{
  this->canvas_window::on_resize(sx, sy);
  if (m_tree)
    {
      tree::walk_rec<window::ref, direction_e, dispose_buffer>(m_tree);
    }
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
	object_refs_remove (L, table_window_plot, ref->slot_id, window_index);
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

/* Returns the slot_id or -1 in case of error. */
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

  r->plot = plot;

  return r->slot_id;
}

typedef void (window::*window_slot_method_type)(int slot_id);

int window_generic_oper (lua_State *L, window_slot_method_type method)
{
  window *win = object_check<window>(L, 1, GS_WINDOW);
  int slot_id = luaL_checkinteger (L, 2);

  win->lock();
  if (win->status == canvas_window::running)
    {
      (win->*method)(slot_id);
    }
  win->unlock();

  return 0;
}

template <class param_type>
int window_generic_oper_ext (lua_State *L, 
			     void (window::*method)(int, param_type),
			     param_type param)
{
  window *win = object_check<window>(L, 1, GS_WINDOW);
  int slot_id = luaL_checkinteger (L, 2);

  win->lock();
  if (win->status == canvas_window::running)
    {
      (win->*method)(slot_id, param);
    }
  win->unlock();

  return 0;
}

int
window_new (lua_State *L)
{
  window *win = push_new_object<window>(L, GS_WINDOW, colors::white);
  const char *spec = lua_tostring (L, 1);

  win->start_new_thread (L);

  if (spec)
    {
      win->split(spec);
    }

  return 1;
}

int
window_show (lua_State *L)
{
  window *win = object_check<window>(L, 1, GS_WINDOW);
  win->start_new_thread (L);
  return 0;
}

int
window_free (lua_State *L)
{
  return object_free<window>(L, 1, GS_WINDOW);
}

int
window_split (lua_State *L)
{
  window *win = object_check<window>(L, 1, GS_WINDOW);
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
  window *win = object_check<window>(L, 1, GS_WINDOW);
  lua_plot *plot = object_check<lua_plot>(L, 2, GS_PLOT);
  const char *spec = luaL_checkstring (L, 3);

  win->lock();

  int slot_id = win->attach (plot, spec);

  if (slot_id >= 0)
    {
      win->draw_slot(slot_id, true);
      win->unlock();
      object_refs_add (L, table_window_plot, slot_id, 1, 2);
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
  return window_generic_oper_ext (L, &window::draw_slot, true);
}

int
window_slot_refresh (lua_State *L)
{
  return window_generic_oper_ext (L, &window::draw_slot, false);
}

int
window_update (lua_State *L)
{
  window *win = object_check<window>(L, 1, GS_WINDOW);

  win->lock();
  win->on_draw();
  win->update_window();
  win->unlock();

  return 0;
}

int
window_save_slot_image (lua_State *L)
{
  return window_generic_oper (L, &window::save_slot_image);
}

int
window_restore_slot_image (lua_State *L)
{
  return window_generic_oper (L, &window::restore_slot_image);
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
