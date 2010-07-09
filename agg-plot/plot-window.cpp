#include <pthread.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "platform/agg_platform_support.h"

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
};

static const struct luaL_Reg plotwin_functions[] = {
  {"window",     plot_window_new},
  {NULL, NULL}
};


class the_application : public agg::platform_support
{
public:
  the_application(agg::pix_format_e format, bool flip_y) :
    agg::platform_support(format, flip_y)
  {
  };

  virtual ~the_application() { };

  virtual void on_draw()
  {
    printf("on draw!\n");
    canvas can(rbuf_window(), width(), height(), agg::rgba(1.0, 1.0, 1.0));
    can.clear();
  };
};

class plot_window {

private:
  enum win_status_e { not_ready, running, closed };

  the_application *m_app;
  int m_lua_id;
  enum win_status_e m_status;

public:

  plot_window() : 
    m_app(NULL), m_lua_id(-1), m_status(not_ready)
  {
  };

  virtual ~plot_window() 
  {
    if (m_app)
      delete m_app;
  };

  void set_id(int id) { m_lua_id = id; };
  int id() const { return m_lua_id; };

  void close() { m_status = closed; };
  void set_running() { m_status = plot_window::running; };

  void main_loop()
  {
    m_app = new the_application(agg::pix_format_bgr24, true);
    m_app->caption("GSL shell plot");
    if (m_app->init(780, 400, agg::window_resize))
      {
	m_app->run();
	m_status = plot_window::closed;
	//	gsl_shell_unref_plot (m_lua_id);
      }
  }
};

void *
win_thread_function (void *_win)
{
  printf("debugging threads!!\n");
  plot_window *win = (plot_window *) _win;
  win->main_loop();
  return NULL;
}

int
plot_window_new (lua_State *L)
{
  plot_window *win = new(L, GS_AGG_WINDOW) plot_window;

  int id = mlua_window_ref(L, 1);
  win->set_id(id);

  pthread_attr_t attr[1];
  pthread_t win_thread[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);
    
  if (pthread_create(win_thread, attr, win_thread_function, (void*) win))
    {
      pthread_attr_destroy (attr);
      mlua_window_unref(L, win->id());
      luaL_error(L, "error creating thread");
    }

  pthread_attr_destroy (attr);
  win->set_running();

  return 1;
}

void
plot_window_register (lua_State *L)
{
  platform_support_prepare();

  /* gsl module registration */
  luaL_register (L, NULL, plotwin_functions);
}
