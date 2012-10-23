#ifndef FOXGUI_GSL_SHELL_WINDOW_H
#define FOXGUI_GSL_SHELL_WINDOW_H

#include <fx.h>

#include "gsl_shell_thread.h"
#include "fx_console.h"

class gsl_shell_window : public FXMainWindow
{
    FXDECLARE(gsl_shell_window)
public:
    gsl_shell_window(gsl_shell_thread* gs, io_redirect* lua_io, FXApp* a, const FXString& name, FXIcon *ic=NULL, FXIcon *mi=NULL, FXint w=0, FXint h=0);

    virtual ~gsl_shell_window()
    {
        delete m_file_menu;
        delete m_text_font;
    }

    fx_console* console() { return m_text; }

    virtual void create();

    long on_close(FXObject* obj, FXSelector sel, void* ptr);
    long on_change_console(FXObject* obj, FXSelector sel, void* ptr);
    long on_cmd_about(FXObject* obj, FXSelector sel, void* ptr);

    enum
    {
        ID_CONSOLE = FXMainWindow::ID_LAST,
        ID_ABOUT,
    };

protected:
    gsl_shell_window() {}

private:
    fx_console* m_text;
    FXStatusBar* m_status_bar;
    FXMenuBar* m_menu_bar;
    FXMenuPane* m_file_menu;
    FXMenuPane* m_help_menu;
    FXFont* m_text_font;
};

#endif
