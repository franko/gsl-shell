#ifndef FOXGUI_LUA_ENGINE_H
#define FOXGUI_LUA_ENGINE_H

extern "C" {
#include "lua.h"
}

#include "agg_array.h"

#include "gsl_shell_interp.h"
#include "pthreadpp.h"
#include "redirect.h"
#include "str.h"

class gsl_shell_thread : public gsl_shell {
public:
  typedef int (*lua_init_func_t)(lua_State*, void*);

  enum engine_status_e { starting, ready, busy, terminated };
  enum { eot_character = 0x04 };

  gsl_shell_thread();
  ~gsl_shell_thread();

  void input(const char* line);
  void start();
  void run();
  void stop();

  void window_close_notify(int window_id);

  void set_init_func(lua_init_func_t init_func, void* userdata)
  {
    m_init_func = init_func;
    m_init_userdata = userdata;
  }

  void user_init(lua_State* L)
  {
    if (m_init_func)
      (*m_init_func)(L, m_init_userdata);
  }

  void lock() { pthread_mutex_lock(&this->exec_mutex); }
  void unlock() { pthread_mutex_unlock(&this->exec_mutex); }

  int read(char* buffer, unsigned buffer_size);

  int eval_status() const { return m_eval_status; }

private:
  void treat_close_window_queue();

  pthread_t m_thread;
  engine_status_e m_status;
  stdout_redirect m_redirect;
  pthread::cond m_eval;
  str m_line_pending;
  int m_eval_status;
  bool m_exit_request;
  lua_init_func_t m_init_func;
  void* m_init_userdata;
  agg::pod_bvector<int> m_window_close_queue;
};

#endif
