
/* plot-auto.h
 * 
 * Copyright (C) 2009, 2010 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AGGPLOT_PLOT_AUTO_H
#define AGGPLOT_PLOT_AUTO_H

#include "plot.h"

#include "agg_array.h"
#include "agg_basics.h"

template<class VertexSource, class resource_manager = no_management>
class plot_auto : public plot<VertexSource, resource_manager> {
public:

  plot_auto() : 
    plot<VertexSource, resource_manager>(true),
    m_bbox_updated(true), m_is_empty(true)
  { };

  virtual ~plot_auto() { };

  virtual void add(VertexSource* vs, agg::rgba8 *color, bool outline);
  virtual void on_draw() { check_bounding_box(); };

private:
  void check_bounding_box();
  void calc_bounding_box();
  bool fit_inside(VertexSource *obj) const;

  // bounding box
  bool m_bbox_updated;
  bool m_is_empty;
};

template <class VS, class RM>
void plot_auto<VS,RM>::add(VS* vs, agg::rgba8 *color, bool outline) 
{ 
  typedef typename plot_auto<VS,RM>::container cnt_type;
  cnt_type d(vs, color, outline);

  if (!this->fit_inside(vs))
    {
      this->m_bbox_updated = false;
      this->m_need_redraw = true;
      this->m_elements.add(d);
    }
  else
    {
      this->m_drawing_queue = new pod_list<cnt_type>(d, this->m_drawing_queue);
    }

  this->m_is_empty = false;

  RM::acquire(vs);
}

template<class VS, class RM>
void plot_auto<VS,RM>::check_bounding_box()
  {
    if (this->m_bbox_updated || this->m_is_empty)
      return;

    this->calc_bounding_box();

    const agg::rect_base<double>& bb = this->m_rect;
    this->m_ux = units(bb.x1, bb.x2);
    this->m_uy = units(bb.y1, bb.y2);

    this->compute_user_trans();
    this->m_bbox_updated = true;
  }

template<class VS, class RM>
void plot_auto<VS,RM>::calc_bounding_box()
{
  for (unsigned j = 0; j < this->m_elements.size(); j++)
  {
    typename plot_auto<VS,RM>::container& d = this->m_elements[j];
    agg::rect_base<double> r;

    d.vs->bounding_box(&r.x1, &r.y1, &r.x2, &r.y2);
      
    if (j == 0)
      this->m_rect = r;
    else
      this->m_rect = agg::unite_rectangles(this->m_rect, r);
  }
}

template<class VS, class RM>
bool plot_auto<VS,RM>::fit_inside(VS* obj) const
{
  if (this->m_is_empty || !this->m_bbox_updated)
    return false;

  agg::rect_base<double> r;
  obj->bounding_box(&r.x1, &r.y1, &r.x2, &r.y2);

  const agg::rect_base<double>& bb = this->m_rect;
  return bb.hit_test(r.x1, r.y1) && bb.hit_test(r.x2, r.y2);
}

#endif
