#ifndef REFS_H
#define REFS_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

extern void register_ref_table (lua_State *L, const char *table_name);

__END_DECLS

#endif
