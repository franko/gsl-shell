#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "platform/agg_platform_support.h"

#include "xwin-show.h"
#include "canvas.h"
#include "plot.h"
#include "lua-plot-priv.h"


extern void platform_support_prepare();

enum flip_y_e { flip_y = true };

class the_application : public agg::platform_support
{
public:
  the_application(agg::pix_format_e format, bool flip_y, struct agg_plot *p) :
    agg::platform_support(format, flip_y), m_plot(p)
  {
  };

  virtual ~the_application()
  {
  }
  
  void on_draw_unprotected()
  {
    canvas can(rbuf_window(), width(), height(), agg::rgba(1.0, 1.0, 1.0));
    can.clear();
    plot_type* plot = (plot_type*) m_plot->plot;
    plot->draw(can);
  }
  
  virtual void on_draw()
  {
    pthread_mutex_lock (m_plot->mutex);
    on_draw_unprotected();
    pthread_mutex_unlock (m_plot->mutex);
  }
  
private:
  struct agg_plot *m_plot;
};

void update_callback (void *_app)
{
  the_application *app = (the_application *) _app;
  app->on_draw_unprotected();
  app->update_window();
};
			
void *
xwin_thread_function (void *_plot) 
{
  struct agg_plot *p = (struct agg_plot *) _plot;

  platform_support_prepare();

  pthread_mutex_lock (p->mutex);

  the_application app(agg::pix_format_bgr24, flip_y, p);
  app.caption("GSL shell plot");

  if(app.init(780, 400, agg::window_resize))
    {
      p->window = (void *) &app;
      pthread_mutex_unlock (p->mutex);
      app.run();
    }
  else
    {
      pthread_mutex_unlock (p->mutex);
    }

  pthread_mutex_lock (p->mutex);
  p->window = NULL;
  if (p->lua_is_owner)
    {
      p->is_shown = 0;
      pthread_mutex_unlock (p->mutex);
    }
  else
    {
      pthread_mutex_unlock (p->mutex);
      agg_plot_destroy (p);
    }

  return NULL;
}
