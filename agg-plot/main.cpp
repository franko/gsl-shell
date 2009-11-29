#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_ellipse.h"
#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_pixfmt_rgb.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_image_filters.h"
#include "ctrl/agg_slider_ctrl.h"
#include "ctrl/agg_rbox_ctrl.h"
#include "ctrl/agg_cbox_ctrl.h"
#include "platform/agg_platform_support.h"

#include "canvas.h"
#include "units_cplot.h"

#include "my_list.h"

enum flip_y_e { flip_y = true };

class the_application : public agg::platform_support
{
public:
  the_application(agg::pix_format_e format, bool flip_y) :
    agg::platform_support(format, flip_y), m_plot()
  {
    {
      poly_outline* ln = new poly_outline(agg::rgba(0.7, 0.5, 0.1), agg::rgba(0,0,0));
      agg::path_storage& p = ln->path;

      p.move_to(-3.0,  -0.7);
      p.line_to(35.0, 2.5);
      p.line_to(5.0,  4.0);
      p.close_polygon();

      m_plot.add(ln);
    }
    {
      line* ln = new line(agg::rgba(0.7, 0, 0));
      agg::path_storage& p = ln->path;
      const int npt = 512, ncycles = 12;
      for (int j = 0; j < npt; j++)
	{
	  double x = j * 2 * M_PI * ncycles / npt;
	  double y = exp(-0.05 * x) * sin(x) +3;
	  if (j == 0)
	    p.move_to(x, y);
	  else
	    p.line_to(x, y);
	}
      m_plot.add(ln);
    }

    {
      line* ln = new line(agg::rgba(0, 0, 0.7));
      agg::path_storage& p = ln->path;

      const int npt = 512, ncycles = 12;
      for (int j = 0; j < npt; j++)
	{
	  double x = j * 2 * M_PI * ncycles / npt;
	  double y = 1.8 * exp(-0.1 * x);
	  if (j == 0)
	    p.move_to(x, y);
	  else
	    p.line_to(x, y);
	}
      m_plot.add(ln);
    }
  };

  virtual ~the_application()
  {
  }
  
  virtual void on_draw()
  {
    canvas can(rbuf_window(), width(), height(), agg::rgba(1.0, 1.0, 1.0));
    can.clear();
    m_plot.draw(can);
  }

private:
  units_cplot m_plot;
};

int agg_main(int argc, char* argv[])
{
  the_application app(agg::pix_format_bgr24, flip_y);
  app.caption("My damn test");

  if(app.init(780, 400, agg::window_resize))
    {
      return app.run();
    }
  return 0;
}
