#ifndef FOXGUI_LUA_ENGINE_H
#define FOXGUI_LUA_ENGINE_H

#include <pthread.h>

#include "redirect.h"

class lua_engine {
public:
  enum engine_status_e { starting, ready, busy, terminated };

  lua_engine();
  ~lua_engine();

  void input(const char* line);
  void start();
  void run();
  void set_state(engine_status_e s) { m_status = s; }
  bool is_ready();
  int read(char* buffer, unsigned buffer_size);
  //  const char* cmd() const { return m_line_pending; }

  //  void start_redirect() { m_redirect.start(); }
  //  void stop_redirect() { m_redirect.start(); }

private:
  engine_status_e m_status;
  stdout_redirect m_redirect;
  pthread_mutex_t m_eval_mutex;
  pthread_cond_t m_eval_ready;
  const char* m_line_pending;
};

#endif
