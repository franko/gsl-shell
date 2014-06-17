#ifndef GSL_SHELL_INTERP_H
#define GSL_SHELL_INTERP_H

#include <pthread.h>
#include <lua.h>

#include "defs.h"
#include "graphics-hooks.h"

__BEGIN_DECLS

enum eval_result {
	eval_success,
	eval_error,
	incomplete_input
};

struct __gsl_shell_interp {
    lua_State *L;
    pthread_mutex_t exec_mutex;
    pthread_mutex_t shutdown_mutex;
    int is_shutting_down;

    str_t m_error_msg;
};

typedef struct __gsl_shell_interp gsl_shell_interp;

extern void gsl_shell_interp_open(gsl_shell_interp *gs, graphics_lib *g);
extern void gsl_shell_interp_close(gsl_shell_interp *gs);
extern void gsl_shell_interp_close_with_graph(gsl_shell_interp* gs, int send_close_req);
extern int gsl_shell_interp_exec(gsl_shell_interp *gs, const char* line);
extern void gsl_shell_interp_interrupt(gsl_shell_interp *gs);
extern const char *gsl_shell_interp_error_msg(gsl_shell_interp *gs);

__END_DECLS

#endif
