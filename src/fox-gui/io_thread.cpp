#include <pthread.h>
#include <errno.h>

#include "io_thread.h"

static void* io_thread_run(void* data)
{
    lua_io_thread* thread = (lua_io_thread*) data;
    thread->run();
    return 0;
}

void lua_io_thread::run()
{
    char buffer[128];

    while (1)
    {
        int nr = m_redirect->read(buffer, 127);
        if (nr < 0)
        {
            fprintf(stderr, "ERROR on read: %d.\n", errno);
            break;
        }
        if (nr == 0)
            break;

        buffer[nr] = 0;

        m_io_protect->lock();
        m_io_buffer->append((const FXchar*)buffer);
        m_io_protect->unlock();

        m_io_channel->message(m_io_target, m_io_selector, (void *) this, sizeof(int));
    }
}

void lua_io_thread::start()
{
    pthread_attr_t attr[1];

    pthread_attr_init (attr);
    pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create (&m_thread, attr, io_thread_run, (void*)this))
    {
        fprintf(stderr, "error creating thread");
    }
}
