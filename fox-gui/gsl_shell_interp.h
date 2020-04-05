#ifndef GSL_SHELL_INTERP_H
#define GSL_SHELL_INTERP_H

extern "C" {
#include <lua.h>
}

#include <string>

#include "defs.h"
#include "fatal.h"

class gsl_shell
{
public:
    enum eval_result_e { eval_success, eval_error, incomplete_input };

    gsl_shell() : m_lua_state(nullptr) {  }
    virtual ~gsl_shell() { }

    virtual void init();
    virtual void close();

    int exec(const char* line);
    void interrupt();

    const char* error_msg() const
    {
        return m_error_msg.c_str();
    }

private:
    int error_report(int status);

    lua_State *m_lua_state;
    std::string m_error_msg;
};

#endif
