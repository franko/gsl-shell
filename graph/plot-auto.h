
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
#include "rect.h"

#include "agg_array.h"
#include "agg_basics.h"

class plot_auto : public plot {
public:
    plot_auto() :
        plot(true), m_bbox_updated(true), m_enlarged_layer(false)
    { };

    virtual ~plot_auto() { };

    virtual void add(sg_object* vs, agg::rgba8& color, bool outline);
    virtual void before_draw()
    {
        plot::before_draw();
        if (!m_bbox_updated)
            check_bounding_box();
    };

    virtual bool push_layer();
    virtual bool pop_layer();
    virtual void clear_current_layer();

private:
    void calc_layer_bounding_box(item_list* layer, opt_rect<double>& rect);
    void set_opt_limits(const opt_rect<double>& r);

    void check_bounding_box();
    void calc_bounding_box();
    bool fit_inside(sg_object *obj) const;

    // bounding box
    bool m_bbox_updated;
    bool m_enlarged_layer;
};

#endif
