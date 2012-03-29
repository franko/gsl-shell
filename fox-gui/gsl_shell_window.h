#ifndef FOXGUI_GSL_SHELL_WINDOW_H
#define FOXGUI_GSL_SHELL_WINDOW_H

#include <fx.h>

#include "fx_console.h"

class gsl_shell_window : public FXMainWindow {
  FXDECLARE(gsl_shell_window)
public:
  gsl_shell_window(FXApp* a, const FXString& name, FXIcon *ic=NULL, FXIcon *mi=NULL, FXint w=0, FXint h=0);

  virtual ~gsl_shell_window()
  {
    delete m_file_menu;
    delete m_text_font;
  }

  virtual void create();

  long on_close(FXObject* obj, FXSelector sel, void* ptr);

protected:
  gsl_shell_window() {}

private:
  fx_console* m_text;
  FXStatusBar* m_status_bar;
  FXMenuBar* m_menu_bar;
  FXMenuPane* m_file_menu;
  FXFont* m_text_font;
};

#endif
