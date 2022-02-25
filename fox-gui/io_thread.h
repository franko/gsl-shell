#ifndef FOXGUI_IO_THREAD_H
#define FOXGUI_IO_THREAD_H

#include <fx.h>

#include "gsl_shell_thread.h"
#include "redirect.h"

class lua_io_thread {
public:
    lua_io_thread(io_redirect* lua_io, FXObject *io_target, FXSelector io_selector, FXMessageChannel *io_channel, FXMutex* mut, FXString* buf):
        m_redirect(lua_io),
        m_io_target(io_target), m_io_selector(io_selector), m_io_channel(io_channel),
        m_io_protect(mut), m_io_buffer(buf)
    { }

    void run();
    void start();

private:
    pthread_t m_thread;
    io_redirect* m_redirect;
    FXObject *m_io_target;
    FXSelector m_io_selector;
    FXMessageChannel *m_io_channel;
    FXMutex* m_io_protect;
    FXString* m_io_buffer;
};

#endif
