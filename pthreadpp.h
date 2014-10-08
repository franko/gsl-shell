#ifndef CPP_PTHREADPP_H
#define CPP_PTHREADPP_H

#include <pthread.h>

/* Simple C++ wrapper around basic mutex/condition operations.
   The added value of the wrapper is more clean C++ code with automatic
   initialization/deallocation of resources. */

namespace pthread {

  class mutex {
  public:
    mutex() { pthread_mutex_init(&m_mutex, NULL); }
    ~mutex() { pthread_mutex_destroy(&m_mutex); }

    void lock() { pthread_mutex_lock(&m_mutex); }
    void unlock() { pthread_mutex_unlock(&m_mutex); }

    pthread_mutex_t* mutex_ptr() { return &m_mutex; }

  private:
    mutex(const mutex&);
    mutex& operator= (const mutex&);

    pthread_mutex_t m_mutex;
  };

  class auto_lock {
  public:
    auto_lock(mutex& m): m_mutex(m) { m_mutex.lock(); }
    ~auto_lock() { m_mutex.unlock(); }
  private:
    mutex& m_mutex;
  };

  /* Since the official documentation recommend to use pthread conditions
     always in pair with a mutex we let a condtion inherit from a mutex.
     In this way a C++ "cond" instance can perform both mutex and condition
     operations. */
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
