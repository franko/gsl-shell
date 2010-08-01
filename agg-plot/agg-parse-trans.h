#ifndef AGG_PARSE_TRANS_H
#define AGG_PARSE_TRANS_H

extern "C" {
#include "lua.h"
}

#include "scalable.h"
#include "drawable.h"

extern drawable * parse_graph_args (lua_State *L);

#endif
