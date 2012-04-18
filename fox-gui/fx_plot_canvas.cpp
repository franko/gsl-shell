#include "fx_plot_canvas.h"

FXDEFMAP(fx_plot_canvas) fx_plot_canvas_map[]={
  FXMAPFUNC(SEL_PAINT,  0, fx_plot_canvas::on_cmd_paint),
  FXMAPFUNC(SEL_UPDATE, 0, fx_plot_canvas::on_update),
};

FXIMPLEMENT(fx_plot_canvas,FXCanvas,fx_plot_canvas_map,ARRAYNUMBER(fx_plot_canvas_map));

fx_plot_canvas::fx_plot_canvas(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h):
  FXCanvas(p, tgt, sel, opts, x, y, w, h),
  m_img_data(0), m_plot(0), m_canvas(0), m_dirty_flag(true)
{
}

fx_plot_canvas::~fx_plot_canvas()
{
  delete[] m_img_data;
  delete m_canvas;
}

void fx_plot_canvas::prepare_image_buffer(int ww, int hh)
{
  delete[] m_img_data;

  const unsigned bpp = 32;
  const unsigned pixel_size = bpp / 8;

  m_img_data = new agg::int8u[ww * hh * pixel_size];

  m_img_width = ww;
  m_img_height = hh;

  unsigned width = ww, height = hh;
  unsigned stride = - width * pixel_size;

  m_rbuf.attach(m_img_data, width, height, stride);
  m_canvas = new canvas(m_rbuf, width, height, colors::white);
}

void fx_plot_canvas::ensure_canvas_size(int ww, int hh)
{
  if (!m_img_data || m_img_width != ww || m_img_height != hh)
    {
      prepare_image_buffer(ww, hh);
    }
}

void fx_plot_canvas::draw(FXEvent* event)
{
  int ww = getWidth(), hh = getHeight();

  ensure_canvas_size(ww, hh);

  if (m_canvas && m_plot)
    {
      agg::trans_affine m(double(ww), 0.0, 0.0, double(hh), 0.0, 0.0);
      m_canvas->clear();
      m_plot->draw<canvas>(*m_canvas, m);

      const FXColor* data = (const FXColor*) m_img_data;
      FXImage img(getApp(), data, IMAGE_SHMI|IMAGE_SHMP, ww, hh);
      img.create();

      FXDCWindow *dc = (event ? new FXDCWindow(this, event) : new FXDCWindow(this));
      dc->drawImage(&img, 0, 0);
      delete dc;
    }

  m_dirty_flag = false;
}

void fx_plot_canvas::attach(plot_type* p)
{
  m_plot = p;
  m_dirty_flag = true;
}


long fx_plot_canvas::on_cmd_paint(FXObject *, FXSelector, void *ptr)
{
  FXEvent* ev = (FXEvent*) ptr;
  draw(m_dirty_flag ? NULL : ev);
  return 1;
}

long fx_plot_canvas::on_update(FXObject *, FXSelector, void *)
{
  bool need_upd = m_dirty_flag;
  if (need_upd)
    draw(NULL);
  return (need_upd ? 1 : 0);
}
