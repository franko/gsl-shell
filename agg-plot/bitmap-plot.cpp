
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
  const char *fn = lua_tostring (L, 2);
  unsigned w, h;

  if (fn == NULL)
    return gs_type_error (L, 2, "string");

  if (lua_gettop (L) >= 4)
    {
      int iw = luaL_checkinteger (L, 3), ih = luaL_checkinteger (L, 4);
      if (iw <= 0 || iw > 1024 * 8 || ih <= 0 || ih > 1024 * 8)
	return luaL_error (L, "width and height should be positive numbers");
      w = (unsigned) iw;
      h = (unsigned) ih;
    }
  else
    {
      w = 480;
      h = 480;
    }

  agg::rendering_buffer rbuf_tmp;
  unsigned row_size = w * (gslshell::bpp / 8);
  unsigned buf_size = h * row_size;
  unsigned char *buffer = new unsigned char[buf_size];
  rbuf_tmp.attach(buffer, w, h, gslshell::flip_y ? row_size : -row_size);

  canvas can(rbuf_tmp, w, h, colors::white);
  agg::trans_affine mtx(w, 0.0, 0.0, h, 0.0, 0.0);

  agg::rect_base<int> r = rect_of_slot_matrix<int>(mtx);
  can.clear_box(r);

  p->draw(can, mtx);

  if (! platform_support_ext::save_image_file (rbuf_tmp, fn))
    {
      delete buffer;
      return luaL_error (L, "error saving image in filename %s", fn);
    }

  delete buffer;
  return 0;
}
