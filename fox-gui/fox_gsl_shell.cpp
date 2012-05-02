
#include "fox_gsl_shell.h"
#include "lua_plot_window.h"
#include "window_registry.h"

void fox_gsl_shell::init()
{
  gsl_shell_thread::init();
  fox_window_register(L);
}

void
fox_gsl_shell::before_eval()
{
  unsigned n = m_window_close_queue.size();
  for (unsigned k = 0; k < n; k++)
    {
      fprintf(stderr, "unregistering window id: %d\n", m_window_close_queue[k]);
      window_index_remove (L, m_window_close_queue[k]);
    }
  m_window_close_queue.clear();
}

void
fox_gsl_shell::window_close_notify(int window_id)
{
  fprintf(stderr, "got WINDOW CLOSE notification: %d\n", window_id);
  pthread::mutex& eval = eval_mutex();
  eval.lock();
  m_window_close_queue.add(window_id);
  eval.unlock();
}
