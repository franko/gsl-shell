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
    fox_gsl_shell(gsl_shell_app* app): m_app(app), m_close_channel(nullptr) { }

    virtual void init();
    virtual void close();

    virtual void before_eval();
    virtual void restart_callback();
    virtual void quit_callback();

    void set_closing_signal(FXObject *target, FXSelector sel, FXMessageChannel *chan) {
        m_close_target = target;
        m_close_selector = sel;
        m_close_channel = chan;
    }

    void window_close_notify(int window_id);

private:
    gsl_shell_app* m_app;
    FXObject *m_close_target;
    FXSelector m_close_selector;
    FXMessageChannel *m_close_channel;
    shared_vector<int> m_window_close_queue;
};

#endif
