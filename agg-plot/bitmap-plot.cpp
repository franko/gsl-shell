
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

void
bitmap_save_image_cpp (lua_plot *p, const char *fn, unsigned w, unsigned h,
		       gslshell::ret_status& st)
{
  try {
    agg::rendering_buffer rbuf_tmp;
    unsigned row_size = w * (gslshell::bpp / 8);
    unsigned buf_size = h * row_size;
    agg::pod_array<unsigned char> buffer(buf_size);
    rbuf_tmp.attach(buffer.data(), w, h, gslshell::flip_y ? row_size : -row_size);

    canvas can(rbuf_tmp, w, h, colors::white);
    agg::trans_affine mtx(w, 0.0, 0.0, h, 0.0, 0.0);

    agg::rect_base<int> r = rect_of_slot_matrix<int>(mtx);
    can.clear_box(r);

    p->draw(can, mtx);
  
    bool success = platform_support_ext::save_image_file (rbuf_tmp, fn);

    if (! success)
      st.error("cannot save image file", "plot save");
  }
  catch (std::exception& e)
    {
      st.error(e.what(), "plot save");
    }
}

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

  gslshell::ret_status st;
  bitmap_save_image_cpp (p, fn, w, h, st);
  if (st.error_msg())
    return luaL_error (L, "%s in %s", st.error_msg(), st.context());

  return 0;

}
