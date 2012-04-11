#ifndef FOXGUI_FX_CONSOLE_H
#define FOXGUI_FX_CONSOLE_H

#include <new>

#include <fx.h>
#include <FXArray.h>

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

class input_history {
public:
  input_history() {}

  void append(const FXString& s) { m_input.append(s); }

  const FXString* operator[] (unsigned i) const
  {
    unsigned n = m_input.no();
    const FXString* p = (i < n ? &m_input[i] : 0);
    return p;
  }

private:
  FXArray<FXString> m_input;
};

class fx_console : public FXText {
  FXDECLARE(fx_console)

private:
  enum status_e { not_ready, input_mode, output_mode };

  static char const * const prompt;

public:
  fx_console(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=3,FXint pr=3,FXint pt=2,FXint pb=2);

  ~fx_console();

  // prepare to accept input
  void init(const FXString& greeting);
  void prepare_input();
  void show_errors();

public:
  virtual void create();

  void stop() { m_engine.stop(); }

  long on_key_press(FXObject*,FXSelector,void*);
  long on_lua_output(FXObject*,FXSelector,void*);

  enum {
    ID_READ_INPUT = FXText::ID_LAST,
    ID_LUA_OUTPUT,
    ID_LAST,
  };

protected:
  fx_console() {}

private:
  input_history m_history;
  FXint m_input_begin;
  FXString m_input;
  status_e m_status;
  gsl_shell_thread m_engine;

  lua_io_thread* m_lua_io_thread;
  FXGUISignal* m_lua_io_signal;
  FXMutex m_lua_io_mutex;
  FXString m_lua_io_buffer;
};

#endif
