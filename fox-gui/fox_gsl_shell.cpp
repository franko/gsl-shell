
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
    m_window_close_queue.lock();
    unsigned n = m_window_close_queue.size();
    for (unsigned k = 0; k < n; k++)
    {
        window_index_remove (L, m_window_close_queue[k]);
    }
    m_window_close_queue.clear();
    m_window_close_queue.unlock();
}

void
fox_gsl_shell::quit_callback()
{
    if (m_close_channel) {
        m_close_channel->message(m_close_target, m_close_selector, (void *) this, sizeof(int));
    }
}
void
fox_gsl_shell::restart_callback()
{
    m_app->lock();
    m_app->send_request({gsl_shell_app::clear_console_rq});
    m_app->unlock();
}

void
fox_gsl_shell::window_close_notify(int window_id)
{
    m_window_close_queue.lock();
    m_window_close_queue.add(window_id);
    m_window_close_queue.unlock();
}
