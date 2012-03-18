
#include <fxkeys.h>
#include "fx_console.h"

FXDEFMAP(fx_console) fx_console_map[]={
  FXMAPFUNC(SEL_KEYPRESS, 0, fx_console::on_key_press),
};

FXIMPLEMENT(fx_console,FXText,fx_console_map,ARRAYNUMBER(fx_console_map))

char const * const fx_console::prompt = "> ";

fx_console::fx_console(FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb):
  FXText(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb), m_status(not_ready)
{
}

void fx_console::prepare_input()
{
  appendText(prompt, strlen(prompt));
  m_status = input_mode;
}

void fx_console::create()
{
  FXText::create();
  init("Welcome to GSL Shell 2.1\n");
}

void fx_console::init(const FXString& greeting)
{
  appendText(greeting);
  prepare_input();
}

long fx_console::on_key_press(FXObject* obj, FXSelector sel, void* ptr)
{
  FXEvent* event = (FXEvent*)ptr;
  if (event->code == KEY_Return)
    printf("KEY Enter!\n");
  return FXText::onKeyPress(obj, sel, ptr);
}
