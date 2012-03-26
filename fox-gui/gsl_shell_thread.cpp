#include <pthread.h>
#include <stdio.h>

#include "gsl_shell_thread.h"
#include "gsl_shell_interp.h"

extern "C" void * luajit_eval_thread (void *userdata);

void *
luajit_eval_thread (void *userdata)
{
  gsl_shell_thread* eng = (gsl_shell_thread*) userdata;
  eng->init();
  eng->run();
  pthread_exit (NULL);
  return NULL;
}

static int
start_interp_thread (gsl_shell_thread* eng)
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

gsl_shell_thread::~gsl_shell_thread()
{
  m_redirect.stop();
}

gsl_shell_thread::gsl_shell_thread()
  : m_status(starting), m_redirect(4096), m_line_pending(0)
{
}

void gsl_shell_thread::start()
{
  m_redirect.start();
  start_interp_thread(this);
}

void
gsl_shell_thread::run()
{
  while (1)
    {
      m_eval.lock();
      m_status = ready;
      m_eval.wait();

      const char* line = m_line_pending;
      m_line_pending = NULL;

      m_status = busy;
      m_eval.unlock();

      m_eval_status = this->exec(line);

      fputc(eot_character, stdout);
      fflush(stdout);
    }
}

void
gsl_shell_thread::input(const char* line)
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
gsl_shell_thread::read(char* buffer, unsigned buffer_size)
{
  return m_redirect.read(buffer, buffer_size);
}

bool gsl_shell_thread::is_ready()
{
  m_eval.lock();
  bool is_ready = (m_status == ready);
  m_eval.unlock();
  return is_ready;
}
