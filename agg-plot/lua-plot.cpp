
/* lua-plot.cpp
 *
 * Copyright (C) 2009, 2010 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua-plot.h"
#include "lua-plot-cpp.h"
#include "lua-cpp-utils.h"
#include "bitmap-plot.h"
#include "window.h"
#include "window-cpp.h"
#include "gs-types.h"
#include "lua-properties.h"
#include "window_registry.h"
#include "lua-cpp-utils.h"
#include "lua-draw.h"
#include "colors.h"
#include "plot.h"
#include "agg-parse-trans.h"
#include "canvas_svg.h"

__BEGIN_DECLS

static int plot_new        (lua_State *L);
static int plot_add        (lua_State *L);
static int plot_update     (lua_State *L);
static int plot_flush      (lua_State *L);
static int plot_add_line   (lua_State *L);
static int plot_free       (lua_State *L);
static int plot_show       (lua_State *L);
static int plot_title_set  (lua_State *L);
static int plot_title_get  (lua_State *L);
static int plot_xtitle_set   (lua_State *L);
static int plot_xtitle_get   (lua_State *L);
static int plot_ytitle_set   (lua_State *L);
static int plot_ytitle_get   (lua_State *L);
static int plot_units_set  (lua_State *L);
static int plot_units_get  (lua_State *L);
static int plot_set_limits (lua_State *L);
static int plot_push_layer (lua_State *L);
static int plot_pop_layer  (lua_State *L);
static int plot_clear      (lua_State *L);
static int plot_save_svg   (lua_State *L);
static int plot_xlab_angle_set (lua_State *L);
static int plot_xlab_angle_get (lua_State *L);
static int plot_xlab_format (lua_State *L);
static int plot_ylab_format (lua_State *L);
static int plot_ylab_angle_set (lua_State *L);
static int plot_ylab_angle_get (lua_State *L);
static int plot_set_categories (lua_State *L);
static int plot_set_legend     (lua_State *L);
static int plot_get_legend     (lua_State *L);

static int plot_sync_mode_get (lua_State *L);
static int plot_sync_mode_set (lua_State *L);
static int plot_pad_mode_get (lua_State *L);
static int plot_pad_mode_set (lua_State *L);
static int plot_clip_mode_get (lua_State *L);
static int plot_clip_mode_set (lua_State *L);

static int canvas_new      (lua_State *L);

static int   plot_add_gener  (lua_State *L, bool as_line);
static void  plot_update_raw (lua_State *L, sg_plot *p, int plot_index);

static const struct luaL_Reg plot_functions[] = {
  {"plot",        plot_new},
  {"canvas",      canvas_new},
  {NULL, NULL}
};

static const struct luaL_Reg plot_metatable[] = {
  {"__gc",        plot_free       },
  {NULL, NULL}
};

static const struct luaL_Reg plot_methods[] = {
  {"add",         plot_add        },
  {"addline",     plot_add_line   },
  {"update",      plot_update     },
  {"flush",       plot_flush      },
  {"show",        plot_show       },
  {"limits",      plot_set_limits },
  {"pushlayer",   plot_push_layer },
  {"poplayer",    plot_pop_layer  },
  {"clear",       plot_clear      },
  {"save",        bitmap_save_image },
  {"save_svg",    plot_save_svg   },
  {"set_categories", plot_set_categories},
  {"set_legend",  plot_set_legend},
  {"get_legend",  plot_get_legend},
  {NULL, NULL}
};

static const struct luaL_Reg plot_properties_get[] = {
  {"title",        plot_title_get  },
  {"xtitle",       plot_xtitle_get  },
  {"ytitle",       plot_ytitle_get  },
  {"xlab_angle",   plot_xlab_angle_get  },
  {"ylab_angle",   plot_ylab_angle_get  },
  {"units",        plot_units_get  },
  {"sync",         plot_sync_mode_get  },
  {"pad",          plot_pad_mode_get  },
  {"clip",         plot_clip_mode_get },
  {NULL, NULL}
};

static const struct luaL_Reg plot_properties_set[] = {
  {"title",        plot_title_set  },
  {"xtitle",       plot_xtitle_set  },
  {"ytitle",       plot_ytitle_set  },
  {"xlab_angle",   plot_xlab_angle_set  },
  {"ylab_angle",   plot_ylab_angle_set  },
  {"xlab_format",  plot_xlab_format  },
  {"ylab_format",  plot_ylab_format  },
  {"units",        plot_units_set  },
  {"sync",         plot_sync_mode_set  },
  {"pad",          plot_pad_mode_set  },
  {"clip",         plot_clip_mode_set },
  {NULL, NULL}
};

__END_DECLS

static void
configure_plot_env(lua_State* L, int index)
{
  INDEX_SET_ABS(L, index);
  lua_newtable(L); /* create a new table for the environment */

  lua_pushstring(L, "__legend");
  lua_newtable(L);
  lua_rawset(L, -3); /* set the field __legend to a new table in the env */

  lua_setfenv (L, index);
}

