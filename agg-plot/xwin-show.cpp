#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "platform/agg_platform_support.h"

#include "xwin-show.h"
#include "canvas.h"
#include "cplot.h"
#include "lua-cplot-priv.h"

enum flip_y_e { flip_y = true };

class the_application : public agg::platform_support
{
public:
  the_application(agg::pix_format_e format, bool flip_y, cplot *plot,
		  pthread_mutex_t *mutex) :
    agg::platform_support(format, flip_y), m_plot(plot), m_mutex(mutex)
  {
  };

  virtual ~the_application()
  {
  }
  
  virtual void on_draw()
  {
    pthread_mutex_lock (m_mutex);
    canvas can(rbuf_window(), width(), height(), agg::rgba(1.0, 1.0, 1.0));
    can.clear();
    m_plot->draw(can);
    pthread_mutex_unlock (m_mutex);
  }
  
private:
  cplot* m_plot;
  pthread_mutex_t *m_mutex;
};

void update_callback (void *_app)
{
  the_application *app = (the_application *) _app;
  app->on_draw();
  app->update_window();
};
			
void *
xwin_thread_function (void *_cplot) 
{
  struct lcplot *cp = (struct lcplot *) _cplot;

  pthread_mutex_lock (cp->mutex);
  cp->is_shown = 1;

  the_application app(agg::pix_format_bgr24, flip_y, cp->plot, cp->mutex);
  app.caption("GSL shell plot");

  if(app.init(780, 400, agg::window_resize))
    {
      cp->x_app = (void *) &app;
      pthread_mutex_unlock (cp->mutex);
      app.run();
    }
  else
    {
      pthread_mutex_lock (cp->mutex);
    }

  pthread_mutex_lock (cp->mutex);
  cp->x_app = NULL;
  if (cp->lua_is_owner)
    {
      cp->is_shown = 0;
      pthread_mutex_unlock (cp->mutex);
    }
  else
    {
      pthread_mutex_unlock (cp->mutex);
      lcplot_destroy (cp);
    }

  return NULL;
}
