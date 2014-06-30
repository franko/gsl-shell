#ifndef GSL_SHELL_INTERP_H
#define GSL_SHELL_INTERP_H

#include <pthread.h>
#include <lua.h>

#include "defs.h"
#include "luajit.h"
#include "graphics-hooks.h"
#include "str.h"

__BEGIN_DECLS

enum eval_result {
	eval_success     = 0,
	incomplete_input = 1,
};

struct __gsl_shell_interp {
    lua_State *L;
    pthread_mutex_t exec_mutex;
    pthread_mutex_t shutdown_mutex;
    int is_shutting_down;
    str_t m_error_msg;
    const graphics_lib *graphics;
};

typedef struct __gsl_shell_interp gsl_shell_interp;

extern gsl_shell_interp *global_gs;

extern void gsl_shell_interp_init(gsl_shell_interp *gs);
extern void gsl_shell_interp_free(gsl_shell_interp *gs);
extern int gsl_shell_interp_open(gsl_shell_interp *gs, const graphics_lib *g);
extern void gsl_shell_interp_close(gsl_shell_interp *gs);
extern void gsl_shell_interp_close_with_graph(gsl_shell_interp* gs, int send_close_req);
extern int gsl_shell_interp_exec(gsl_shell_interp *gs, const char* line);
extern void gsl_shell_interp_interrupt(gsl_shell_interp *gs);
extern const char *gsl_shell_interp_error_msg(const gsl_shell_interp *gs);
extern int gsl_shell_interp_dostring(gsl_shell_interp *gs, const char *s, const char *name);
extern int gsl_shell_interp_dofile(gsl_shell_interp *gs, const char *name);
extern int gsl_shell_interp_dolibrary(gsl_shell_interp *gs, const char *name);
extern int gsl_shell_interp_doscript(gsl_shell_interp *gs, char **argv, int n);
extern void gsl_shell_interp_lock(gsl_shell_interp *gs);
extern void gsl_shell_interp_unlock(gsl_shell_interp *gs);

__END_DECLS

#endif
