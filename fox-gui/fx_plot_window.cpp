
#include "fx_plot_window.h"

FXDEFMAP(fx_plot_window) fx_plot_window_map[]={
};

FXIMPLEMENT(fx_plot_window,FXMainWindow,fx_plot_window_map,ARRAYNUMBER(fx_plot_window_map))

fx_plot_window::fx_plot_window(FXApp* app, const FXString& name, FXIcon *ic, FXIcon *mi, FXint w, FXint h):
  FXMainWindow(app, name, ic, mi, DECOR_ALL, 0, 0, w, h)
{
  m_menu_bar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

  m_plot_menu = new FXMenuPane(this);
  new FXMenuCommand(m_plot_menu, "&Close\tCtl-C", NULL, app, FXApp::ID_QUIT);
  new FXMenuTitle(m_menu_bar, "&Plot", NULL, m_plot_menu);

  FXVerticalFrame* frame = new FXVerticalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  // Sunken border for text widget
  FXHorizontalFrame *cbox = new FXHorizontalFrame(frame, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

  m_canvas = new fx_plot_canvas(cbox, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
}
