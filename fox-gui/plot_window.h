#ifndef FOXGUI_PLOT_WINDOW_H
#define FOXGUI_PLOT_WINDOW_H

#include <agg_rendering_buffer.h>

#include "sg_object.h"
#include "plot.h"
#include "canvas.h"
#include "image_buf.h"

typedef plot<sg_object, manage_owner> sg_plot;

template <class Image>
class plot_window {
public:
  plot_window(sg_plot& p): m_plot(p), m_canvas(0) { }

  ~plot_window() { delete m_canvas; }

  //  virtual void update_region(const agg::rect_base<int>& r) = 0;

  sg_plot& plot() { return m_plot; }

  void render(agg::trans_affine& m);
  void draw(agg::trans_affine& m, unsigned width, unsigned height);
  opt_rect<double> render_queue(agg::trans_affine& m);
  //  void draw_queue(agg::trans_affine& m, bool draw_all);

  //  agg::trans_affine& plot_matrix() { return m_area_mtx; }
  //  bool is_ready() const { return m_canvas && m_plot; }

  bool save_image();
  bool restore_image();

private:
  void prepare_image_buffer(unsigned ww, unsigned hh);
  void ensure_canvas_size(unsigned ww, unsigned hh);

  Image m_img;
  Image m_save_img;
  sg_plot& m_plot;
  canvas* m_canvas;
  //  Window* m_window;
  opt_rect<double> m_dirty_rect;
  //  agg::trans_affine m_area_mtx;
};

template <class Image>
void plot_window<Image>::prepare_image_buffer(unsigned ww, unsigned hh)
{
  m_img.resize(ww, hh);
  m_canvas = new canvas(m_img, ww, hh, colors::white);
}

template <class Image>
void plot_window<Image>::ensure_canvas_size(unsigned ww, unsigned hh)
{
  if (m_img.width() != ww || m_img.height() != hh)
    prepare_image_buffer(ww, hh);
}

template <class Image>
void plot_window<Image>::render(agg::trans_affine& m)
{
  assert(m_canvas != NULL);
  m_canvas->clear();
  AGG_LOCK();
  m_plot.draw(*m_canvas, m);
  AGG_UNLOCK();
}

template <class Image>
void plot_window<Image>::draw(agg::trans_affine& m, unsigned width, unsigned height)
{
  ensure_canvas_size(width, height);
  render(m);
  //  m_window->put_image(m_img, m_x, m_y);
  //  m_dirty_flag = false;
}

template <class Image>
opt_rect<double> plot_window<Image>::render_queue(agg::trans_affine& m)
{
  assert(m_canvas != NULL);
  opt_rect<double> r;
  AGG_LOCK();
  m_plot.draw_queue(*m_canvas, m, r);
  AGG_UNLOCK();
  return r;
}

#endif
