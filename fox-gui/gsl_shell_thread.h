#ifndef FOXGUI_LUA_ENGINE_H
#define FOXGUI_LUA_ENGINE_H

extern "C" {
#include "lua.h"
}

#include "gsl_shell_interp.h"
#include "pthreadpp.h"
#include "strpp.h"

class gsl_shell_thread
{
    enum thread_cmd_e { thread_cmd_exec, thread_cmd_exit, thread_cmd_continue };
public:
    enum engine_status_e { starting, waiting, busy, terminated };
    enum request_e { no_request = 0, exit_request, restart_request, execute_request };
    enum { eot_character = 0x04 };

    gsl_shell_thread();

    void set_request(request_e req, const char* line = 0);

    void start();
    void run();

    virtual void before_eval() { }
    virtual void restart_callback() { }
    virtual void quit_callback() { }

    void lock()
    {
        gsl_shell_interp_lock(this->m_gsl_shell);
    }

    void unlock()
    {
        gsl_shell_interp_unlock(this->m_gsl_shell);
    }

    int eval_status() const
    {
        return m_eval_status;
    }

    pthread::mutex& eval_mutex()
    {
        return m_eval;
    }

    const char* error_msg() const
    {
        return gsl_shell_interp_error_msg(this->m_gsl_shell);
    }

    gsl_shell_interp * interp() { return m_gsl_shell; }

    // asyncronous request
    void interrupt_request();

private:
    thread_cmd_e process_request();

    pthread_t m_thread;
    gsl_shell_interp m_gsl_shell[1];
    engine_status_e m_status;
    pthread::cond m_eval;
    str m_line_pending;
    int m_eval_status;
    request_e m_request;
};

#endif
