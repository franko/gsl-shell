#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "agg_array.h"

class gsl_shell_app : public FXApp {
public:
  gsl_shell_app() : FXApp("GSL Shell", "GSL Shell") {}

  void schedule_window(FXMainWindow* win) { m_windows_queue.add(win); }

  void spawn_scheduled_window()
  {
    while (m_windows_queue.size() > 0)
      {
	FXMainWindow* w = m_windows_queue.last();
	w->create();
	w->show(PLACEMENT_SCREEN);
	m_windows_queue.remove_last();
      }
  }

private:
  agg::pod_bvector<FXMainWindow*> m_windows_queue;
};

#endif
