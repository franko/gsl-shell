#include <errno.h>

#include "io_thread.h"

FXint lua_io_thread::run()
{
    char buffer[128];

    while (1)
    {
        int nr = m_engine->read(buffer, 127);
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

        m_io_ready->signal();
    }

    return 0;
}
