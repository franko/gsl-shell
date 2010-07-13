#include <pthread.h>
#include <unistd.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "platform/agg_platform_support.h"

#include "defs.h"
#include "resource-manager.h"
#include "gsl-shell.h"
#include "plot-window.h"
#include "agg-parse-trans.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "lua-draw.h"
#include "gs-types.h"
#include "colors.h"
#include "canvas.h"
#include "trans.h"

extern void platform_support_prepare   ();
extern void platform_support_lock      (agg::platform_support *app);
extern void platform_support_unlock    (agg::platform_support *app);
extern bool platform_support_is_mapped (agg::platform_support *app);


__BEGIN_DECLS

static void * win_thread_function (void *_win);

static int plot_window_new           (lua_State *L);
static int plot_window_free          (lua_State *L);
static int plot_window_index         (lua_State *L);
static int plot_window_draw          (lua_State *L);
static int plot_window_clear         (lua_State *L);
static int plot_window_update        (lua_State *L);
static int plot_window_size          (lua_State *L);
static int plot_window_set_transform (lua_State *L);

static const struct luaL_Reg plotwin_functions[] = {
  {"window",       plot_window_new},
  {NULL, NULL}
};

static const struct luaL_Reg plot_window_methods[] = {
  {"__gc",         plot_window_free},
  {"__index",      plot_window_index},
  {NULL, NULL}
};

static const struct luaL_Reg plot_window_methods_protected[] = {
  {"draw",         plot_window_draw},
  {"clear",        plot_window_clear},
  {"update",       plot_window_update},
  {"size",         plot_window_size},
  {"transform",    plot_window_set_transform},
  {NULL, NULL}
};

pthread_mutex_t window_mutex[1];

__END_DECLS

class plot_window : public agg::platform_support {
private:
  canvas *m_canvas;
  agg::rgba m_bgcolor;

public:
  agg::trans_affine m_trans;

  enum win_status_e { not_ready, starting, running, error, closed };

  int id;
  enum win_status_e status;

  plot_window(agg::rgba& bgcol) :
    agg::platform_support(agg::pix_format_bgr24, true), 
    m_canvas(NULL), m_bgcolor(bgcol), m_trans(), id(-1), status(not_ready)
  { };

  virtual ~plot_window() 
  {
    if (m_canvas)
      delete m_canvas;
  };

  virtual void on_draw();
  virtual void on_init();
  virtual void on_resize(int sx, int sy);

  void start();
  void clear() { if (m_canvas) m_canvas->clear(); };

  bool draw(vertex_source *obj, agg::rgba8 *color)
  {
    if (! m_canvas)
      return false;

    m_canvas->draw(*obj, *color);
    return true;
  };

  void set_transform(double sx, double sy, double x0, double y0)
  {
    m_trans = agg::trans_affine(sx, 0.0, 0.0, sy, x0, y0);
  };
 
  static plot_window *check (lua_State *L, int index);
};

void
plot_window::on_init()
{
  if (m_canvas)
    delete m_canvas;

  m_canvas = new canvas(rbuf_window(), width(), height(), m_bgcolor);
}

void
plot_window::on_draw()
{
  printf("on draw!\n");
  if (! m_canvas)
    return;

  m_canvas->clear();
};

void
plot_window::on_resize(int sx, int sy)
{
  if (m_canvas)
    delete m_canvas;

  m_canvas = new canvas(rbuf_window(), sx, sy, m_bgcolor);
}

void
plot_window::start()
{
  //  sleep (15);

  this->caption("GSL shell plot");
  if (this->init(480, 480, agg::window_resize))
    {
      this->status = plot_window::running;
      
      this->run();

      this->status = plot_window::closed;

      GSL_SHELL_LOCK();
      gsl_shell_unref_plot (this->id);
      GSL_SHELL_UNLOCK();
    }
}

void *
win_thread_function (void *_win)
{
  printf("debugging threads!!\n");

  platform_support_prepare();

  plot_window *win = (plot_window *) _win;
  win->start();
  return NULL;
}

plot_window *
plot_window::check (lua_State *L, int index)
{
  return (plot_window *) gs_check_userdata (L, index, GS_AGG_WINDOW);
}

