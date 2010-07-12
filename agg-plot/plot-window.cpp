#include <pthread.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "platform/agg_platform_support.h"

#include "defs.h"
#include "gsl-shell.h"
#include "plot-window.h"
#include "agg-parse-trans.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "lua-draw.h"
#include "gs-types.h"
#include "colors.h"
#include "canvas.h"

extern void platform_support_prepare   ();
extern void platform_support_lock      (agg::platform_support *app);
extern void platform_support_unlock    (agg::platform_support *app);
extern bool platform_support_is_mapped (agg::platform_support *app);

__BEGIN_DECLS

static void * win_thread_function (void *_win);

static int plot_window_new         (lua_State *L);
static int plot_window_free        (lua_State *L);
static int plot_window_prepare     (lua_State *L);
static int plot_window_draw        (lua_State *L);
static int plot_window_render      (lua_State *L);

static const struct luaL_Reg plotwin_functions[] = {
  {"window",       plot_window_new},
  {NULL, NULL}
};

static const struct luaL_Reg plot_window_methods[] = {
  {"__gc",         plot_window_free},
  {"prepare",      plot_window_prepare},
  {"draw",         plot_window_draw},
  {"render",       plot_window_render},
  {NULL, NULL}
};

pthread_mutex_t window_mutex[1];

__END_DECLS

class plot_window : public agg::platform_support {
private:
  canvas *m_canvas;

public:
  enum win_status_e { not_ready, starting, running, error, closed };

  int id;
  enum win_status_e status;

  plot_window() :
    agg::platform_support(agg::pix_format_bgr24, true), 
    m_canvas(NULL), id(-1), status(not_ready) 
  { };

  virtual ~plot_window() 
  {
    if (m_canvas)
      delete m_canvas;
  };

  virtual void on_draw()
  {
    printf("on draw!\n");
    canvas can(rbuf_window(), width(), height(), agg::rgba(1.0, 1.0, 1.0));
    can.clear();
  };

  void canvas_prepare() 
  {
    if (m_canvas)
      delete m_canvas;

    m_canvas = new canvas(rbuf_window(), width(), height(), 
			  agg::rgba(1.0, 1.0, 1.0));
  };

  void start();

  bool draw(vertex_source *obj, agg::rgba8 *color)
  {
    if (! m_canvas)
      return false;

    m_canvas->draw(*obj, *color);
    return true;
  };
 
  static plot_window *check (lua_State *L, int index);
};

void
plot_window::start()
{
  WINDOW_LOCK();

  this->caption("GSL shell plot");
  if (this->init(480, 480, agg::window_resize))
    {
      this->status = plot_window::running;
      
      WINDOW_UNLOCK();
      this->run();

      WINDOW_LOCK();
      this->status = plot_window::closed;
      WINDOW_UNLOCK();

      GSL_SHELL_LOCK();
      gsl_shell_unref_plot (this->id);
      GSL_SHELL_UNLOCK();
    }
  
  WINDOW_UNLOCK();
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
  plot_window *win = new(L, GS_AGG_WINDOW) plot_window;

  WINDOW_LOCK();

  win->id = mlua_window_ref(L, 1);

  pthread_attr_t attr[1];
  pthread_t win_thread[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);
    
  if (pthread_create(win_thread, attr, win_thread_function, (void*) win))
    {
      mlua_window_unref(L, win->id);

      pthread_attr_destroy (attr);
      win->status = plot_window::error; 
      WINDOW_UNLOCK();

      luaL_error(L, "error creating thread");
    }

  pthread_attr_destroy (attr);
  win->status = plot_window::starting;
  WINDOW_UNLOCK();

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

  if (narg > 3)
    {
      curr = parse_spec_pipeline (L, 4, curr);
      lua_pop (L, 1);
    }

  if (! win->draw(curr, color))
    return luaL_error (L, "canvas not ready");

  return 0;
  
}

int
plot_window_prepare (lua_State *L)
{
  plot_window *win = plot_window::check (L, 1);
  win->canvas_prepare();
  return 0;
}

int
plot_window_render (lua_State *L)
{
  plot_window *win = plot_window::check (L, 1);
  win->update_window();
  return 0;
}

void
plot_window_register (lua_State *L)
{
  pthread_mutex_init (window_mutex, NULL);

  luaL_newmetatable (L, GS_METATABLE(GS_AGG_WINDOW));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, plot_window_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plotwin_functions);
}
