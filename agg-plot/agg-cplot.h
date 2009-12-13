#ifndef CPLOT_CINTFC_H
#define CPLOT_CINTFC_H

#include "defs.h"

#define CCPLOT struct _cplot
#define CPATH  struct _cpath
#define CDRAW  struct _cdraw

__BEGIN_DECLS

struct cmd_call_stack;

CCPLOT;
CPATH;
CDRAW;

extern CCPLOT * cplot_new  (int with_units);
extern void     cplot_add  (CCPLOT *p, CDRAW *d);
extern void     cplot_free (CCPLOT *p);

extern CPATH *  poly_new  (const char *color_str, const char *outline_color);

extern CPATH *  path_new  (const char *color_str);
extern CPATH *  path_copy (CPATH *p);
extern void     path_free (CPATH *p);
extern void     path_cmd  (CPATH *p, struct cmd_call_stack *stack);

extern CDRAW *  ellipse_new  (double x, double y, double rx, double ry);
extern void     ellipse_free (CDRAW *e);

extern CDRAW *  drawable_copy (CDRAW *d);

__END_DECLS

#ifndef __cplusplus
typedef CCPLOT cplot;
typedef CPATH path;
typedef CDRAW drawable;
#endif

#endif
