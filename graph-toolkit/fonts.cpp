
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "fonts.h"

agg::font_engine_freetype_int32 global_font_eng;
agg::font_cache_manager<agg::font_engine_freetype_int32> global_font_man(global_font_eng);

int initialize_fonts(lua_State* L)
{
    const char* font_name = get_font_name();
    if (!font_name)
        luaL_error(L, "cannot find a suitable truetype font");
    agg::glyph_rendering gren = agg::glyph_ren_outline;
    if (!global_font_eng.load_font(font_name, 0, gren))
        luaL_error(L, "cannot load truetype font: %s", font_name);
    global_font_eng.hinting(true);
    return 0;
}

agg::font_engine_freetype_int32& font_engine()
{
    return global_font_eng;
}

agg::font_cache_manager<agg::font_engine_freetype_int32>& font_manager()
{
    return global_font_man;
}
