#ifndef GS_TYPES_H
#define GS_TYPES_H

#include "defs.h"

__BEGIN_DECLS

#include <lua.h>
#include <lauxlib.h>

enum gs_type_e {
  GS_MATRIX = 0,
  GS_CMATRIX,
  GS_RNG,
  GS_NLINFIT,
  GS_CNLINFIT,
  GS_ODESOLV,
  GS_CODESOLV,
  GS_HALFCMPL_R2,
  GS_HALFCMPL_MR,
  GS_FDFMULTIMIN,
  GS_FMULTIMIN,
  GS_BSPLINE,
#ifdef AGG_PLOT_ENABLED
  GS_DRAW_PLOT,
  GS_DRAW_PATH,
  GS_DRAW_TEXT,
  GS_RGBA_COLOR,
  GS_PLOT_WINDOW,
  GS_CANVAS_WINDOW,
#endif
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
  const struct gs_type *derived_class;
};

extern const char * full_type_name (lua_State *L, int narg);
extern const char * type_qualified_name (int type_id);
extern int          gs_type_error (lua_State *L, int narg, const char *req_type);
extern void *       gs_is_userdata    (lua_State *L, int index, int type_id);
extern void *       gs_check_userdata (lua_State *L, int index, int type_id);
extern void *       gs_check_userdata_w_alt (lua_State *L, int index, 
					     int typeid1, int typeid2, 
					     int *sel);
extern int          gs_gsl_errorcheck (lua_State *L, const char *routine, int status);
extern void         gs_set_metatable (lua_State *L, int type_id);
extern double       gs_check_number (lua_State *L, int index, int check_normal);

extern const struct gs_type gs_type_table[];

#define GS_METATABLE(id) gs_type_table[(id)].mt_name

extern const struct luaL_Reg gs_type_functions[];

__END_DECLS

#endif
