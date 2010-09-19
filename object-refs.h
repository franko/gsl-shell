#ifndef OBJECT_REFS_H
#define OBJECT_REFS_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

enum object_table_e { table_window_plot = 0, table_plot_obj = 1 };

extern void object_refs_prepare (lua_State *L);

extern void object_refs_add (lua_State *L, int table, int index, 
			     int key_index, int value_index);

extern void object_refs_remove (lua_State *L, int table, int index, int key_index);

extern void object_refs_lookup_apply (lua_State *L, int table, int value_index, 
				      lua_CFunction func);

__END_DECLS

#endif
