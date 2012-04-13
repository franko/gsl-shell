#include <fx.h>
#include "gsl_shell_window.h"
#include "gsl_shell_app.h"

int
main (int argc, char *argv[])
{
  gsl_shell_app app;
  app.init(argc, argv);
  app.create();
  return app.run();
}
