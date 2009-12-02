#ifndef CPLOT_CINTFC_H
#define CPLOT_CINTFC_H

#include "defs.h"

#define CCPLOT struct _cplot
#define CLINE  struct _line

__BEGIN_DECLS

CCPLOT;
CLINE;

extern CCPLOT * cplot_new(int with_units);
extern void cplot_add  (CCPLOT *p, CLINE *d);
extern void cplot_free (CCPLOT *p);

extern CLINE *line_new  (const char *color_str);
extern CLINE *line_copy (CLINE *d);
extern CLINE *poly_new(const char *color_str, const char *outline_color);
// extern CLINE *dashed_new(const char *color_str, const char *outline_color);

extern void line_free    (CLINE *d);
extern void line_move_to (CLINE *d, double x, double y);
extern void line_line_to (CLINE *d, double x, double y);  
extern void line_close   (CLINE *d);

__END_DECLS

#ifndef __cplusplus
typedef CCPLOT cplot;
typedef CLINE line;
#endif

#endif
