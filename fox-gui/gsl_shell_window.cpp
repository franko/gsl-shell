#include "gsl_shell_window.h"

FXDEFMAP(gsl_shell_window) gsl_shell_window_map[]={
  FXMAPFUNC(SEL_IO_READ, gsl_shell_window::ID_SOCKET, gsl_shell_window::on_socket_input),
};

FXIMPLEMENT(gsl_shell_window,FXMainWindow,gsl_shell_window_map,ARRAYNUMBER(gsl_shell_window_map))

gsl_shell_window::gsl_shell_window(FXApp* app, const FXString& name, FXIcon *ic, FXIcon *mi, FXint w, FXint h):
  FXMainWindow(app, name, ic, mi, DECOR_ALL, 0, 0, w, h)
{
  m_menu_bar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  m_status_bar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

  m_file_menu = new FXMenuPane(this);
  new FXMenuCommand(m_file_menu, "&Quit\tCtl-Q", NULL, app, FXApp::ID_QUIT);
  new FXMenuTitle(m_menu_bar, "&File", NULL, m_file_menu);

  FXVerticalFrame* frame = new FXVerticalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  m_text_font = new FXFont(app, "lucida console", 10);
  m_text = new fx_console(frame, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED);
  m_text->setFont(m_text_font);
}

long gsl_shell_window::on_socket_input(FXObject*,FXSelector,void* _socket)
{
  return 0;
  //  FXInputHandle* f = (FXInputHandle)
}

void gsl_shell_window::create()
{
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
}
