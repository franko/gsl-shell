#ifndef GSL_SHELL_INCLUDED
#define GSL_SHELL_INCLUDED

#include "defs.h"
#include <pthread.h>

__BEGIN_DECLS

extern void gsl_shell_unref_plot (int id);

extern pthread_mutex_t gsl_shell_mutex[1];

__END_DECLS

#define GSL_SHELL_LOCK() pthread_mutex_lock (gsl_shell_mutex)
#define GSL_SHELL_UNLOCK() pthread_mutex_unlock (gsl_shell_mutex)

#endif
