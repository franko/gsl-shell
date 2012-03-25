#ifndef MY_PTHREADPP_H
#define MY_PTHREADPP_H

#include <pthread.h>

namespace pthread {

  class mutex {
  public:
    mutex() { pthread_mutex_init(&m_mutex, NULL); }
    ~mutex() { pthread_mutex_destroy(&m_mutex); }

    void lock() { pthread_mutex_lock(&m_mutex); }
    void unlock() { pthread_mutex_unlock(&m_mutex); }

    pthread_mutex_t* mutex_ptr() { return &m_mutex; }

  private:
    pthread_mutex_t m_mutex;
  };

  class cond : public mutex {
  public:
    cond() { pthread_cond_init(&m_cond, NULL); }
    ~cond() { pthread_cond_destroy(&m_cond); }

    void signal() { pthread_cond_signal(&m_cond); }
    void wait() { pthread_cond_wait(&m_cond, mutex_ptr()); }

  private:
    pthread_cond_t m_cond;
  };
}

#endif