int
plot_new (lua_State *L)
{
  sg_plot *p = push_new_object<sg_plot_auto>(L, GS_PLOT);

  configure_plot_env(L, -1);

  if (lua_isstring (L, 1))
    {
      const char *title = lua_tostring (L, 1);
      if (title)
        p->title() = title;
    }

  return 1;
}

int
canvas_new (lua_State *L)
{
  sg_plot *p = push_new_object<sg_plot>(L, GS_PLOT);

  configure_plot_env(L, -1);

  p->sync_mode(false);

  if (lua_isstring (L, 1))
    {
      const char *title = lua_tostring (L, 1);
      if (title)
        p->title() = title;
    }

  return 1;
}

int
plot_free (lua_State *L)
{
  return object_free<sg_plot>(L, 1, GS_PLOT);
}

void
plot_add_gener_cpp (lua_State *L, sg_plot* p, bool as_line,
                    gslshell::ret_status& st)
{
  agg::rgba8 color;
  int layer_index = p->current_layer_index();

  sg_object* obj = parse_graph_args(L, color, st, layer_index);

  if (!obj) return;

  AGG_LOCK();
  p->add(obj, color, as_line);
  AGG_UNLOCK();

  if (p->sync_mode())
    plot_flush (L);
}

static void
objref_mref_add (lua_State *L, int table_index, int index, int value_index)
{
  int n;
  INDEX_SET_ABS(L, table_index);
  INDEX_SET_ABS(L, value_index);

  lua_rawgeti (L, table_index, index);
  if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      lua_newtable (L);
      lua_pushvalue (L, -1);
      lua_rawseti (L, table_index, index);
      n = 0;
    }
  else
    {
      n = lua_objlen (L, -1);
    }

  lua_pushvalue (L, value_index);
  lua_rawseti (L, -2, n+1);
  lua_pop (L, 1);
}


void
plot_lua_add_ref (lua_State* L, int plot_index, int ref_index)
{
  lua_getfenv (L, plot_index);
  objref_mref_add (L, -1, ref_index, -2);
  lua_pop (L, 1);
}

int
plot_add_gener (lua_State *L, bool as_line)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);

  gslshell::ret_status st;
  plot_add_gener_cpp (L, p, as_line, st);

  if (st.error_msg())
    return luaL_error (L, "%s in %s", st.error_msg(), st.context());

  lua_getfenv (L, 1);
  objref_mref_add (L, -1, p->current_layer_index(), 2);

  return 0;
}

int
plot_add (lua_State *L)
{
  return plot_add_gener (L, false);
}

int
plot_add_line (lua_State *L)
{
  return plot_add_gener (L, true);
}

static int plot_string_property_get (lua_State* L, str& (sg_plot::*getref)())
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);

  AGG_LOCK();
  str& ref = (p->*getref)();
  lua_pushstring (L, ref.cstr());
  AGG_UNLOCK();
  return 1;
}

