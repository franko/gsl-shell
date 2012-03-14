#include <fx.h>
#include "fx_console.h"

int
main (int argc, char *argv[])
{
  FXApp app("GSL Shell", "GSL Shell");

  app.init(argc, argv);

  fx_console* c = new fx_console(&app, "GSL Shell Console", NULL, NULL, 600, 800);

  app.create();

  return app.run();
}
