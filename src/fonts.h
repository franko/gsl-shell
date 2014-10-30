#ifndef FONTS_H
#define FONTS_H

#include "lua.h"
#include "agg_font_freetype.h"

extern "C" int initialize_fonts(lua_State* L);

extern const char *get_font_name();
extern const char *get_console_font_name();

extern agg::font_engine_freetype_int32& font_engine();
extern agg::font_cache_manager<agg::font_engine_freetype_int32>& font_manager();

#endif
