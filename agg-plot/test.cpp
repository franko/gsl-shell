#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "platform/agg_platform_support.h"

#include "utils.h"
#include "plot.h"
#include "drawables.h"
#include "trans.h"

extern void platform_support_prepare();

enum flip_y_e { flip_y = true };

class the_application : public agg::platform_support
{
public:
  the_application(agg::pix_format_e format, bool flip_y) :
    agg::platform_support(format, flip_y)
  { };

  virtual ~the_application() { };
  
  virtual void on_draw()
  {
    canvas canvas(rbuf_window(), width(), height(), agg::rgba(1.0, 1.0, 1.0));

    canvas.clear();

    my::path tri;

    agg::path_storage& p = tri.get_path();
    p.move_to(0.0, 0.0);
    p.line_to(1.0, 0.0);
    p.line_to(0.5, 0.8);
    p.close_polygon();
  
    trans::resize ttri(tri);

    trans::stroke tris(ttri, 8.0);
    trans::stroke tris2(tris, 1.0);

    my::ellipse ell(1.5, 1.5, 0.3, 0.3);
    trans::resize tell(ell);
    trans::stroke ells(tell);

    my::text txt(-0.5, 1.5);
    txt.set_text("Hello world!");

    plot<vertex_source> plot;
    plot.add(&ttri,    agg::rgba8(0,180,0));
    plot.add(&tris,    agg::rgba8(0,0,120));
    plot.add(&tris2,   agg::rgba8(0,0,0));

    plot.add(&tell,    agg::rgba8(180,0,0));
    plot.add(&ells,    agg::rgba8(0,0,0));

    plot.add(&txt,     agg::rgba8(120,0,0));

    plot.draw(canvas);
  }
};

int main(int argc, char *argv[])
{
  platform_support_prepare();

  the_application app(agg::pix_format_bgr24, flip_y);
  app.caption("GSL shell plot - TESTING");

  if(app.init(300, 300, agg::window_resize))
    {
      app.run();
    }

  return 0;
};
