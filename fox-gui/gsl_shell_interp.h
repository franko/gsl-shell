#ifndef GSL_SHELL_INTERP_H
#define GSL_SHELL_INTERP_H

extern "C" {
#include <lua.h>
}

#include "defs.h"
#include "pthreadpp.h"
#include "lua-gsl.h"
#include "strpp.h"
#include "fatal.h"

class gsl_shell : public gsl_shell_state
{
public:
    enum eval_result_e { eval_success, eval_error, incomplete_input };

    gsl_shell()
    {
        gsl_shell_init(this);
    }

    virtual ~gsl_shell()
    {
        if (unlikely(this->L != NULL))
            fatal_exception("warning: attempt to dispose an open Lua state");
        gsl_shell_free(this);
    }

    virtual void init();
    virtual void close();

    int exec(const char* line);
    const char* error_msg() const
    {
        return m_error_msg.cstr();
    }

private:
    int error_report(int status);

    str m_error_msg;
};

#endif
