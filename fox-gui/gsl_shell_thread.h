#ifndef FOXGUI_LUA_ENGINE_H
#define FOXGUI_LUA_ENGINE_H

extern "C" {
#include "lua.h"
}

#include <string>

#include "LuaInterpreter.h"
#include "pthreadpp.h"

class gsl_shell_thread : public LuaInterpreter
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
        pthread_mutex_lock(&m_exec_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&m_exec_mutex);
    }

    Interpreter::Result eval_status() const {
        return m_eval_status;
    }

    pthread::mutex& eval_mutex()
    {
        return m_eval;
    }

    // asyncronous request
    void interrupt_request();

private:
    thread_cmd_e process_request();

    pthread_t m_thread;
    pthread_mutex_t m_exec_mutex;
    engine_status_e m_status;
    pthread::cond m_eval;
    std::string m_line_pending;
    Interpreter::Result m_eval_status;
    request_e m_request;
};

#endif
