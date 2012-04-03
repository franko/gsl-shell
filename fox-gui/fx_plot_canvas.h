#ifndef FOXGUI_PLOT_CANVAS_H
#define FOXGUI_PLOT_CANVAS_H

#include <fx.h>
#include <agg_rendering_buffer.h>

#include "sg_object.h"
#include "plot-auto.h"
#include "canvas.h"

class fx_plot_canvas : public FXCanvas {
  FXDECLARE(fx_plot_canvas)

public:
  typedef plot_auto<sg_object, manage_owner> plot;

  fx_plot_canvas(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0,
		 FXuint opts=FRAME_NORMAL,
		 FXint x=0, FXint y=0, FXint w=0, FXint h=0);

  ~fx_plot_canvas();

  void attach(plot* p);
  void draw(FXEvent* event);

  long on_cmd_paint(FXObject *, FXSelector, void *);
  long on_update(FXObject *, FXSelector, void *);

protected:
  fx_plot_canvas() {}

private:
  void prepare_image_buffer(int ww, int hh);
  void ensure_canvas_size(int ww, int hh);

  FXImage* m_img;
  agg::rendering_buffer m_rbuf;
  plot* m_plot;
  canvas* m_canvas;
  bool m_dirty_flag;
};

#endif
