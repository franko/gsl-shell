
void plot_window::render(agg::trans_affine& m)
{
  assert(m_canvas != NULL);
  m_canvas->clear();
  AGG_LOCK();
  m_plot->draw(*m_canvas, m);
  AGG_UNLOCK();
}