static void plot_string_property_set (lua_State* L, str& (sg_plot::*getref)(), bool update)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);
  const char *s = lua_tostring (L, 2);

  if (s == NULL)
    gs_type_error (L, 2, "string");

  AGG_LOCK();
  (p->*getref)() = s;
  AGG_UNLOCK();

  if (update)
    plot_update_raw (L, p, 1);
}

static int plot_bool_property_get(lua_State* L, bool (sg_plot::*getter)() const)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);
  AGG_LOCK();
  bool r = (p->*getter)();
  lua_pushboolean(L, (int)r);
  AGG_UNLOCK();
  return 1;
}

static void plot_bool_property_set(lua_State* L, void (sg_plot::*setter)(bool), bool update)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);

  if (!lua_isboolean(L, 2))
    gs_type_error (L, 2, "boolean");

  bool request = (bool) lua_toboolean (L, 2);

  AGG_LOCK();
  (p->*setter)(request);
  AGG_UNLOCK();

  if (update)
    plot_update_raw (L, p, 1);
}

int
plot_title_set (lua_State *L)
{
  plot_string_property_set(L, &sg_plot::title, true);
  return 0;
}

int
plot_title_get (lua_State *L)
{
  return plot_string_property_get(L, &sg_plot::title);
}

static int
plot_axis_label_angle_set (lua_State *L, sg_plot::axis_e axis)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);
  double angle = luaL_checknumber(L, 2);

  AGG_LOCK();
  p->set_axis_labels_angle(axis, angle);
  AGG_UNLOCK();

  plot_update_raw (L, p, 1);
  return 0;
}

static int
plot_axis_label_angle_get (lua_State *L, sg_plot::axis_e axis)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);

  AGG_LOCK();
  double angle = p->get_axis_labels_angle(axis);
  AGG_UNLOCK();

  lua_pushnumber(L, angle);
  return 1;
}

static int
plot_axis_label_format (lua_State *L, sg_plot::axis_e axis)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);
  const char* fmt = luaL_optstring(L, 2, NULL);

  AGG_LOCK();
  bool success = p->enable_label_format(axis, fmt);
  AGG_UNLOCK();

  if (success)
    plot_update_raw (L, p, 1);
  else
    luaL_error(L, "got invalid label format: %s", fmt); 

  return 0;
}

int
plot_xlab_format (lua_State* L)
{
  return plot_axis_label_format(L, sg_plot::x_axis);
}

int
plot_ylab_format (lua_State* L)
{
  return plot_axis_label_format(L, sg_plot::y_axis);
}

int
plot_xlab_angle_set(lua_State *L)
{
  return plot_axis_label_angle_set(L, sg_plot::x_axis);
}

int
plot_xlab_angle_get(lua_State *L)
{
  return plot_axis_label_angle_get(L, sg_plot::x_axis);
}

int
plot_ylab_angle_set(lua_State *L)
{
  return plot_axis_label_angle_set(L, sg_plot::y_axis);
}

int
plot_ylab_angle_get(lua_State *L)
{
  return plot_axis_label_angle_get(L, sg_plot::y_axis);
}

int
plot_xtitle_set (lua_State *L)
{
  plot_string_property_set(L, &sg_plot::x_axis_title, true);
  return 0;
}

int
plot_xtitle_get (lua_State *L)
{
  return plot_string_property_get(L, &sg_plot::x_axis_title);
}

int
plot_ytitle_set (lua_State *L)
{
  plot_string_property_set(L, &sg_plot::y_axis_title, true);
  return 0;
}

int
plot_ytitle_get (lua_State *L)
{
  return plot_string_property_get(L, &sg_plot::y_axis_title);
}

int
plot_units_set (lua_State *L)
{
  plot_bool_property_set(L, &sg_plot::set_units, true);
  return 0;
}

int
plot_units_get (lua_State *L)
{
  return plot_bool_property_get(L, &sg_plot::use_units);
}

