#ifdef _WIN32
  #include <windows.h>
#endif

#include <fx.h>
#include "gsl_shell_window.h"
#include "gsl_shell_app.h"
#include "window_hooks.h"
#include "lua_plot_window.h"

struct window_hooks app_window_hooks[1] = {{
        fox_window_new, fox_window_show, fox_window_attach,
        fox_window_slot_update, fox_window_slot_refresh,
        fox_window_close, fox_window_close,
        fox_window_save_slot_image, fox_window_restore_slot_image,
        fox_window_register,
    }
};

int
main (int argc, char *argv[])
{
#if defined(_WIN32)
    HINSTANCE lib = LoadLibrary("user32.dll");
    int (*SetProcessDPIAware)() = (int (*)()) GetProcAddress(lib, "SetProcessDPIAware");
    SetProcessDPIAware();
#endif
    gsl_shell_app app;
    app.init(argc, argv);
    app.create();
    return app.run();
}
