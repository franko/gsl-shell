#ifndef FOXGUI_IO_THREAD_H
#define FOXGUI_IO_THREAD_H

#include <fx.h>

#include "gsl_shell_thread.h"

class lua_io_thread : public FXThread {
public:
  lua_io_thread(gsl_shell_thread* eng, FXGUISignal* sig, FXMutex* mut, FXString* buf):
    m_engine(eng), m_io_ready(sig), m_io_protect(mut), m_io_buffer(buf)
  { }

  virtual FXint run();

private:
  gsl_shell_thread* m_engine;
  FXGUISignal* m_io_ready;
  FXMutex* m_io_protect;
  FXString* m_io_buffer;
};

#endif
