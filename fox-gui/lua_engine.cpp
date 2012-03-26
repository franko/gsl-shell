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
  m_redirect.stop();
}

lua_engine::lua_engine()
  : m_status(starting), m_redirect(4096), m_line_pending(0)
{
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
      m_eval.lock();
      this->set_state(ready);
      m_eval.wait();

      const char* line = m_line_pending;
      m_line_pending = NULL;

      this->set_state(busy);
      m_eval.unlock();

      m_eval_status = m_gsl_shell.exec(line);

      fputc(eot_character, stdout);
      fflush(stdout);
    }
}

void
lua_engine::input(const char* line)
{
  m_eval.lock();
  if (m_status == ready)
    {
      m_line_pending = line;
      m_eval.signal();
    }
  m_eval.unlock();
}

int
lua_engine::read(char* buffer, unsigned buffer_size)
{
  return m_redirect.read(buffer, buffer_size);
}

bool lua_engine::is_ready()
{
  m_eval.lock();
  bool is_ready = (m_status == ready);
  m_eval.unlock();
  return is_ready;
}
