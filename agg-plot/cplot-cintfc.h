#ifndef CPLOT_CINTFC_H
#define CPLOT_CINTFC_H

#include "defs.h"

#define CCPLOT struct _cplot
#define CDRAW  struct _cdrawable

__BEGIN_DECLS

CCPLOT;
CDRAW;

extern CCPLOT * cplot_new(int with_units);
extern void cplot_add  (CCPLOT *p, CDRAW *d);
extern void cplot_free (CCPLOT *p);

extern CDRAW *line_new  (const char *color_str);
extern CDRAW *line_copy (CDRAW *d);
// extern CDRAW *polygon_new(const char *color_str, const char *outline_color);
// extern CDRAW *dashed_new(const char *color_str, const char *outline_color);

extern void line_free    (CDRAW *d);
extern void line_move_to (CDRAW *d, double x, double y);
extern void line_line_to (CDRAW *d, double x, double y);  
extern void line_close   (CDRAW *d);

__END_DECLS

#ifndef __cplusplus
typedef CCPLOT cplot;
typedef CDRAW line;
#endif

#endif
