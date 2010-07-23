
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}


#include "defs.h"
#include "canvas-window.h"
#include "canvas-window-cpp.h"
#include "resource-manager.h"
#include "gsl-shell.h"
#include "agg-parse-trans.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "lua-draw.h"
#include "gs-types.h"
#include "colors.h"
#include "canvas.h"
#include "trans.h"

__BEGIN_DECLS

static int canvas_window_new           (lua_State *L);
static int canvas_window_free          (lua_State *L);
static int canvas_window_draw          (lua_State *L);
static int canvas_window_clear         (lua_State *L);
static int canvas_window_refresh       (lua_State *L);
static int canvas_window_set_box_trans (lua_State *L);

static void * canvas_thread_function        (void *_win);
static int    canvas_window_index_protected (lua_State *L);

static const struct luaL_Reg canvas_win_functions[] = {
  {"window",       canvas_window_new},
  {NULL, NULL}
};

static const struct luaL_Reg canvas_window_methods[] = {
  {"__gc",         canvas_window_free},
  {"__index",      canvas_window_index},
  {NULL, NULL}
};

static const struct luaL_Reg canvas_window_methods_protected[] = {
  {"draw",         canvas_window_draw},
  {"clear",        canvas_window_clear},
  {"refresh",      canvas_window_refresh},
  {"setview",      canvas_window_set_box_trans},
  {NULL, NULL}
};

__END_DECLS

void
canvas_window::on_draw()
{
  if (m_canvas) 
    m_canvas->clear();
}

void
canvas_window::on_resize(int sx, int sy)
{
  if (m_canvas)
    delete m_canvas;

  m_canvas = new canvas(rbuf_window(), sx, sy, m_bgcolor);
}

void
canvas_window::on_init()
{
  this->on_resize(width(), height());
}

void
canvas_window::user_transform(agg::trans_affine& m)
{
  m = m_user_trans;
}

void
canvas_window::start_new_thread (lua_State *L)
{
  this->id = mlua_window_ref(L, lua_gettop (L));

  pthread_attr_t attr[1];
  pthread_t win_thread[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

  this->lock();
    
  if (pthread_create(win_thread, attr, canvas_thread_function, (void*) this))
    {
      mlua_window_unref(L, this->id);

      pthread_attr_destroy (attr);
      this->status = canvas_window::error; 

      luaL_error(L, "error creating thread");
    }

  pthread_attr_destroy (attr);
  this->status = canvas_window::starting;
}

void *
canvas_thread_function (void *_win)
{
  platform_support_prepare();

  canvas_window *win = (canvas_window *) _win;

  win->caption("GSL shell plot");
  if (win->init(480, 480, agg::window_resize))
    {
      win->status = canvas_window::running;
      win->run();
      win->status = canvas_window::closed;

      GSL_SHELL_LOCK();
      gsl_shell_unref_plot (win->id);
      GSL_SHELL_UNLOCK();
    }

  win->unlock();

  return NULL;
}

canvas_window *
canvas_window::check (lua_State *L, int index)
{
  return (canvas_window *) gs_check_userdata (L, index, GS_CANVAS_WINDOW);
}

int
canvas_window_new (lua_State *L)
{
  agg::rgba8 *c8;

  if (lua_gettop (L) == 0)
    c8 = rgba8_push_default (L);
  else
    c8 = color_arg_lookup (L, 1);

  const double bs = (double) agg::rgba8::base_mask;
  agg::rgba color(c8->r / bs, c8->g / bs, c8->b / bs, c8->a / bs);

  canvas_window *win = new(L, GS_CANVAS_WINDOW) canvas_window(color);

  win->start_new_thread (L);

  return 1;
}

int
canvas_window_free (lua_State *L)
{
  canvas_window *win = canvas_window::check (L, 1);
  win->~canvas_window();
  return 0;
}

int
canvas_window_draw (lua_State *L)
{
  canvas_window *win = canvas_window::check (L, 1);
  int narg = lua_gettop (L);
  agg::rgba8 *color;

  if (narg <= 2)
    color = rgba8_push_default (L);
  else
    color = color_arg_lookup (L, 3);

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

  const agg::trans_affine& mtx = win->transform();
  curr->apply_transform(mtx, 1.0);

  bool success = win->draw(curr, color);

  lua_management::dispose(curr);

  if (! success)
    return luaL_error (L, "canvas not ready");

  return 0;
}

int
canvas_window_clear (lua_State *L)
{
  canvas_window *win = canvas_window::check (L, 1);
  win->on_draw();
  return 0;
}

int
canvas_window_refresh (lua_State *L)
{
  canvas_window *win = canvas_window::check (L, 1);
  win->update_window();
  return 0;
}

int
canvas_window_index_protected (lua_State *L)
{
  canvas_window *win = canvas_window::check(L, lua_upvalueindex(2));

  win->lock();

  if (win->status != canvas_window::running)
    {
      win->unlock();
      return luaL_error (L, "window is not active");
    }

  int narg = lua_gettop (L);

  lua_pushvalue (L, lua_upvalueindex(1));
  lua_insert (L, 1);

  if (lua_pcall (L, narg, LUA_MULTRET, 0) != 0)
    {
      win->unlock();
      return lua_error (L);
    }

  win->unlock();
  return lua_gettop (L);
}

int
canvas_window_index (lua_State *L)
{
  canvas_window *win = canvas_window::check(L, 1);
  const char *key = luaL_checkstring (L, 2);

  const struct luaL_Reg *r = mlua_find_method (canvas_window_methods, key);
  if (r)
    {
      lua_pushcfunction (L, r->func);
      return 1;
    }

  r = mlua_find_method (canvas_window_methods_protected, key);
  if (r)
    {
      lua_pushcfunction (L, r->func);
      lua_pushvalue (L, 1);
      lua_pushcclosure (L, canvas_window_index_protected, 2);
      return 1;
    }

  return 0;
}

int
canvas_window_set_box_trans (lua_State *L)
{
  canvas_window *win = canvas_window::check(L, 1);
  double x0 = luaL_checknumber (L, 2);
  double y0 = luaL_checknumber (L, 3);
  double x1 = luaL_checknumber (L, 4);
  double y1 = luaL_checknumber (L, 5);

  double sx = 1/(x1 - x0), sy = 1/(y1 - y0);
  double bx = -x0*sx, by = -y0*sy;

  agg::trans_affine mtx(sx, 0.0, 0.0, sy, bx, by);
  win->set_user_transform(mtx);

  return 0;
}

void
canvas_window_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_CANVAS_WINDOW));
  luaL_register (L, NULL, canvas_window_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, canvas_win_functions);
}
