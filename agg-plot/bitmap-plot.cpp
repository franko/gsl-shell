
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "bitmap-plot.h"
#include "lua-cpp-utils.h"
#include "lua-plot-cpp.h"
#include "gs-types.h"
#include "canvas.h"
#include "colors.h"
#include "agg-pixfmt-config.h"
#include "platform_support_ext.h"

int
bitmap_save_image (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  const char *fn = luaL_checkstring (L, 2);
  int w = luaL_optint (L, 3, 480), h = luaL_optint (L, 4, 480);

  if (w <= 0 || w > 1024 * 8)
    luaL_error (L, "width out of range");

  if (h <= 0 || h > 1024 * 8)
    luaL_error (L, "height out of range");

  unsigned char * buffer = 0;
  try
    {
      agg::rendering_buffer rbuf_tmp;
      unsigned row_size = w * (gslshell::bpp / 8);
      unsigned buf_size = h * row_size;
      buffer = new unsigned char[buf_size];
      rbuf_tmp.attach(buffer, w, h, gslshell::flip_y ? row_size : -row_size);

      canvas can(rbuf_tmp, w, h, colors::white);
      agg::trans_affine mtx(w, 0.0, 0.0, h, 0.0, 0.0);

      agg::rect_base<int> r = rect_of_slot_matrix<int>(mtx);
      can.clear_box(r);

      p->draw(can, mtx);

      if (! platform_support_ext::save_image_file (rbuf_tmp, fn))
	{
	  delete [] buffer;
	  return luaL_error (L, "error saving image in filename %s", fn);
	}
    }
  catch (std::bad_alloc&)
    {
      if (buffer == 0) delete [] buffer;
      return luaL_error (L, "out of virtual memory");
    }

  delete [] buffer;
  return 0;
}
