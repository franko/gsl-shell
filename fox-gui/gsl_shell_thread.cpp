#include <pthread.h>
#include <stdio.h>

#include "gsl_shell_thread.h"

extern "C" void * luajit_eval_thread (void *userdata);

void *
luajit_eval_thread (void *userdata)
{
    gsl_shell_thread* eng = (gsl_shell_thread*) userdata;
    eng->lock();
    eng->init();
    eng->run();
    pthread_exit(NULL);
    return NULL;
}

gsl_shell_thread::gsl_shell_thread():
    m_status(starting), m_request(no_request)
{
}

void gsl_shell_thread::start()
{
    pthread_attr_t attr[1];

    pthread_attr_init (attr);
    pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create (&m_thread, attr, luajit_eval_thread, (void*)this))
    {
        fprintf(stderr, "error creating thread");
    }
}

gsl_shell_thread::thread_cmd_e
gsl_shell_thread::process_request()
{
    thread_cmd_e cmd;

    if (m_request == gsl_shell_thread::exit_request)
    {
        cmd = thread_cmd_exit;
    }
    else if (m_request == gsl_shell_thread::restart_request)
    {
        this->close();
        this->init();
        restart_callback();
        cmd = thread_cmd_continue;
    }
    else if (m_request == gsl_shell_thread::execute_request)
    {
        cmd = thread_cmd_exec;
    }
    else
    {
        cmd = thread_cmd_continue;
    }

    m_request = gsl_shell_thread::no_request;
    return cmd;
}

void
gsl_shell_thread::run()
{
    str line;

    while (true)
    {
        this->unlock();
        m_eval.lock();
        m_status = waiting;

        while (!m_request)
        {
            m_eval.wait();
        }

        this->lock();
        before_eval();

        thread_cmd_e cmd = process_request();
        if (cmd == thread_cmd_exit)
        {
            m_status = terminated;
            m_eval.unlock();
            break;
        }

        if (cmd == thread_cmd_exec)
            line = m_line_pending;
        m_status = busy;
        m_eval.unlock();

        if (cmd == thread_cmd_exec)
        {
            m_eval_status = this->exec(line.cstr());
            fputc(eot_character, stdout);
            fflush(stdout);
        }
    }

    this->close();
    this->unlock();
    quit_callback();
}

void
gsl_shell_thread::set_request(gsl_shell_thread::request_e req, const char* line)
{
    m_eval.lock();
    m_request = req;
    if (line) m_line_pending = line;
    if (m_status == waiting)
    {
        m_eval.signal();
    }
    m_eval.unlock();
    sched_yield();
}
