#include "api.h"
#include "renderer.h"
#include "rencache.h"


static RenColor checkcolor(lua_State *L, int idx, int def) {
  RenColor color;
  if (lua_isnoneornil(L, idx)) {
    return (RenColor) { def, def, def, 255 };
  }
  lua_rawgeti(L, idx, 1);
  lua_rawgeti(L, idx, 2);
  lua_rawgeti(L, idx, 3);
  lua_rawgeti(L, idx, 4);
  color.r = luaL_checknumber(L, -4);
  color.g = luaL_checknumber(L, -3);
  color.b = luaL_checknumber(L, -2);
  color.a = luaL_optnumber(L, -1, 255);
  lua_pop(L, 4);
  return color;
}


static int f_show_debug(lua_State *L) {
  luaL_checkany(L, 1);
  rencache_show_debug(lua_toboolean(L, 1));
  return 0;
}


static int f_get_size(lua_State *L) {
  int w, h;
  ren_get_size(&w, &h);
  lua_pushnumber(L, w);
  lua_pushnumber(L, h);
  return 2;
}


static int f_begin_frame(lua_State *L) {
  rencache_begin_frame(L);
  return 0;
}


static int f_end_frame(lua_State *L) {
  rencache_end_frame(L);
  return 0;
}


static RenRect rect_to_grid(lua_Number x, lua_Number y, lua_Number w, lua_Number h) {
  int x1 = (int) (x + 0.5), y1 = (int) (y + 0.5);
  int x2 = (int) (x + w + 0.5), y2 = (int) (y + h + 0.5);
  return (RenRect) {x1, y1, x2 - x1, y2 - y1};
}


static int f_set_clip_rect(lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);
  lua_Number w = luaL_checknumber(L, 3);
  lua_Number h = luaL_checknumber(L, 4);
  RenRect rect = rect_to_grid(x, y, w, h);
  rencache_set_clip_rect(rect);
  return 0;
}


static int f_draw_rect(lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);
  lua_Number w = luaL_checknumber(L, 3);
  lua_Number h = luaL_checknumber(L, 4);
  RenRect rect = rect_to_grid(x, y, w, h);
  RenColor color = checkcolor(L, 5, 255);
  rencache_draw_rect(rect, color);
  return 0;
}

static int draw_text_subpixel_impl(lua_State *L, bool draw_subpixel) {
  FontDesc *font_desc = luaL_checkudata(L, 1, API_TYPE_FONT);
  const char *text = luaL_checkstring(L, 2);
  /* The coordinate below will be in subpixel iff draw_subpixel is true.
     Otherwise it will be in pixels. */
  int x_subpixel = luaL_checknumber(L, 3);
  int y = luaL_checknumber(L, 4);
  RenColor color = checkcolor(L, 5, 255);

  CPReplaceTable *rep_table;
  RenColor replace_color;
  if (lua_gettop(L) >= 7) {
    rep_table = luaL_checkudata(L, 6, API_TYPE_REPLACE);
    replace_color = checkcolor(L, 7, 255);
  } else {
    rep_table = NULL;
    replace_color = (RenColor) {0};
  }

  x_subpixel = rencache_draw_text(L, font_desc, 1, text, x_subpixel, y, color, draw_subpixel, rep_table, replace_color);
  lua_pushnumber(L, x_subpixel);
  return 1;
}

static int f_draw_text(lua_State *L) {
  return draw_text_subpixel_impl(L, false);
}


static int f_draw_text_subpixel(lua_State *L) {
  return draw_text_subpixel_impl(L, true);
}


static const luaL_Reg lib[] = {
  { "show_debug",         f_show_debug         },
  { "get_size",           f_get_size           },
  { "begin_frame",        f_begin_frame        },
  { "end_frame",          f_end_frame          },
  { "set_clip_rect",      f_set_clip_rect      },
  { "draw_rect",          f_draw_rect          },
  { "draw_text",          f_draw_text          },
  { "draw_text_subpixel", f_draw_text_subpixel },
  { NULL,                 NULL                 }
};


int luaopen_renderer_font(lua_State *L);
int luaopen_renderer_replacements(lua_State *L);

int luaopen_renderer(lua_State *L) {
  luaL_newlib(L, lib);
  luaopen_renderer_font(L);
  lua_setfield(L, -2, "font");
  luaopen_renderer_replacements(L);
  lua_setfield(L, -2, "replacements");
  return 1;
}
