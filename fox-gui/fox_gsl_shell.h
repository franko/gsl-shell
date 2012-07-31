#ifndef FOX_GSL_SHELL_H
#define FOX_GSL_SHELL_H

#include <fx.h>
#include "agg_array.h"

#include "gsl_shell_thread.h"

class fox_gsl_shell : public gsl_shell_thread
{
public:
    fox_gsl_shell(FXApp* app): m_app(app) { }

    virtual void init();
    virtual void before_eval();

    void window_close_notify(int window_id);

private:
    FXApp* m_app;
    agg::pod_bvector<int> m_window_close_queue;
};

#endif
