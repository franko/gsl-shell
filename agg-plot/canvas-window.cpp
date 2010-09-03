
/* canvas-window.cpp
 * 
 * Copyright (C) 2009, 2010 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "defs.h"
#include "canvas-window-cpp.h"
#include "resource-manager.h"
#include "gsl-shell.h"
#include "agg-parse-trans.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "object-index.h"
#include "lua-draw.h"
#include "gs-types.h"
#include "colors.h"
#include "canvas.h"
#include "trans.h"

__BEGIN_DECLS

static void * canvas_thread_function        (void *_win);

__END_DECLS

void
canvas_window::on_resize(int sx, int sy)
{
  if (m_canvas)
    delete m_canvas;

  try 
    {
      m_canvas = new canvas(rbuf_window(), sx, sy, m_bgcolor);
    }
  catch (std::bad_alloc&)
    {
      m_canvas = 0;
    }
  
  m_matrix.sx = sx;
  m_matrix.sy = sy;
}

void
canvas_window::on_init()
{
  this->on_resize(width(), height());
}

void
canvas_window::start_new_thread (lua_State *L)
{
  if (status != not_ready && status != closed)
    return;

  this->id = object_index_add (L, OBJECT_WINDOW, -1);

  pthread_attr_t attr[1];
  pthread_t win_thread[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

  this->lock();
    
  this->status = canvas_window::starting;

  if (pthread_create(win_thread, attr, canvas_thread_function, (void*) this))
    {
      object_index_remove (L, OBJECT_WINDOW, this->id);

      pthread_attr_destroy (attr);
      this->status = canvas_window::error; 

      luaL_error(L, "error creating thread");
    }

  pthread_attr_destroy (attr);
}

void *
canvas_thread_function (void *_win)
{
  platform_support_prepare();

  canvas_window *win = (canvas_window *) _win;

  win->caption("GSL shell plot");
  if (win->init(480, 480, agg::window_resize))
    {
      win->status = canvas_window::running;
      int ec = win->run();
      win->status = (ec == 0 ? canvas_window::closed : canvas_window::error);
    }
  else
    {
      win->status = canvas_window::error;
    }
  
  GSL_SHELL_LOCK();
  gsl_shell_unref_plot (win->id);
  GSL_SHELL_UNLOCK();

  win->unlock();

  return NULL;
}

int
canvas_window_close (lua_State *L)
{
  canvas_window *win = object_check<canvas_window>(L, 1, GS_CANVAS_WINDOW);
  win->lock();
  if (win->status == canvas_window::running)
    win->close();
  win->unlock();
  return 0;
}
