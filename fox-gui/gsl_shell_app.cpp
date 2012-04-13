
#include "gsl_shell_app.h"
#include "gsl_shell_window.h"
#include "fx_plot_window.h"
#include "lua_plot_window.h"
#include "fatal.h"

FXDEFMAP(gsl_shell_app) gsl_shell_app_map[]={
  FXMAPFUNC(SEL_IO_READ, gsl_shell_app::ID_LUA_INTERRUPT, gsl_shell_app::on_lua_interrupt),
  FXMAPFUNC(SEL_IO_READ, gsl_shell_app::ID_LUA_REQUEST, gsl_shell_app::on_lua_request),
  FXMAPFUNC(SEL_CLOSE, 0, gsl_shell_app::on_window_close),
};

FXIMPLEMENT(gsl_shell_app,FXApp,gsl_shell_app_map,ARRAYNUMBER(gsl_shell_app_map))

static int
lua_fox_init(lua_State* L, void* _app)
{
  lua_pushlightuserdata(L, _app);
  lua_setfield(L, LUA_REGISTRYINDEX, "__fox_app");

  fox_window_register(L);
  return 0;
}

gsl_shell_app::gsl_shell_app() : FXApp("GSL Shell", "GSL Shell"),
  m_waiting_lua(false)
{
  m_event_loop  = new FXGUISignal(this, this, ID_LUA_INTERRUPT);
  m_lua_request = new FXGUISignal(this, this, ID_LUA_REQUEST);

  m_engine.set_init_func(lua_fox_init, this);
  m_engine.start();

  new gsl_shell_window(&m_engine, this, "GSL Shell Console", NULL, NULL, 600, 500);
}

gsl_shell_app::~gsl_shell_app()
{
  delete m_event_loop;
  delete m_lua_request;
}

long gsl_shell_app::on_lua_interrupt(FXObject*, FXSelector, void*)
{
  FXMutex& app_mutex = mutex();
  m_waiting_lua = true;
  m_lua_int.signal();
  m_lua_int.wait(app_mutex);
  m_waiting_lua = false;
  return 1;
}

bool gsl_shell_app::interrupt()
{
  FXMutex& app_mutex = mutex();
  app_mutex.lock();

  if (!m_waiting_lua)
    {
      m_event_loop->signal();
      m_lua_int.wait(app_mutex);
      return true;
    }

  return false;
}

long gsl_shell_app::on_lua_request(FXObject*, FXSelector, void*)
{
  FXMainWindow* win = m_lua_request_win;

  win->create();
  win->show(PLACEMENT_SCREEN);

  return 1;
}

void gsl_shell_app::window_create_request(FXMainWindow* win)
{
  m_lua_request_win = win;
  m_lua_request->signal();
}

long gsl_shell_app::on_window_close(FXObject* sender, FXSelector, void*)
{
  fx_plot_window* win = (fx_plot_window*) sender;
  m_engine.window_close_notify(win->lua_id);
  return 0;
}

void gsl_shell_app::resume(bool signal_end)
{
  FXMutex& app_mutex = mutex();
  if (signal_end)
    m_lua_int.signal();
  app_mutex.unlock();
}
