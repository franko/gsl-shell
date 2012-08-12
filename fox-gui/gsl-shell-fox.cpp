#include <fx.h>
#include "gsl_shell_window.h"
#include "gsl_shell_app.h"
#include "window_hooks.h"
#include "lua_plot_window.h"

struct window_hooks app_window_hooks[1] = {{
        fox_window_new, fox_window_attach,
        fox_window_slot_update, fox_window_slot_refresh,
        fox_window_close,
        fox_window_save_slot_image, fox_window_restore_slot_image,
        fox_window_register,
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
