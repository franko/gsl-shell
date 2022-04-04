#ifndef FOX_GSL_SHELL_H
#define FOX_GSL_SHELL_H

#include <fx.h>
#include "agg_array.h"

#include "gsl_shell_thread.h"
#include "shared_vector.h"

class gsl_shell_app;

class fox_gsl_shell : public gsl_shell_thread
{
public:
    fox_gsl_shell(gsl_shell_app* app): m_app(app), m_app_channel(nullptr) { }

    virtual void init();
    virtual void close();

    virtual void before_eval();
    virtual void restart_callback();
    virtual void quit_callback();

    // Set the channel to communicate with the FXApp and the selector needed for this
    // latter to close.
    void bind_app_channel(FXSelector close_selector, FXMessageChannel *app_channel) {
        m_close_selector = close_selector;
        m_app_channel = app_channel;
    }

    void window_close_notify(int window_id);

private:
    gsl_shell_app* m_app;
    FXSelector m_close_selector;
    FXMessageChannel *m_app_channel;
    shared_vector<int> m_window_close_queue;
};

#endif