int
plot_window_new (lua_State *L)
{
  agg::rgba8 *c8;

  if (lua_gettop (L) == 0)
    c8 = rgba8_push_default (L);
  else
    c8 = color_arg_lookup (L, 1);

  const double bs = (double) agg::rgba8::base_mask;
  agg::rgba color(c8->r / bs, c8->g / bs, c8->b / bs, c8->a / bs);

  plot_window *win = new(L, GS_AGG_WINDOW) plot_window(color);

  win->id = mlua_window_ref(L, 1);

  pthread_attr_t attr[1];
  pthread_t win_thread[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

  platform_support_lock (win);
    
  if (pthread_create(win_thread, attr, win_thread_function, (void*) win))
    {
      mlua_window_unref(L, win->id);

      pthread_attr_destroy (attr);
      win->status = plot_window::error; 

      luaL_error(L, "error creating thread");
    }

  pthread_attr_destroy (attr);
  win->status = plot_window::starting;

  return 1;
}

int
plot_window_free (lua_State *L)
{
  plot_window *win = plot_window::check (L, 1);
  printf("freying plot window\n");
  win->~plot_window();
  return 0;
}

int
plot_window_draw (lua_State *L)
{
  plot_window *win = plot_window::check (L, 1);
  int narg = lua_gettop (L);
  agg::rgba8 *color;

  if (narg <= 2)
    color = rgba8_push_default (L);
  else
    color = color_arg_lookup (L, 3);

  vertex_source *curr = check_agg_obj (L, 2);

  trans::affine *to = new trans::affine(curr);
  to->set_matrix(win->m_trans);
  curr = to;

  if (narg > 3)
    {
      curr = parse_spec_pipeline (L, 4, curr);
      lua_pop (L, 1);
    }

  bool success = win->draw(curr, color);

  lua_management::dispose(curr);

  if (! success)
    return luaL_error (L, "canvas not ready");

  return 0;
}

int
plot_window_clear (lua_State *L)
{
  plot_window *win = plot_window::check (L, 1);
  win->clear();
  return 0;
}

int
plot_window_update (lua_State *L)
{
  plot_window *win = plot_window::check (L, 1);
  win->update_window();
  return 0;
}

static int
plot_window_index_protected (lua_State *L)
{
  plot_window *win = plot_window::check(L, lua_upvalueindex(2));

  int narg = lua_gettop (L);

  lua_pushvalue (L, lua_upvalueindex(1));
  lua_insert (L, 1);

  platform_support_lock (win);
  if (lua_pcall (L, narg, LUA_MULTRET, 0) != 0)
    {
      platform_support_unlock (win);
      return lua_error (L);
    }

  platform_support_unlock (win);
  return lua_gettop (L);
}

int
plot_window_index (lua_State *L)
{
  const char *key = luaL_checkstring (L, 2);

  const struct luaL_Reg *r = mlua_find_method (plot_window_methods, key);
  if (r)
    {
      lua_pushcfunction (L, r->func);
      return 1;
    }

  r = mlua_find_method (plot_window_methods_protected, key);
  if (r)
    {
      lua_pushcfunction (L, r->func);
      lua_pushvalue (L, 1);
      lua_pushcclosure (L, plot_window_index_protected, 2);
      return 1;
    }

  return 0;
}

int
plot_window_size (lua_State *L)
{
  plot_window *win = plot_window::check(L, 1);
  lua_pushinteger (L, win->width());
  lua_pushinteger (L, win->height());
  return 2;
}

int
plot_window_set_transform (lua_State *L)
{
  plot_window *win = plot_window::check(L, 1);
  double sx = luaL_checknumber (L, 2);
  double sy = luaL_checknumber (L, 3);
  double x0 = luaL_optnumber (L, 4, 0.0);
  double y0 = luaL_optnumber (L, 5, 0.0);
  win->set_transform(sx, sy, x0, y0);
  return 0;
}

void
plot_window_register (lua_State *L)
{
  pthread_mutex_init (window_mutex, NULL);

  luaL_newmetatable (L, GS_METATABLE(GS_AGG_WINDOW));
  luaL_register (L, NULL, plot_window_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plotwin_functions);
}