void
plot_update_raw (lua_State *L, sg_plot *p, int plot_index)
{
  window_refs_lookup_apply (L, plot_index, window_slot_update);
  p->commit_pending_draw();
}

int
plot_update (lua_State *L)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);
  plot_update_raw (L, p, 1);
  return 0;
}

int
plot_flush (lua_State *L)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);
  window_refs_lookup_apply (L, 1, window_slot_refresh);
  p->commit_pending_draw();
  return 0;
}

int
plot_show (lua_State *L)
{
  sg_plot* plot = object_check<sg_plot>(L, 1, GS_PLOT);
  window* win = push_new_object<window>(L, GS_WINDOW, colors::white);

  int slot_id = win->attach (plot, "");
  assert(slot_id >= 0);
  window_refs_add (L, slot_id, -1, 1);

  gslshell::ret_status st;
  win->start(L, st);

  if (st.error_msg())
    return luaL_error (L, "%s (reported during %s)", st.error_msg(), st.context());

  return 0;
}

int
plot_set_limits (lua_State *L)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);

  agg::rect_base<double> r;
  r.x1 = gs_check_number (L, 2, true);
  r.y1 = gs_check_number (L, 3, true);
  r.x2 = gs_check_number (L, 4, true);
  r.y2 = gs_check_number (L, 5, true);

  AGG_LOCK();
  p->set_limits(r);
  AGG_UNLOCK();
  plot_update_raw (L, p, 1);
  return 0;
}

int
plot_push_layer (lua_State *L)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);

  window_refs_lookup_apply (L, 1, window_slot_refresh);

  AGG_LOCK();
  p->push_layer();
  AGG_UNLOCK();

  window_refs_lookup_apply (L, 1, window_save_slot_image);

  return 0;
}

static void
plot_ref_clear (lua_State *L, int index, int layer_id)
{
  lua_getfenv (L, index);
  lua_newtable (L);
  lua_rawseti (L, -2, layer_id);
  lua_pop (L, 1);
 }

int
plot_pop_layer (lua_State *L)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);

  plot_ref_clear (L, 1, p->current_layer_index());

  AGG_LOCK();
  p->pop_layer();
  AGG_UNLOCK();

  plot_update_raw (L, p, 1);
  return 0;
}

int
plot_clear (lua_State *L)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);

  plot_ref_clear (L, 1, p->current_layer_index());

  AGG_LOCK();
  p->clear_current_layer();
  AGG_UNLOCK();

  window_refs_lookup_apply (L, 1, window_restore_slot_image);

  if (p->sync_mode())
    plot_update_raw (L, p, 1);

  return 0;
}

int
plot_save_svg (lua_State *L)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);
  const char *filename = lua_tostring(L, 2);
  double w = luaL_optnumber(L, 3, 800.0);
  double h = luaL_optnumber(L, 4, 600.0);

  if (!filename)
    return gs_type_error(L, 2, "string");

  FILE* f = fopen(filename, "w");
  if (!f)
    return luaL_error(L, "cannot open filename: %s", filename);

  canvas_svg canvas(f, h);
  agg::trans_affine_scaling m(w, h);
  canvas.write_header(w, h);
  p->draw(canvas, m);
  canvas.write_end();
  fclose(f);

  return 0;
}

static int plot_pad_mode_set (lua_State *L)
{
  plot_bool_property_set(L, &sg_plot::pad_mode, true);
  return 0;
}

static int plot_pad_mode_get (lua_State *L)
{
  return plot_bool_property_get(L, &sg_plot::pad_mode);
}

static int plot_clip_mode_set (lua_State *L)
{
  plot_bool_property_set(L, &sg_plot::set_clip_mode, true);
  return 0;
}

static int plot_clip_mode_get (lua_State *L)
{
  return plot_bool_property_get(L, &sg_plot::clip_is_active);
}

int
plot_sync_mode_get (lua_State *L)
{
  return plot_bool_property_get(L, &sg_plot::sync_mode);
}

