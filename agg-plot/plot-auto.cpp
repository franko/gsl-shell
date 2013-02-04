#include "plot-auto.h"

void plot_auto::add(sg_object* vs, agg::rgba8& color, bool outline)
{
    item d(vs, color, outline);

    if (!this->fit_inside(vs))
    {
        this->m_bbox_updated = false;
        this->m_need_redraw = true;
        this->m_enlarged_layer = true;
    }

    list<item> *nn = new list<item>(d);
    this->m_drawing_queue = list<item>::push_back(this->m_drawing_queue, nn);

    RM::acquire(vs);
}

void plot_auto::check_bounding_box()
{
    this->calc_bounding_box();
    this->update_units();
    this->m_bbox_updated = true;
}

void plot_auto::calc_layer_bounding_box(plot_auto::item_list* layer,
        opt_rect<double>& rect)
{
    for (unsigned j = 0; j < layer->size(); j++)
    {
        item& d = (*layer)[j];
        agg::rect_base<double> r;

        d.vs->bounding_box(&r.x1, &r.y1, &r.x2, &r.y2);
        rect.add<rect_union>(r);
    }
}

void plot_auto::calc_bounding_box()
{
    opt_rect<double> box;

    unsigned n = this->nb_layers();
    for (unsigned j = 0; j < n-1; j++)
    {
        box.add<rect_union>(this->get_layer(j)->bounding_box());
    }

    calc_layer_bounding_box(this->get_layer(n-1), box);
    for (list<item> *t = this->m_drawing_queue; t; t = t->next())
    {
        const item& d = t->content();
        agg::rect_base<double> r;
        d.vs->bounding_box(&r.x1, &r.y1, &r.x2, &r.y2);
        box.add<rect_union>(r);
    }

    this->m_rect = box;
}

bool plot_auto::fit_inside(sg_object* obj) const
{
    if (!this->m_bbox_updated || !this->m_rect.is_defined())
        return false;

    agg::rect_base<double> r;
    obj->bounding_box(&r.x1, &r.y1, &r.x2, &r.y2);

    const agg::rect_base<double>& bb = this->m_rect.rect();
    return bb.hit_test(r.x1, r.y1) && bb.hit_test(r.x2, r.y2);
}

void plot_auto::set_opt_limits(const opt_rect<double>& r)
{
    if (r.is_defined())
        this->set_limits(r.rect());
    else
        this->unset_limits();
}

bool plot_auto::push_layer()
{
    bool retval = this->plot::push_layer();
    if (this->m_rect.is_defined())
        this->parent_layer()->set_bounding_box(this->m_rect.rect());
    this->m_bbox_updated = true;
    this->m_enlarged_layer = false;
    return retval;
}

bool plot_auto::pop_layer()
{
    bool retval = this->plot::pop_layer();
    if (this->m_enlarged_layer)
        this->m_bbox_updated = false;
    this->m_enlarged_layer = true;
    return retval;
}

void plot_auto::clear_current_layer()
{
    this->plot::clear_current_layer();
    if (this->m_enlarged_layer)
    {
        item_list* parent = this->parent_layer();
        if (parent)
            set_opt_limits(parent->bounding_box());
        else
            this->unset_limits();
    }
    this->m_bbox_updated = true;
    this->m_enlarged_layer = false;
}
