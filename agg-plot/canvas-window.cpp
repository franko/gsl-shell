
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

#include "defs.h"
#include "canvas-window-cpp.h"
#include "resource-manager.h"
#include "gsl-shell.h"
#include "agg-parse-trans.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "window_registry.h"
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

  m_canvas = new(std::nothrow) canvas(rbuf_window(), sx, sy, m_bgcolor);
  
  m_matrix.sx = sx;
  m_matrix.sy = sy;
}

void
canvas_window::on_init()
{
  this->on_resize(width(), height());
}

bool canvas_window::start_new_thread (std::auto_ptr<canvas_window::thread_info>& inf)
{
  if (status != not_ready && status != closed)
    return false;

  pthread_attr_t attr[1];
  pthread_t win_thread[1];

  pthread_attr_init (attr);
  pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

  this->status = canvas_window::starting;

  void *user_data = (void *) inf.get();
  if (pthread_create(win_thread, attr, canvas_thread_function, user_data))
    {
      this->status = canvas_window::error; 
      pthread_attr_destroy (attr);
      return false;
    }
  else
    {
      inf.release();
      pthread_attr_destroy (attr);
    }

  return true;
}

void *
canvas_thread_function (void *_inf)
{
  typedef canvas_window::thread_info thread_info;

  std::auto_ptr<thread_info> inf((thread_info *) _inf);
  platform_support_ext::prepare();
  canvas_window *win = inf->win;

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

  win->unlock();

  GSL_SHELL_LOCK();
  window_index_remove (inf->L, inf->window_id);
  GSL_SHELL_UNLOCK();

  return NULL;
}