int
plot_sync_mode_set (lua_State *L)
{
  plot_bool_property_set(L, &sg_plot::sync_mode, false);
  return 0;
}

int
plot_set_categories (lua_State *L)
{
  sg_plot *p = object_check<sg_plot>(L, 1, GS_PLOT);
  const char* axis_s = luaL_checkstring(L, 2);
  sg_plot::axis_e dir;

  if (strcmp(axis_s, "x") == 0)
    dir = sg_plot::x_axis;
  else if (strcmp(axis_s, "y") == 0)
    dir = sg_plot::y_axis;
  else
    return luaL_error(L, "axis argument should be \"x\" or \"y\"");

  AGG_LOCK();

  if (lua_isnoneornil(L, 3))
    {
      p->disable_categories(dir);
    }
  else
    {
      int k, n;

      if (!lua_istable(L, 3))
        {
          AGG_UNLOCK();
          return luaL_error(L, "invalid categories, should be a table or nil");
        }

      p->enable_categories(dir);

      n = lua_objlen(L, 3);
      for (k = 1; k+1 <= n; k += 2)
        {
          lua_rawgeti(L, 3, k);
          lua_rawgeti(L, 3, k+1);
          if (lua_isnumber(L, -2) && lua_isstring(L, -1))
            {
              double v = lua_tonumber(L, -2);
              const char* s = lua_tostring(L, -1);
              p->add_category_entry(dir, v, s);
            }
          lua_pop(L, 2);
        }
    }

  AGG_UNLOCK();

  plot_update_raw (L, p, 1);

  return 0;
}

static sg_plot::placement_e
char_to_placement_enum(lua_State* L, const char *s)
{
  sg_plot::placement_e pos;

  char letter = s[0];
  if (letter == 'r')
    pos = sg_plot::right;
  else if (letter == 'l')
    pos = sg_plot::left;
  else if (letter == 'b')
    pos = sg_plot::bottom;
  else if (letter == 't')
    pos = sg_plot::top;
  else
    luaL_error (L, "invalid legend placement specification.");

  return pos;
}

static void set_legend_ref(lua_State* L, sg_plot::placement_e pos,
			   int plot_index, int legend_index)
{
  INDEX_SET_ABS_2(L, plot_index, legend_index);
  lua_getfenv(L, plot_index); /* env = getfenv(plot) */
  lua_pushstring(L, "__legend");
  lua_rawget(L, -2); /* leg_table = env.__legend */
  lua_pushvalue(L, legend_index);
  lua_rawseti(L, -2, pos);  /* leg_table[pos] = legend */
  lua_pop(L, 1); /* pop environment table */
}

int
plot_set_legend(lua_State *L)
{
  sg_plot* p = object_check<sg_plot>(L, 1, GS_PLOT);
  sg_plot* mp = object_check<sg_plot>(L, 2, GS_PLOT);
  const char* placement = luaL_optstring(L, 3, "r");
  sg_plot::placement_e pos = char_to_placement_enum(L, placement);

  set_legend_ref(L, pos, 1, 2);

  AGG_LOCK();
  p->add_legend(mp, pos);
  AGG_UNLOCK();

  plot_update_raw (L, p, 1);

  return 0;
}

int
plot_get_legend(lua_State *L)
{
  object_check<sg_plot>(L, 1, GS_PLOT);
  const char* placement = luaL_optstring(L, 2, "r");
  sg_plot::placement_e pos = char_to_placement_enum(L, placement);

  lua_getfenv(L, 1); /* env = getfenv(plot) */
  lua_pushstring(L, "__legend");
  lua_rawget(L, -2); /* leg_table = env.__legend */
  lua_rawgeti(L, -1, pos);  /* push leg_table[pos] */

  return 1;
}

void
plot_register (lua_State *L)
{
  /* plot declaration */
  luaL_newmetatable (L, GS_METATABLE(GS_PLOT));
  register_properties_index(L, plot_methods, plot_properties_get, plot_properties_set);
  luaL_register (L, NULL, plot_metatable);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plot_functions);
}
