
#include "fox_gsl_shell.h"
#include "gsl_shell_app.h"
#include "lua_plot_window.h"
#include "window_registry.h"
#include "lua-graph.h"

void fox_gsl_shell::init()
{
    gsl_shell_thread::init();
}

void fox_gsl_shell::close()
{
    graph_close_windows(L);
    gsl_shell_thread::close();
}

void
fox_gsl_shell::before_eval()
{
    unsigned n = m_window_close_queue.size();
    for (unsigned k = 0; k < n; k++)
    {
        window_index_remove (L, m_window_close_queue[k]);
    }
    m_window_close_queue.clear();
}

void
fox_gsl_shell::quit_callback()
{
    if (m_close)
        m_close->signal();
}
void
fox_gsl_shell::restart_callback()
{
    m_app->lock();
    m_app->reset_console_request();
    m_app->wait_action();
    m_app->unlock();
}

void
fox_gsl_shell::window_close_notify(int window_id)
{
    pthread::mutex& eval = eval_mutex();
    eval.lock();
    m_window_close_queue.add(window_id);
    eval.unlock();
}
