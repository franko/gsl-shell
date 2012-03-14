#ifndef FOXGUI_FX_CONSOLE_H
#define FOXGUI_FX_CONSOLE_H

#include <fx.h>

class fx_console : public FXMainWindow {
  FXDECLARE(fx_console)
public:
  fx_console(FXApp* a, const FXString& name, FXIcon *ic=NULL, FXIcon *mi=NULL, FXint w=0, FXint h=0);

  virtual ~fx_console()
  {
    delete m_file_menu;
    delete m_text_font;
  }

  virtual void create();

protected:
  fx_console() {}

  fx_console(const fx_console&);
  fx_console& operator=(const fx_console&);

private:
  FXText* m_text;
  FXStatusBar* m_status_bar;
  FXMenuBar* m_menu_bar;
  FXMenuPane* m_file_menu;
  FXFont* m_text_font;
};

#endif
