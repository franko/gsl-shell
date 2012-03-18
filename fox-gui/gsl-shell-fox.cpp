#include <fx.h>
#include "gsl_shell_window.h"

int
main (int argc, char *argv[])
{
  FXApp app("GSL Shell", "GSL Shell");
  app.init(argc, argv);
  new gsl_shell_window(&app, "GSL Shell Console", NULL, NULL, 600, 500);
  app.create();
  return app.run();
}
