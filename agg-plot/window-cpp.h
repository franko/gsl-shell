
extern "C" {
#include "lua.h"
}

#include "window.h"
#include "canvas-window-cpp.h"
#include "resource-manager.h"
#include "lua-plot-cpp.h"
#include "plot.h"
#include "drawable.h"

#include "agg_color_rgba.h"
#include "agg_trans_affine.h"

template <class T>
class pod_list {
  T m_content;
  pod_list *m_next;

public:
  pod_list(const T& c, pod_list* next = NULL) : m_content(c), m_next(next) { };

  void free_subtree()
  {
    if (m_next)
      {
	m_next->free_subtree();
	delete m_next;
      }
  };

        T& content()       { return m_content; };
  const T& content() const { return m_content; };

  pod_list *next() { return m_next; };

  static void free(pod_list *list);
};

template <class T>
void pod_list<T>::free(pod_list<T> *list)
{
  list->free_subtree();
  delete list;
}

class window : public canvas_window {
  typedef plot<drawable, lua_management> plot_type;

  struct plot_matrix {
    plot_type *plot;
    agg::trans_affine matrix;

    plot_matrix(plot_type *p) : plot(p), matrix() {};
  };

  pod_list<plot_matrix> *m_plot_matrix;

public:
  window(agg::rgba& bgcol) : canvas_window(bgcol), m_plot_matrix(NULL) {};

  static window *check (lua_State *L, int index);

  void split3();
  bool attach(lua_plot *plot, int slot);

  void on_draw_unprotected();
  virtual void on_draw();
};
