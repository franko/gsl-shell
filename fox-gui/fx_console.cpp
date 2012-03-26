
#include <errno.h>
#include <fxkeys.h>

#include "fx_console.h"
#include "lua_engine.h"

FXDEFMAP(fx_console) fx_console_map[]={
  FXMAPFUNC(SEL_KEYPRESS, 0, fx_console::on_key_press),
  FXMAPFUNC(SEL_TIMEOUT, fx_console::ID_READ_INPUT, fx_console::on_read_input),
};

FXIMPLEMENT(fx_console,FXText,fx_console_map,ARRAYNUMBER(fx_console_map))

char const * const fx_console::prompt = "> ";

fx_console::fx_console(FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb):
  FXText(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb),
  m_status(not_ready), m_engine()
{
}

void fx_console::prepare_input()
{
  appendText(prompt, strlen(prompt));
  m_status = input_mode;
  m_input_begin = getCursorPos();
}

void fx_console::show_errors()
{
  if (m_engine.eval_status() == gsl_shell::eval_error)
    {
      appendText("Error reported: ");
      appendText(m_engine.error_msg());
      appendText("\n");
    }
}

void fx_console::create()
{
  FXText::create();
  init("Welcome to GSL Shell 2.1\n");
  m_engine.start();
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
    {
      FXint pos = getCursorPos();
      FXint line_end = lineEnd(pos), line_start = m_input_begin;
      extractText(m_input, line_start, line_end - line_start);
      appendText("\n");

      this->m_status = output_mode;
      m_engine.input(m_input.text());
      on_read_input(NULL, 0, NULL);
      return 1;
    }

  return FXText::onKeyPress(obj, sel, ptr);
}

long fx_console::on_read_input(FXObject* obj, FXSelector sel, void* ptr)
{
  char buffer[1024];

  while (1)
    {
      int nr = m_engine.read(buffer, 1023);
      if (nr < 0)
	{
#ifndef WIN32
	  if (errno == EAGAIN || errno == EWOULDBLOCK)
	    break;
#endif
	  fprintf(stderr, "ERROR on read: %d.\n", errno);
	  break;
	}
      if (nr == 0)
	break;

      if (buffer[nr-1] == gsl_shell_thread::eot_character)
	{
	  buffer[nr-1] = 0;
	  appendText(buffer);
	  show_errors();
	  prepare_input();
	  return 1;
	}

      buffer[nr] = 0;
      appendText(buffer);
    }

  getApp()->addTimeout(this, ID_READ_INPUT, 200, NULL);
  return 1;
}
