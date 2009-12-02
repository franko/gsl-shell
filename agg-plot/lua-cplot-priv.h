
#include <pthread.h>

#include "defs.h"
#include "agg-cplot.h"

__BEGIN_DECLS

struct lcplot {
  cplot *plot;
  int lua_is_owner;
  int is_shown;
  pthread_mutex_t *mutex;
  void *x_app;
};

extern void lcplot_destroy (struct lcplot *cp);
extern void update_callback (void *_app);

__END_DECLS
