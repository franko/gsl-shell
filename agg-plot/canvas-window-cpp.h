#ifndef CANVAS_WINDOW_CPP_H
#define CANVAS_WINDOW_CPP_H

#include "platform/agg_platform_support.h"
#include "agg_trans_affine.h"
#include "agg_color_rgba.h"

#include "defs.h"
#include "canvas-window.h"
#include "vertex-source.h"
#include "canvas.h"
#include "utils.h"

extern void platform_support_prepare   ();
extern void platform_support_lock      (agg::platform_support *app);
extern void platform_support_unlock    (agg::platform_support *app);
extern bool platform_support_is_mapped (agg::platform_support *app);

class canvas_window : public agg::platform_support {
protected:
  canvas *m_canvas;
  agg::rgba m_bgcolor;

  agg::trans_affine m_user_trans;
  agg::trans_affine m_canvas_trans;

public:

  enum win_status_e { not_ready, starting, running, error, closed };

  int id;
  enum win_status_e status;

  canvas_window(agg::rgba& bgcol) :
    agg::platform_support(agg::pix_format_bgr24, true), 
    m_canvas(NULL), m_bgcolor(bgcol), m_user_trans(), m_canvas_trans(), 
    id(-1), status(not_ready)
  { };

  virtual ~canvas_window() 
  {
    if (m_canvas)
      delete m_canvas;
  };

  virtual void on_init();
  virtual void on_resize(int sx, int sy);
  virtual void on_draw();

  void lock() { platform_support_lock(this); };
  void unlock() { platform_support_unlock(this); };

  void start_new_thread (lua_State *L);

  bool draw(vertex_source *obj, agg::rgba8 *color)
  {
    if (! m_canvas)
      return false;

    m_canvas->draw(*obj, *color);
    return true;
  };

  void set_user_transform(agg::trans_affine& mtx)
  {
    m_user_trans = mtx;
  };

  void set_global_transform(agg::trans_affine& mtx)
  {
    mtx = m_user_trans;
    trans_affine_compose (mtx, m_canvas_trans);
  }
 
  static canvas_window *check (lua_State *L, int index);
};

#endif
