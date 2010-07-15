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

#endif
