#ifndef GSL_SHELL_INTERP_H
#define GSL_SHELL_INTERP_H

#include "defs.h"
#include "pthreadpp.h"

extern "C" {
#include "lua.h"
}

class gsl_shell {
public:
  gsl_shell(): m_lua_state(0) { }
  ~gsl_shell();

  void init();
  int exec(const char* line);

private:
  lua_State* m_lua_state;
  pthread::mutex m_interp;
};

#endif
