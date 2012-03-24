#include <pthread.h>
#include <stdio.h>
//#include <io.h>

#include "lua_engine.h"
#include "gsl_shell_interp.h"

extern "C" void * luajit_eval_thread (void *userdata);

void *
luajit_eval_thread (void *userdata)
{
  lua_engine* eng = (lua_engine*) userdata;
  eng->start_gsl_shell();
  eng->run();
  pthread_exit (NULL);
  return NULL;
}

static int
start_interp_thread (lua_engine* eng)
{
  pthread_t eval_thread[1];
  pthread_attr_t attr[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

  if (pthread_create (eval_thread, attr, luajit_eval_thread, eng))
    {
      fprintf(stderr, "error creating thread");
      return 1;
    }

  return 0;
}

lua_engine::~lua_engine()
{
  pthread_mutex_destroy(&m_eval_mutex);
  pthread_cond_destroy(&m_eval_ready);
  m_redirect.stop();
}

lua_engine::lua_engine()
  : m_status(starting), m_redirect(4096), m_line_pending(0), m_lua_state(0)
{
  pthread_mutex_init(&m_eval_mutex, NULL);
  pthread_cond_init(&m_eval_ready, NULL);
}

void lua_engine::start_gsl_shell()
{
  m_lua_state = gsl_shell_init();
}

void lua_engine::start()
{
  m_redirect.start();
  start_interp_thread(this);
}

void
lua_engine::run()
{
  while (1)
    {
      pthread_mutex_lock (&m_eval_mutex);
      this->set_state(ready);
      pthread_cond_wait (&m_eval_ready, &m_eval_mutex);

      const char* line = m_line_pending;
      m_line_pending = NULL;

      this->set_state(busy);
      pthread_mutex_unlock(&m_eval_mutex);

      gsl_shell_exec (m_lua_state, line);

      fputc(0x1a, stdout);

      fflush(stdout);
    }
}

void
lua_engine::input(const char* line)
{
  pthread_mutex_lock (&m_eval_mutex);
  if (m_status == ready)
    {
      m_line_pending = line;
      pthread_cond_signal (&m_eval_ready);
    }
  pthread_mutex_unlock (&m_eval_mutex);
}

int
lua_engine::read(char* buffer, unsigned buffer_size)
{
  return m_redirect.read(buffer, buffer_size);
}

bool lua_engine::is_ready()
{
  pthread_mutex_lock (&m_eval_mutex);
  bool is_ready = (m_status == ready);
  pthread_mutex_unlock (&m_eval_mutex);
  return is_ready;
}
