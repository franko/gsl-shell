#include <fx.h>
#include "GslShellWindow.h"
#include "GslShellApp.h"

int
main (int argc, char *argv[])
{
    GslShellApp app;
    app.init(argc, argv);
    app.create();
    return app.run();
}
