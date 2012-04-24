#ifndef FOXGUI_FX_PLOT_CANVAS_H
#define FOXGUI_FX_PLOT_CANVAS_H

#include <new>

#include <fx.h>
#include <agg_rendering_buffer.h>

#include "sg_object.h"
#include "plot-auto.h"
#include "canvas.h"
#include "rect.h"

template <unsigned PixelSize, bool FlipY>
struct image_gen : agg::rendering_buffer {
  image_gen() { }
  image_gen(unsigned w, unsigned h) { init(w, h); }

  ~image_gen() { dispose(); }

  bool defined() const { return (buf() != 0); }

  void resize(unsigned w, unsigned h)
  {
    dispose();
    init(w, h);
  }

private:
  void init(unsigned w, unsigned h)
  {
    agg::int8u* data = new(std::nothrow) agg::int8u[w * h * PixelSize];
    int stride = (FlipY ? - w * PixelSize : w * PixelSize);
    attach(data, w, h, stride);
  }

  void dispose()
  {
    agg::int8u* data = buf();
    delete[] data;
  }
};

typedef image_gen<4, true> image;

class fx_plot_canvas : public FXCanvas {
  FXDECLARE(fx_plot_canvas)

public:
  typedef plot<sg_object, manage_owner> plot_type;

  fx_plot_canvas(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0,
		 FXuint opts=FRAME_NORMAL,
		 FXint x=0, FXint y=0, FXint w=0, FXint h=0);

  ~fx_plot_canvas();

  void attach(plot_type* p);
  void update_region(const agg::rect_base<int>& r);

  plot_type* get_plot() { return m_plot; }

  void plot_render(agg::trans_affine& m);
  void plot_draw(agg::trans_affine& m);
  opt_rect<double> plot_render_queue(agg::trans_affine& m);
  void plot_draw_queue(agg::trans_affine& m);

  agg::trans_affine& plot_matrix() { return m_area_mtx; }
  bool is_ready() const { return m_canvas && m_plot; }

  long on_cmd_paint(FXObject *, FXSelector, void *);
  long on_update(FXObject *, FXSelector, void *);

protected:
  fx_plot_canvas() {}

private:
  void prepare_image_buffer(unsigned ww, unsigned hh);
  void ensure_canvas_size(unsigned ww, unsigned hh);

  image m_img;
  image m_save_img;
  plot_type* m_plot;
  canvas* m_canvas;
  bool m_dirty_flag;
  opt_rect<double> m_dirty_rect;
  agg::trans_affine m_area_mtx;
};

#endif
