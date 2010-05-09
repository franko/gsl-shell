#ifndef GS_TYPES_H
#define GS_TYPES_H

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
  GS_PLOT,
  GS_DRAW_OBJ,
  GS_RGBA_COLOR,
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
};

extern const char * full_type_name (lua_State *L, int narg);
extern const char * type_qualified_name (int typeid);
extern int          gs_type_error (lua_State *L, int narg, const char *req_type);
extern void *       gs_check_userdata (lua_State *L, int index, int typeid);
extern int          gs_gsl_errorcheck (lua_State *L, const char *routine, int status);
extern void         gs_set_metatable (lua_State *L, int typeid);
extern double       gs_check_number (lua_State *L, int index, int check_normal);

extern const struct gs_type gs_type_table[];

#define GS_METATABLE(id) gs_type_table[(id)].mt_name

extern const struct luaL_Reg gs_type_functions[];

#endif
