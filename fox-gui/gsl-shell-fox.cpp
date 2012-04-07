#include <fx.h>
#include "gsl_shell_window.h"
#include "gsl_shell_app.h"

int
main (int argc, char *argv[])
{
  gsl_shell_app app;
  app.init(argc, argv);
  new gsl_shell_window(&app, "GSL Shell Console", NULL, NULL, 600, 500);
  app.create();
  return app.run();
}
