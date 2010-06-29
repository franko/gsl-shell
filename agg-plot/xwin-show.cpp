#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "platform/agg_platform_support.h"

#include "xwin-show.h"
#include "gsl-shell.h"
#include "canvas.h"
#include "lua-plot.h"
#include "lua-cpp-utils.h"
#include "lua-draw.h"


extern void platform_support_prepare   ();
extern void platform_support_lock      (agg::platform_support *app);
extern void platform_support_unlock    (agg::platform_support *app);
extern bool platform_support_is_mapped (agg::platform_support *app);

enum flip_y_e { flip_y = true };

class the_application : public agg::platform_support
{
public:
  the_application(agg::pix_format_e format, bool flip_y, agg_plot *p) :
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
    this->m_plot->draw(can);
  }
  
  virtual void on_draw()
  {
    AGG_PROTECT(on_draw_unprotected());
  }
  
private:
  agg_plot *m_plot;
};

int update_callback (void *_app)
{
  the_application *app = (the_application *) _app;
  int status = 0;

  platform_support_lock (app);

  if (platform_support_is_mapped (app))
    {
      app->on_draw_unprotected();
      app->update_window();
      status = 1;
    }

  platform_support_unlock (app);
  return status;
};
			
void *
xwin_thread_function (void *_plot) 
{
  agg_plot *p = (agg_plot *) _plot;

  platform_support_prepare();

  AGG_LOCK();

  the_application app(agg::pix_format_bgr24, flip_y, p);
  app.caption("GSL shell plot");

  if(app.init(780, 400, agg::window_resize))
    {
      p->window = (void *) &app;
      AGG_UNLOCK();

      /* start main loop for the plot window */
      app.run();

      AGG_LOCK();
      p->window = NULL;
      p->is_shown = 0;
      AGG_UNLOCK();

      GSL_SHELL_LOCK();
      printf("unref plot\n");
      gsl_shell_unref_plot (p->id);
      GSL_SHELL_UNLOCK();
    }

  return NULL;
}
