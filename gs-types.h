#ifndef GS_TYPES_H
#define GS_TYPES_H

#include "defs.h"

__BEGIN_DECLS

#include <lua.h>
#include <lauxlib.h>

enum gs_type_e {
  GS_NO_TYPE = -1,
  GS_WINDOW = 0,
  GS_DRAW_SCALABLE, /* derived types are declared only after their base class */
  GS_DRAW_PATH,
  GS_DRAW_ELLIPSE,
  GS_DRAW_DRAWABLE,
  GS_DRAW_TEXT,
  GS_DRAW_TEXTSHAPE,
  GS_DRAW_MARKER,
  GS_PLOT,
  GS_INVALID_TYPE,
};

enum fp_check_e {
  FP_NO_CHECK = 0,
  FP_CHECK_NORMAL = 1
};

struct gs_type {
  enum gs_type_e tp;
  const char * mt_name;
  const char * fullname;
  enum gs_type_e base_type;
};

extern const char * full_type_name (lua_State *L, int narg);
extern const char * type_qualified_name (int type_id);
extern int          gs_type_error (lua_State *L, int narg, const char *req_type);
extern void *       gs_new_object (size_t nbytes, lua_State *L, enum gs_type_e tp);
extern void *       gs_is_userdata    (lua_State *L, int index, int type_id);
extern void *       gs_check_userdata (lua_State *L, int index, int type_id);
extern void         gs_set_metatable (lua_State *L, int type_id);
extern double       gs_check_number (lua_State *L, int index, int check_normal);

extern const struct gs_type gs_type_table[];

#define GS_METATABLE(id) gs_type_table[(id)].mt_name

extern int gs_type_string (lua_State *L);

__END_DECLS

#endif
