
extern "C" {
#include "lua.h"
}

#include "window.h"
#include "canvas-window-cpp.h"
#include "resource-manager.h"
#include "lua-plot-cpp.h"
#include "plot.h"
#include "drawable.h"

#include "my_list.h"

#include "agg_color_rgba.h"
#include "agg_trans_affine.h"
#include "split-spec-parser.h"

class window : public canvas_window {
  typedef plot<drawable, lua_management> plot_type;

  split::node<plot_type*>* m_tree;

  void draw_rec(split::node<plot_type*> *n);

public:
  window(agg::rgba& bgcol) : canvas_window(bgcol), m_tree(0) {};

  ~window() { if (m_tree) delete m_tree; };

  static window *check (lua_State *L, int index);

  void split(const char *spec);
  bool attach(lua_plot *plot, const char *spec);

  void on_draw_unprotected();
  virtual void on_draw();
};
