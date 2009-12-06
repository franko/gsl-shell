
#include <pthread.h>

#include "defs.h"
#include "agg-cplot.h"

__BEGIN_DECLS

struct lcplot {
  cplot *plot;
  pthread_mutex_t mutex[1];
  int lua_is_owner;
  int is_shown;
  void *window;
};

extern void lcplot_destroy (struct lcplot *cp);
extern void update_callback (void *_app);

__END_DECLS
