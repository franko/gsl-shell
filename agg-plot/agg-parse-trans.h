#ifndef AGG_PARSE_TRANS_H
#define AGG_PARSE_TRANS_H

extern "C" {
#include "lua.h"
}

#include "vertex-source.h"

extern vertex_source * parse_spec (lua_State *L, int specindex, 
				   vertex_source *obj);

extern vertex_source * parse_spec_pipeline (lua_State *L, int index, 
					    vertex_source *obj);


extern vertex_source * build_stroke    (lua_State *L, int i, vertex_source *s);
extern vertex_source * build_curve     (lua_State *L, int i, vertex_source *s);
extern vertex_source * build_marker    (lua_State *L, int i, vertex_source *s);
extern vertex_source * build_dash      (lua_State *L, int i, vertex_source *s);
extern vertex_source * build_translate (lua_State *L, int i, vertex_source *s);
extern vertex_source * build_rotate    (lua_State *L, int i, vertex_source *s);

#endif
