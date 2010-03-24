#ifndef GS_TYPES_H
#define GS_TYPES_H

enum gs_type_e {
  GS_MATRIX = 0,
  GS_CMATRIX,
  GS_NLINFIT,
  GS_CNLINFIT,
  GS_ODESOLV,
  GS_CODESOLV,
  GS_HALFCMPL_R2,
  GS_HALFCMPL_MR,
  GS_FDFMULTIMIN,
  GS_FMULTIMIN,
  GS_INVALID_TYPE,
};

/*
extern const char * const GS_MATRIX_NAME;
extern const char * const GS_CMATRIX_NAME;
extern const char * const GS_NLINFIT_NAME;
extern const char * const GS_CNLINFIT_NAME;
extern const char * const GS_ODESOLV_NAME;
extern const char * const GS_CODESOLV_NAME;
extern const char * const GS_HALFCMPL_R2_NAME;
extern const char * const GS_HALFCMPL_MR_NAME;
*/

struct gs_type {
  enum gs_type_e tp;
  const char * mt_name;
  const char * fullname;
};

extern const char * type_qualified_name (int typeid);
extern int gs_type_error (lua_State *L, int narg, const char *req_type);
extern void * gs_check_userdata (lua_State *L, int index, int typeid);

extern const struct gs_type gs_type_table[];

#define GS_METATABLE(id) gs_type_table[(id)].mt_name

#endif
