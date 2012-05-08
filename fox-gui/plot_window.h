#include "sg_object.h"
#include "plot.h"

typedef plot<sg_object, manage_owner> sg_type;

template <class Image>
class plot_window {
protected:
  plot_window();

  virtual ~plot_window();

  virtual void update_region(const agg::rect_base<int>& r) = 0;

  void attach(plot_type* p);
  sg_plot* get_plot() { return m_plot; }

  void plot_render(agg::trans_affine& m);
  void plot_draw(agg::trans_affine& m);
  opt_rect<double> plot_render_queue(agg::trans_affine& m);
  void plot_draw_queue(agg::trans_affine& m, bool draw_all);

  agg::trans_affine& plot_matrix() { return m_area_mtx; }
  bool is_ready() const { return m_canvas && m_plot; }

  bool save_image();
  bool restore_image();

private:
  void prepare_image_buffer(unsigned ww, unsigned hh);
  void ensure_canvas_size(unsigned ww, unsigned hh);

  Image m_img;
  Image m_save_img;
  sg_plot* m_plot;
  canvas* m_canvas;
  opt_rect<double> m_dirty_rect;
  agg::trans_affine m_area_mtx;
};
