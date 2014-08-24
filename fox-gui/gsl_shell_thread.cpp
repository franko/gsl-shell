#include <pthread.h>
#include <stdio.h>

#include "gsl_shell_thread.h"
#include "graphics-hooks.h"
#include "lua-graph.h"
#include "lua-gsl.h"

extern "C" void * luajit_eval_thread (void *userdata);

static int start_interp(gsl_shell_thread* eng)
{
    gsl_shell_interp* gs = eng->interp();
    gsl_shell_interp_open(gs);
    graphics->init(gs->L); /* Initialize graphics module. */
    luaopen_gsl(gs->L); /* Perform some GSL Shell's specific initializations. */
    int status = gsl_shell_interp_dolibrary(gs, "gslext");
    return status;
}

void *
luajit_eval_thread (void *userdata)
{
    gsl_shell_thread* eng = (gsl_shell_thread*) userdata;
    eng->lock();
    start_interp(eng);
    eng->run();
    pthread_exit(NULL);
    return NULL;
}

gsl_shell_thread::gsl_shell_thread():
    m_status(starting), m_request(no_request)
{
    gsl_shell_interp_init(m_gsl_shell);
}

gsl_shell_thread::~gsl_shell_thread()
{
    gsl_shell_interp_free(m_gsl_shell);
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
        graph_close_windows(m_gsl_shell->L);
        gsl_shell_interp_close(m_gsl_shell);
        start_interp(this);
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
            m_eval_status = gsl_shell_interp_exec(m_gsl_shell, line.cstr());
            if (m_eval_status >> 8 == exit_command) {
                m_status = terminated;
                break;
            }
            fputc(eot_character, stdout);
            fflush(stdout);
        }
    }

    graph_close_windows(m_gsl_shell->L);
    gsl_shell_interp_close(m_gsl_shell);
    this->unlock();
    quit_callback();
}

void
gsl_shell_thread::set_request(gsl_shell_thread::request_e req, const char* line)
{
    m_eval.lock();
    m_request = req;
    if (line) m_line_pending = line;
    if (m_status == waiting) {
        m_eval.signal();
    }
    m_eval.unlock();
    sched_yield();
}

void
gsl_shell_thread::interrupt_request()
{
    m_eval.lock();
    if (m_status == busy) {
        gsl_shell_interp_interrupt(m_gsl_shell);
    }
    m_eval.unlock();
}
