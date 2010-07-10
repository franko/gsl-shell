#include <pthread.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "platform/agg_platform_support.h"

#include "gsl-shell.h"
#include "plot-window.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "gs-types.h"
#include "canvas.h"

extern void platform_support_prepare   ();
extern void platform_support_lock      (agg::platform_support *app);
extern void platform_support_unlock    (agg::platform_support *app);
extern bool platform_support_is_mapped (agg::platform_support *app);

extern "C" {
  static void * win_thread_function (void *_win);

  static int plot_window_new        (lua_State *L);
  static int plot_window_free       (lua_State *L);
};

static const struct luaL_Reg plotwin_functions[] = {
  {"window",     plot_window_new},
  {NULL, NULL}
};

static const struct luaL_Reg agg_window_methods[] = {
  {"__gc",        plot_window_free},
  {NULL, NULL}
};

pthread_mutex_t window_mutex[1];

class window_app : public agg::platform_support
{
public:
  window_app(agg::pix_format_e format, bool flip_y) :
    agg::platform_support(format, flip_y)
  {
  };

  virtual ~window_app() { };

  virtual void on_draw()
  {
    printf("on draw!\n");
    canvas can(rbuf_window(), width(), height(), agg::rgba(1.0, 1.0, 1.0));
    can.clear();
  };
};

class plot_window {
private:
  window_app *m_app;

public:
  enum win_status_e { not_ready, starting, running, error, closed };

  int id;
  enum win_status_e status;

  plot_window() : 
    m_app(NULL), id(-1), status(not_ready)
  {
  };

  virtual ~plot_window() 
  {
    if (m_app)
      delete m_app;
  };

  void start()
  {
    WINDOW_LOCK();

    m_app = new window_app(agg::pix_format_bgr24, true);
    m_app->caption("GSL shell plot");
    if (m_app->init(780, 400, agg::window_resize))
      {
	this->status = plot_window::running;

	WINDOW_UNLOCK();
	m_app->run();

	WINDOW_LOCK();
	this->status = plot_window::closed;
	WINDOW_UNLOCK();

	GSL_SHELL_LOCK();
	gsl_shell_unref_plot (this->id);
	GSL_SHELL_UNLOCK();
      }

    WINDOW_UNLOCK();
  }
 
  static plot_window *check (lua_State *L, int index);
};

void *
win_thread_function (void *_win)
{
  printf("debugging threads!!\n");

  platform_support_prepare();

  plot_window *win = (plot_window *) _win;
  win->start();
  return NULL;
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

plot_window *
plot_window::check (lua_State *L, int index)
{
  return (plot_window *) gs_check_userdata (L, index, GS_AGG_WINDOW);
}

void
plot_window_register (lua_State *L)
{
  pthread_mutex_init (window_mutex, NULL);

  luaL_newmetatable (L, GS_METATABLE(GS_AGG_WINDOW));
  luaL_register (L, NULL, agg_window_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plotwin_functions);
}
