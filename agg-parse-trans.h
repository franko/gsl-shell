#ifndef AGG_PARSE_TRANS_H
#define AGG_PARSE_TRANS_H

extern "C" {
#include "lua.h"
}

#include "lua-cpp-utils.h"
#include "sg_object.h"
#include "agg_color_rgba.h"

extern sg_object* parse_graph_args (lua_State *L, agg::rgba8& color,
                                    gslshell::ret_status& st, int layer_index);

#endif
