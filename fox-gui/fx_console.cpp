
#include <fxkeys.h>

#include "luajit.h"

#include "fx_console.h"
#include "gsl_shell_app.h"
#include "gsl_shell_thread.h"
#include "fx_plot_window.h"

FXDEFMAP(fx_console) fx_console_map[]={
  FXMAPFUNC(SEL_KEYPRESS, 0, fx_console::on_key_press),
  FXMAPFUNC(SEL_IO_READ, fx_console::ID_LUA_OUTPUT, fx_console::on_lua_output),
};

FXIMPLEMENT(fx_console,FXText,fx_console_map,ARRAYNUMBER(fx_console_map))

char const * const fx_console::prompt = "> ";

fx_console::fx_console(gsl_shell_thread* gs, FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb):
  FXText(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb),
  m_status(not_ready), m_engine(gs)
{
  FXApp* app = getApp();
  m_lua_io_signal = new FXGUISignal(app, this, ID_LUA_OUTPUT);
  m_lua_io_thread = new lua_io_thread(m_engine, m_lua_io_signal, &m_lua_io_mutex, &m_lua_io_buffer);
}

fx_console::~fx_console()
{
  delete m_lua_io_thread;
  delete m_lua_io_signal;
}

void fx_console::prepare_input()
{
  appendText(prompt, strlen(prompt));
  m_status = input_mode;
  m_input_begin = getCursorPos();
}

void fx_console::show_errors()
{
  if (m_engine->eval_status() == gsl_shell::eval_error)
    {
      appendText("Error reported: ");
      appendText(m_engine->error_msg());
      appendText("\n");
      makePositionVisible(getCursorPos());
    }
}

void fx_console::create()
{
  FXText::create();
  FXString msg;
  msg.format("GSL Shell %s, Copyright (C) 2009-2012 Francesco Abbate\n"
	     "GNU Scientific Library, Copyright (C) The GSL Team\n"
	     "%s -- %s\n",
	     GSL_SHELL_RELEASE, LUAJIT_VERSION, LUAJIT_COPYRIGHT);
  init(msg);
  setFocus();
  m_lua_io_thread->start();
}

void fx_console::init(const FXString& greeting)
{
  appendText(greeting);
  prepare_input();
}

long fx_console::on_key_press(FXObject* obj, FXSelector sel, void* ptr)
{
  FXEvent* event = (FXEvent*)ptr;
  if (event->code == KEY_Return && m_status == input_mode)
    {
      FXint pos = getCursorPos();
      FXint line_end = lineEnd(pos), line_start = m_input_begin;
      extractText(m_input, line_start, line_end - line_start);
      appendText("\n");

      this->m_status = output_mode;
      m_engine->input(m_input.text());

      return 1;
    }

  return FXText::onKeyPress(obj, sel, ptr);
}

long fx_console::on_lua_output(FXObject* obj, FXSelector sel, void* ptr)
{
  bool eot = false;

  m_lua_io_mutex.lock();
  FXint len = m_lua_io_buffer.length();
  if (len > 0)
    {
      if (m_lua_io_buffer[len-1] == gsl_shell_thread::eot_character)
	{
	  eot = true;
	  m_lua_io_buffer.trunc(len-1);
	}
    }
  appendText(m_lua_io_buffer);
  makePositionVisible(getCursorPos());

  m_lua_io_buffer.clear();
  m_lua_io_mutex.unlock();

  if (eot)
    {
      int status = m_engine->eval_status();

      if (status == gsl_shell::incomplete_input)
	{
	  m_status = input_mode;
	}
      else if (status == gsl_shell::exit_request)
	{
	  FXApp* app = getApp();
	  app->handle(this, FXSEL(SEL_COMMAND,FXApp::ID_QUIT), NULL);
	}
      else
	{
	  show_errors();
	  prepare_input();
	}
    }

  return 1;
}
