#include "GslShellWindow.h"
#include "GslShellApp.h"
#include "luajit.h"
#include "icons.h"

#ifdef WIN32
static const char* console_font = "Consolas";
#else
static const char* console_font = "Monospace";
#endif

FXDEFMAP(GslShellWindow) GslShellWindow_map[]=
{
    FXMAPFUNC(SEL_CLOSE, 0, GslShellWindow::on_close),
    FXMAPFUNC(SEL_COMMAND, FXTopWindow::ID_CLOSE, GslShellWindow::on_close),
    FXMAPFUNC(SEL_COMMAND, GslShellWindow::ID_ABOUT, GslShellWindow::on_cmd_about),
    FXMAPFUNC(SEL_COMMAND, LuaConsole::ID_SCROLL_CONTENT, GslShellWindow::on_cmd_scroll_content),
};

FXIMPLEMENT(GslShellWindow,FXMainWindow,GslShellWindow_map,ARRAYNUMBER(GslShellWindow_map))

GslShellWindow::GslShellWindow(gsl_shell_thread* gs, io_redirect* lua_io, FXApp* app, const FXString& name, FXIcon *ic, FXIcon *mi, FXint w, FXint h):
    FXMainWindow(app, name, ic, mi, DECOR_ALL, 0, 0, w, h)
{
    m_menu_bar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
    m_status_bar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

    m_file_menu = new FXMenuPane(this);
    new FXMenuCommand(m_file_menu, "&Restart\tCtl-R", NULL, app, GslShellApp::ID_LUA_RESTART);
    new FXMenuCommand(m_file_menu, "&Interrupt\tCtl-I", NULL, app, GslShellApp::ID_LUA_INTERRUPT);
    new FXMenuCommand(m_file_menu, "&Quit\tCtl-Q", NULL, this, FXTopWindow::ID_CLOSE);
    new FXMenuTitle(m_menu_bar, "&File", NULL, m_file_menu);

    m_help_menu = new FXMenuPane(this);
    new FXMenuCommand(m_help_menu, "About", NULL, this, ID_ABOUT);
    new FXMenuTitle(m_menu_bar, "&Help", NULL, m_help_menu, LAYOUT_RIGHT);

    FXVerticalFrame* frame = new FXVerticalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    // Sunken border for text widget
    FXHorizontalFrame *textbox = new FXHorizontalFrame(frame, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

    m_text_font = new FXFont(app, console_font, 11);
    m_scroll_win = new FXScrollWindow(textbox, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_text = new LuaConsole(gs, lua_io, m_scroll_win, this, ID_CONSOLE, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_text->setFont(m_text_font);
}

void GslShellWindow::create()
{
    FXMainWindow::create();
    show(PLACEMENT_SCREEN);
}

long GslShellWindow::on_close(FXObject* obj, FXSelector sel, void* ptr)
{
    FXApp* app = getApp();
    app->handle(this, FXSEL(SEL_COMMAND, GslShellApp::ID_CONSOLE_CLOSE), NULL);
    return 0;
}

long
GslShellWindow::on_cmd_about(FXObject*, FXSelector, void*)
{
  FXDialogBox about(this,"About GSL Shell",DECOR_TITLE|DECOR_BORDER,0,0,0,0, 0,0,0,0, 0,0);
  FXGIFIcon picture(getApp(),gsl_shell_icon_data);
  new FXLabel(&about,FXString::null,&picture,LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y,0,0,0,0, 0,0,0,0);
  FXVerticalFrame* side=new FXVerticalFrame(&about,LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 10,10,10,10, 0,0);
  new FXLabel(side,"G S L   S h e l l",NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FILL_X);
  new FXHorizontalSeparator(side,SEPARATOR_LINE|LAYOUT_FILL_X);
  new FXLabel(side, FXStringFormat(
    "GSL Shell %s, Copyright (C) 2009-2013 Francesco Abbate\n"
    "GNU Scientific Library, Copyright (C) The GSL Team\n"
    "%s -- %s\n"
    "\n"
    "With contribution of Lesley De Cruz, Benjamin Von Ardenne\n"
    "and Jesus Romero Hebrero.",
    GSL_SHELL_RELEASE, LUAJIT_VERSION, LUAJIT_COPYRIGHT),
    NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXButton *button=new FXButton(side,"&OK",NULL,&about,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
  button->setFocus();
  about.execute(PLACEMENT_OWNER);
  return 1;
}

long GslShellWindow::on_cmd_scroll_content(FXObject*, FXSelector, void*) {
    m_scroll_win->layout();
    FXint content_height = m_scroll_win->getContentHeight();
    FXint win_height = m_scroll_win->getHeight();
    m_scroll_win->setPosition(0, - (content_height - win_height));
    return 1;
}
