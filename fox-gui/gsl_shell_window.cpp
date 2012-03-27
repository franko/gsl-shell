#include "gsl_shell_window.h"

#ifdef WIN32
#define CONSOLE_FONT "lucida console"
#else
#define CONSOLE_FONT "monospace"
#endif

FXDEFMAP(gsl_shell_window) gsl_shell_window_map[]={
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

  // Sunken border for text widget
  FXHorizontalFrame *textbox = new FXHorizontalFrame(frame, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

  m_text_font = new FXFont(app, CONSOLE_FONT, 10);
  m_text = new fx_console(textbox, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
  m_text->setFont(m_text_font);
}

void gsl_shell_window::create()
{
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
}
