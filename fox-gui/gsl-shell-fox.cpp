#include <fx.h>
#include "gsl_shell_window.h"
#include "gsl_shell_app.h"
#include "window_hooks.h"
#include "window.h"

struct window_hooks app_window_hooks[1] = {{
    window_new, window_attach,
    window_slot_update, window_slot_refresh,
    window_save_slot_image, window_restore_slot_image,
  }
};

int
main (int argc, char *argv[])
{
  gsl_shell_app app;
  app.init(argc, argv);
  app.create();
  return app.run();
}
