#ifndef FOXGUI_FX_PLOT_WINDOW_H
#define FOXGUI_FX_PLOT_WINDOW_H

#include <fx.h>

#include "fx_plot_canvas.h"

class fx_plot_window : public FXMainWindow {
  FXDECLARE(fx_plot_window)
public:
  fx_plot_window(FXApp* a, const FXString& name, FXIcon *ic=NULL, FXIcon *mi=NULL, FXint w=0, FXint h=0);

  ~fx_plot_window();

  fx_plot_canvas& canvas() { return *m_canvas; }

  int lua_id; // the following is used by Lua to keep trace of the window

protected:
  fx_plot_window() {}

private:
  FXMenuBar* m_menu_bar;
  FXMenuPane* m_plot_menu;
  fx_plot_canvas* m_canvas;
};

#endif
