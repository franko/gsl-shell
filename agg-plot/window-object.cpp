
void
win_scalable_object::apply_transform(const agg::trans_affine& m)
{
  m_trans.transformer(m);
  double as = trans_affine_max_norm (m);
  m_source->approximation_scale (as);
}

void
win_scalable_object::bounding_box(double *x1, double *y1,
				  double *x2, double *y2)
{
  bounding_rect_single (*m_source, 0, x1, y1, x2, y2);
}
