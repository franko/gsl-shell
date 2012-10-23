#ifndef FOXGUI_IO_THREAD_H
#define FOXGUI_IO_THREAD_H

#include <fx.h>

#include "gsl_shell_thread.h"
#include "redirect.h"

class lua_io_thread {
public:
    lua_io_thread(io_redirect* lua_io, FXGUISignal* sig, FXMutex* mut, FXString* buf):
        m_redirect(lua_io), m_io_ready(sig), m_io_protect(mut), m_io_buffer(buf)
    { }

    void run();
    void start();

private:
    pthread_t m_thread;
    io_redirect* m_redirect;
    FXGUISignal* m_io_ready;
    FXMutex* m_io_protect;
    FXString* m_io_buffer;
};

#endif
