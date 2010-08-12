#ifndef CANVAS_WINDOW_CPP_H
#define CANVAS_WINDOW_CPP_H

#include "platform_support_ext.h"
#include "agg_trans_affine.h"
#include "agg_color_rgba.h"

#include "defs.h"
#include "canvas-window.h"
#include "drawable.h"
#include "canvas.h"
#include "utils.h"

class canvas_window : public agg::platform_support {
protected:
  canvas *m_canvas;
  agg::rgba m_bgcolor;

  agg::trans_affine m_matrix;

public:

  enum win_status_e { not_ready, starting, running, error, closed };

  int id;
  enum win_status_e status;

  canvas_window(agg::rgba& bgcol) :
    agg::platform_support(agg::pix_format_bgr24, true), 
    m_canvas(NULL), m_bgcolor(bgcol), m_matrix(), id(-1), status(not_ready)
  { };

  virtual ~canvas_window() 
  {
    if (m_canvas)
      delete m_canvas;
  };

  virtual void on_init();
  virtual void on_resize(int sx, int sy);

  void lock() { platform_support_lock(this); };
  void unlock() { platform_support_unlock(this); };

  void close() { platform_support_close_window(this); };

  void start_new_thread (lua_State *L);

  bool draw(drawable *obj, agg::rgba8 *color, bool as_line)
  {
    if (! m_canvas)
      return false;

    if (as_line)
      m_canvas->draw_outline(*obj, *color);
    else
      m_canvas->draw(*obj, *color);

    return true;
  };

  void scale (agg::trans_affine& m) { trans_affine_compose (m, m_matrix); };
 
  static canvas_window *check (lua_State *L, int index);
};

#endif
