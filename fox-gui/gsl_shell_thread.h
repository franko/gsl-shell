#ifndef FOXGUI_LUA_ENGINE_H
#define FOXGUI_LUA_ENGINE_H

extern "C" {
#include "lua.h"
}

#include "pthreadpp.h"
#include "gsl_shell_interp.h"
#include "redirect.h"

class gsl_shell_thread : public gsl_shell {
public:
  enum engine_status_e { starting, ready, busy, terminated };
  enum { eot_character = 0x04 };

  gsl_shell_thread();
  ~gsl_shell_thread();

  void input(const char* line);
  void start();
  void run();
  void stop();

  void lock() { pthread_mutex_lock(&this->exec_mutex); }
  void unlock() { pthread_mutex_unlock(&this->exec_mutex); }

  int read(char* buffer, unsigned buffer_size);

  int eval_status() const { return m_eval_status; }

private:
  pthread_t m_thread;
  engine_status_e m_status;
  stdout_redirect m_redirect;
  pthread::cond m_eval;
  const char* m_line_pending;
  int m_eval_status;
  bool m_exit_request;
};

#endif
