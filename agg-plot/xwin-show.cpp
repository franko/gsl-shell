#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "platform/agg_platform_support.h"

#include "xwin-show.h"
#include "canvas.h"
#include "cplot.h"
#include "lua-cplot-priv.h"


extern void platform_support_prepare();

enum flip_y_e { flip_y = true };

class the_application : public agg::platform_support
{
public:
  the_application(agg::pix_format_e format, bool flip_y, struct lcplot *cp) :
    agg::platform_support(format, flip_y), m_cp(cp)
  {
  };

  virtual ~the_application()
  {
  }
  
  void on_draw_unprotected()
  {
    canvas can(rbuf_window(), width(), height(), agg::rgba(1.0, 1.0, 1.0));
    can.clear();
    m_cp->plot->draw(can);
  }
  
  virtual void on_draw()
  {
    pthread_mutex_lock (m_cp->mutex);
    on_draw_unprotected();
    pthread_mutex_unlock (m_cp->mutex);
  }
  
private:
  struct lcplot *m_cp;
};

void update_callback (void *_app)
{
  the_application *app = (the_application *) _app;
  app->on_draw_unprotected();
  app->update_window();
};
			
void *
xwin_thread_function (void *_cplot) 
{
  struct lcplot *cp = (struct lcplot *) _cplot;

  platform_support_prepare();

  printf("locking for cplot: %p\n", _cplot);
  pthread_mutex_lock (cp->mutex);

  the_application app(agg::pix_format_bgr24, flip_y, cp);
  app.caption("GSL shell plot");

  if(app.init(780, 400, agg::window_resize))
    {
      cp->window = (void *) &app;
      pthread_mutex_unlock (cp->mutex);
      app.run();
    }
  else
    {
      pthread_mutex_unlock (cp->mutex);
    }

  printf("thread finishing, locking for cplot: %p...", _cplot);
  fflush(stdout);
  pthread_mutex_lock (cp->mutex);
  printf("locked!\n");
  fflush(stdout);
  cp->window = NULL;
  if (cp->lua_is_owner)
    {
      printf ("thread finished, Lua is still owner for %p\n", _cplot);
      cp->is_shown = 0;
      pthread_mutex_unlock (cp->mutex);
    }
  else
    {
      printf ("thread finished, destroying plot and resources for %p\n", 
	      _cplot);
      pthread_mutex_unlock (cp->mutex);
      lcplot_destroy (cp);
    }

  return NULL;
}
