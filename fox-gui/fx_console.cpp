
#include "fx_console.h"

// Message Map for the Scribble Window class
FXDEFMAP(fx_console) fx_console_map[]={

  //________Message_Type_____________________ID____________Message_Handler_______
};

FXIMPLEMENT(fx_console,FXMainWindow,fx_console_map,ARRAYNUMBER(fx_console_map))

fx_console::fx_console(FXApp* app, const FXString& name, FXIcon *ic, FXIcon *mi, FXint w, FXint h):
  FXMainWindow(app, name, ic, mi, DECOR_ALL, 0, 0, w, h)
{
  m_menu_bar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  m_status_bar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

  m_file_menu = new FXMenuPane(this);
  new FXMenuCommand(m_file_menu, "&Quit\tCtl-Q", NULL, app, FXApp::ID_QUIT);
  new FXMenuTitle(m_menu_bar, "&File", NULL, m_file_menu);

  FXVerticalFrame* frame = new FXVerticalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  m_text_font = new FXFont(app, "lucida console", 10);
  m_text = new FXText(frame, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED);
  m_text->setFont(m_text_font);
}

void fx_console::create()
{
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
}
