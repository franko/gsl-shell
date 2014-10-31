#include "plot.h"

static double compute_scale(const agg::trans_affine& m)
{
    return m.scale() / 480.0;
}

static double
std_line_width(double scale, double w = 1.0)
{
#if 0
    const double dsf = M_LN10;
    double ls = log(scale) / dsf;
    return exp(round(ls) * dsf) * w * 1.5;
#else
    return w * 1.5;
#endif
}

void plot::commit_pending_draw()
{
    push_drawing_queue();
    m_need_redraw = false;
    m_changes_pending.clear();
}

void plot::add(sg_object* vs, agg::rgba8& color, bool outline)
{
    item d(vs, color, outline);
    list<item> *new_node = new list<item>(d);
    m_drawing_queue = list<item>::push_back(m_drawing_queue, new_node);
    RM::acquire(vs);
}

void plot::push_drawing_queue()
{
    item_list* layer = current_layer();
    for (list<item> *c = m_drawing_queue; c != 0; c = c->next())
    {
        layer->add(c->content());
    }

    while (m_drawing_queue)
        m_drawing_queue = list<item>::pop(m_drawing_queue);
}

void plot::clear_drawing_queue()
{
    while (m_drawing_queue)
    {
        item& d = m_drawing_queue->content();
        RM::dispose(d.vs);
        m_drawing_queue = list<item>::pop(m_drawing_queue);
    }
}

static bool area_is_valid(const agg::trans_affine& b)
{
    const double thresold = 40.0;
    return (b.sx > thresold && b.sy > thresold);
}

void plot::draw_virtual_canvas(canvas_type& canvas, plot_layout& layout, const agg::rect_i* clip)
{
    before_draw();
    draw_legends(canvas, layout);

    if (area_is_valid(layout.plot_area))
    {
        draw_axis(canvas, layout, clip);
        draw_elements(canvas, layout);
    }
};

void plot::draw_simple(canvas_type& canvas, plot_layout& layout, const agg::rect_i* clip)
{
    before_draw();
    draw_axis(canvas, layout, clip);
    draw_elements(canvas, layout);
};

void plot::draw_element(item& c, canvas_type& canvas, const agg::trans_affine& m)
{
    sg_object& vs = c.content();
    vs.apply_transform(m, 1.0);

    if (c.outline)
        canvas.draw_outline(vs, c.color);
    else
        canvas.draw(vs, c.color);
}

agg::trans_affine plot::get_model_matrix(const plot_layout& layout)
{
    agg::trans_affine m = m_trans;
    trans_affine_compose (m, layout.plot_active_area);
    return m;
}

void plot::clip_plot_area(canvas_type& canvas, const agg::trans_affine& area_mtx)
{
    if (this->clip_is_active())
    {
        agg::rect_base<int> clip = rect_of_slot_matrix<int>(area_mtx);
        canvas.clip_box(clip);
    }
}

void plot::draw_elements(canvas_type& canvas, const plot_layout& layout)
{
    const agg::trans_affine m = get_model_matrix(layout);

    this->clip_plot_area(canvas, layout.plot_active_area);

    for (unsigned k = 0; k < m_layers.size(); k++)
    {
        item_list& layer = *(m_layers[k]);
        for (unsigned j = 0; j < layer.size(); j++)
        {
            draw_element(layer[j], canvas, m);
        }
    }

    canvas.reset_clipping();
}

void plot::compute_user_trans()
{
    agg::rect_base<double> r;

    if (m_use_units && m_pad_units)
    {
        int ixi, ixs, iyi, iys;
        double xd, yd;
        m_ux.limits(ixi, ixs, xd);
        r.x1 = ixi * xd;
        r.x2 = ixs * xd;

        m_uy.limits(iyi, iys, yd);
        r.y1 = iyi * yd;
        r.y2 = iys * yd;
    }
    else
    {
        r = m_rect.is_defined() ? m_rect.rect() : agg::rect_base<double>(0.0, 0.0, 1.0, 1.0);
    }

    if (m_xaxis_hol)
    {
        for (unsigned k = 0; k < m_xaxis_hol->size(); k++)
        {
            factor_labels* f = m_xaxis_hol->at(k);
            double x1 = f->mark(0), x2 = f->mark(f->labels_number());
            if (k == 0 || x1 < r.x1) r.x1 = x1;
            if (k == 0 || x2 > r.x2) r.x2 = x2;
        }
    }

    double dx = r.x2 - r.x1, dy = r.y2 - r.y1;
    double fx = (dx == 0 ? 1.0 : 1/dx), fy = (dy == 0 ? 1.0 : 1/dy);
    this->m_trans = agg::trans_affine(fx, 0.0, 0.0, fy, -r.x1 * fx, -r.y1 * fy);
}

void plot::draw_grid(const axis_e dir, const units& u,
                         const agg::trans_affine& user_mtx,
                         agg::path_storage& ln)
{
    const double eps = 1.0e-3;
    const bool isx = (dir == x_axis);

    int jinf = u.begin(), jsup = u.end();
    for (int j = jinf+1; j < jsup; j++)
    {
        double uq = u.mark_value(j);
        double x = (isx ? uq : 0), y = (isx ? 0.0 : uq);
        user_mtx.transform(&x, &y);
        const double q = (isx ? x : y);

        if (q >= -eps && q <= 1.0 + eps)
        {
            ln.move_to(isx ? q : 0.0, isx ? 0.0 : q);
            ln.line_to(isx ? q : 1.0, isx ? 1.0 : q);
        }
    }
}


double plot::draw_axis_m(axis_e dir, units& u,
                             const agg::trans_affine& user_mtx,
                             ptr_list<draw::text>& labels, double scale,
                             agg::path_storage& mark, agg::path_storage& ln)
{
    const double ppad = double(axis_label_prop_space) / 1000.0;
    const double text_label_size = get_default_font_size(text_axis_labels, scale);
    const double eps = 1.0e-3;

    // used to store the bounding box of text labels
    opt_rect<double> bb;
    agg::rect_base<double> r;

    bool isx = (dir == x_axis);

    const axis& ax = get_axis(dir);
    double hj = ax.labels_hjustif(), vj = ax.labels_vjustif();
    double langle = ax.labels_angle();

    category_map::iterator clabels(ax.categories);
    units_iterator ulabels(u, ax.format_tag, ax.label_format());

    label_iterator* ilabels = (ax.use_categories ? (label_iterator*) &clabels : (label_iterator*) &ulabels);

    double uq;
    const char* text;
    while (ilabels->next(uq, text))
    {
        double x = (isx ? uq : 0.0), y = (isx ? 0.0 : uq);
        user_mtx.transform(&x, &y);

        double q = (isx ? x : y);

        if (q < -eps || q > 1.0 + eps)
            continue;

        draw::text* label = new draw::text(text, text_label_size, hj, vj);

        label->set_point(isx ? q : -ppad, isx ? -ppad : q);
        label->angle(langle);

        agg::bounding_rect_single(*label, 0, &r.x1, &r.y1, &r.x2, &r.y2);
        bb.add<rect_union>(r);

        labels.add(label);

        mark.move_to(isx ? q :  0.0 , isx ?  0.0  : q);
        mark.line_to(isx ? q : -0.01, isx ? -0.01 : q);
    }

    this->draw_grid(dir, u, user_mtx, ln);

    double label_size;
    if (bb.is_defined())
    {
        const agg::rect_base<double>& br = bb.rect();
        label_size = (isx ? br.y2 - br.y1 : br.x2 - br.x1);
    }
    else
    {
        label_size = 0.0;
    }

    return label_size;
}

double plot::draw_xaxis_factors(units& u,
                             const agg::trans_affine& user_mtx,
                             ptr_list<draw::text>& labels,
                             ptr_list<factor_labels>* f_labels, double scale,
                             agg::path_storage& mark, agg::path_storage& ln)
{
    const double text_label_size = get_default_font_size(text_axis_labels, scale);

    const axis& ax = get_axis(x_axis);
    const double lab_angle = ax.labels_angle();

    const double y_spac_top = 3, y_spac_bot = 3;
    const int layers_number = f_labels->size();
    double p_lab = 0;
    for (int layer = layers_number - 1; layer >= 0; layer--)
    {
        factor_labels* factor = f_labels->at(layer);

        if (factor->labels_number() > 256) continue;

        agg::pod_bvector<draw::text*> tlabels;
        double hmax = 0.0;
        bool draw_labels =  (factor->labels_number() < 32);
        if (draw_labels)
        {
            for (int k = 0; k < factor->labels_number(); k++)
            {
                const char* text = factor->label_text(k);
                draw::text* label = new draw::text(text, text_label_size, 0.5, 0.5);
                label->angle(lab_angle);

                double rx1, ry1, rx2, ry2;
                agg::bounding_rect_single(*label, 0, &rx1, &ry1, &rx2, &ry2);
                double rh = ry2 - ry1;
                if (rh > hmax) hmax = rh;
                tlabels.add(label);
            }
        }

        double p_lab_inf = p_lab - (y_spac_top + y_spac_bot + hmax);

        for (int k = 0; k < factor->labels_number(); k++)
        {
            double x_lab_a = factor->mark(k);
            double x_lab_b = factor->mark(k+1);

            double x_a = x_lab_a, y_a = 0.0;
            user_mtx.transform(&x_a, &y_a);
            double q_a = x_a;

            double x_lab = (x_lab_a + x_lab_b) / 2, y_lab = 0.0;
            user_mtx.transform(&x_lab, &y_lab);
            double q_lab = x_lab;

            mark.move_to(q_a, p_lab);
            mark.line_to(q_a, p_lab_inf);

            if (draw_labels)
            {
                draw::text* label = tlabels[k];
                label->set_point(q_lab, p_lab_inf + y_spac_bot + hmax/2.0);
                labels.add(label);
            }
        }

        double x_lab = factor->mark(factor->labels_number());
        double x_a = x_lab, y_a = 0.0;
        user_mtx.transform(&x_a, &y_a);
        double q_a = x_a;
        mark.move_to(q_a, p_lab);
        mark.line_to(q_a, p_lab_inf);

        p_lab = p_lab_inf;
    }

    this->draw_grid(x_axis, u, user_mtx, ln);

    return - p_lab;
}

static inline double approx_text_height(double text_size)
{
    return text_size * 1.5;
}

plot_layout plot::compute_plot_layout(const agg::trans_affine& canvas_mtx, bool do_legends)
{
    plot_layout layout;

    const double sx = canvas_mtx.sx, sy = canvas_mtx.sy;
    const double ppad = double(canvas_margin_prop_space) / 1000.0;
    const double fpad = double(canvas_margin_fixed_space);
    const double size_frac_x = 0.125, size_frac_y = 0.05;

    double dxl, dxr, dyb, dyt;

    dxl = dxr = fpad + ppad * sx;
    dyb = dyt = fpad + ppad * sy;

    if (!str_is_null(&m_title))
    {
        const double scale = compute_scale(canvas_mtx);
        const double ptpad = double(axis_title_prop_space) / 1000.0;
        const double title_text_size = get_default_font_size(text_plot_title, scale);
        const double th = approx_text_height(title_text_size);

        double x = 0.5, y = 1.0;
        canvas_mtx.transform(&x, &y);
        y -= ptpad + dyt + title_text_size;

        layout.title_pos = plot_layout::point(x, y);
        layout.title_font_size = title_text_size;

        dyt += 2 * ptpad + th;
    }

    for (int k = 0; k < 4 && do_legends; k++)
    {
        plot* mp = m_legend[k];

        if (mp)
        {
            agg::rect_base<double> bb;
            mp->get_bounding_rect(bb);

            double bb_dx = bb.x2 - bb.x1, bb_dy = bb.y2 - bb.y1;
            double dx, dy;
            double px, py;
            switch (k)
            {
            case right:
                dx = max(sx * size_frac_x, bb_dx);
                dy = dx * bb_dy / bb_dx;
                px = sx - dx - ppad * sx - dxr;
                py = (sy - dy) / 2;
                dxr += dx + 2 * ppad * sx;
                break;
            case left:
                dx = max(sx * size_frac_x, bb_dx);
                dy = dx * bb_dy / bb_dx;
                px = ppad * sx + dxr;
                py = (sy - dy) / 2;
                dxl += dx + 2 * ppad * sx;
                break;
            case bottom:
                dy = sy * size_frac_y;
                dx = dy * bb_dx / bb_dy;
                py = ppad * sy + dyb;
                px = (sx - dx) / 2;
                dyb += dy + 2 * ppad * sy;
                break;
            case top:
                dy = sy * size_frac_y;
                dx = dy * bb_dx / bb_dy;
                py = sy - dy - ppad * sy - dyt;
                px = (sx - dx) / 2;
                dyt += dy + 2 * ppad * sy;
            }

            if (px >= 0 && py >= 0 && px + dx < sx && py + dy < sy)
            {
                const double x0 = canvas_mtx.tx + px, y0 = canvas_mtx.ty + py;
                layout.legend_area[k] = agg::trans_affine(dx, 0.0, 0.0, dy, x0, y0);
            }
            else
            {
                plot_layout::set_area_undefined(layout.legend_area[k]);
            }
        }
    }

    double x0 = canvas_mtx.tx + dxl, y0 = canvas_mtx.ty + dyb;
    double ssx = sx - (dxl + dxr), ssy = sy - (dyb + dyt);
    layout.plot_area = agg::trans_affine(ssx, 0.0, 0.0, ssy, x0, y0);

    return layout;
}

void plot::draw_legends(canvas_type& canvas, const plot_layout& layout)
{
    if (!str_is_null(&m_title))
    {
        const plot_layout::point& pos = layout.title_pos;
        draw::text title(m_title.cstr(), layout.title_font_size, 0.5, 0.0);
        title.set_point(pos.x, pos.y);
        title.apply_transform(identity_matrix, 1.0);
        canvas.draw(title, colors::black);
    }

    for (int k = 0; k < 4; k++)
    {
        plot* mp = m_legend[k];
        const agg::trans_affine& mtx = layout.legend_area[k];

        if (mp && plot_layout::is_area_defined(mtx))
        {
            agg::rect_i clip = rect_of_slot_matrix<int>(mtx);
            plot_layout mp_layout = mp->compute_plot_layout(mtx, false);
            mp->draw_simple(canvas, mp_layout, &clip);
        }
    }
}

// Draw the axis elements and labels and set layout.plot_active_area
// to the actual plotting are matrix.
void plot::draw_axis(canvas_type& canvas, plot_layout& layout, const agg::rect_i* clip)
{
    if (!m_use_units)
    {
        layout.plot_active_area = layout.plot_area;
        return;
    }

    double scale = compute_scale(layout.plot_area);

    if (clip)
        canvas.clip_box(*clip);

    const agg::trans_affine& m = layout.plot_active_area;

    agg::path_storage box;
    sg_object_gen<agg::conv_transform<agg::path_storage> > boxtr(box, m);
    trans::stroke_a boxvs(&boxtr);

    box.move_to(0.0, 0.0);
    box.line_to(0.0, 1.0);
    box.line_to(1.0, 1.0);
    box.line_to(1.0, 0.0);
    box.close_polygon();

    agg::path_storage x_mark;
    sg_object_gen<agg::conv_transform<agg::path_storage> > x_mark_tr(x_mark, m);
    trans::stroke_a x_mark_stroke(&x_mark_tr);

    agg::path_storage y_mark;
    sg_object_gen<agg::conv_transform<agg::path_storage> > y_mark_tr(y_mark, m);
    trans::stroke_a y_mark_stroke(&y_mark_tr);

    agg::path_storage ln;
    sg_object_gen<agg::conv_transform<agg::path_storage> > ln_tr(ln, m);
    trans::dash_a lndash(&ln_tr);
    trans::stroke_a lns(&lndash);

    const double label_text_size = get_default_font_size(text_axis_title, scale);
    const double plpad = double(axis_label_prop_space) / 1000.0;
    const double ptpad = double(axis_title_prop_space) / 1000.0;

    ptr_list<draw::text> xlabels, ylabels;

    double dy_label = 0;
    if (this->m_xaxis_hol)
        dy_label = draw_xaxis_factors(m_ux, m_trans, xlabels, this->m_xaxis_hol, scale, x_mark, ln);
    else
        dy_label = draw_axis_m(x_axis, m_ux, m_trans, xlabels, scale, x_mark, ln);

    double dx_label = draw_axis_m(y_axis, m_uy, m_trans, ylabels, scale, y_mark, ln);

    double ppad_left = plpad, ppad_right = plpad;
    double ppad_bottom = plpad, ppad_top = plpad;
    double dx_left = dx_label, dx_right = 0.0;
    double dy_bottom = dy_label, dy_top = 0.0;

    if (!str_is_null(&m_y_axis.title))
    {
        dx_left += approx_text_height(label_text_size);
        ppad_left += ptpad;
    }

    if (!str_is_null(&m_x_axis.title))
    {
        dy_bottom += approx_text_height(label_text_size);
        ppad_bottom += ptpad;
    }

    const double sx = layout.plot_area.sx, sy = layout.plot_area.sy;
    const double x0 = layout.plot_area.tx, y0 = layout.plot_area.ty;

    const double xppad = (ppad_left + ppad_right);
    const double lsx = (dx_left + dx_right + xppad * sx) / (1 + xppad);

    const double yppad = (ppad_bottom + ppad_top);
    const double lsy = (dy_bottom + dy_top + yppad * sy) / (1 + yppad);

    const double sxr = sx - lsx;
    const double syr = sy - lsy;

    const double aax = x0 + dx_left + ppad_left * sxr;
    const double aay = y0 + dy_bottom + ppad_bottom * syr;
    layout.set_plot_active_area(sxr, syr, aax, aay);

    agg::trans_affine m_xlabels;
    if (this->m_xaxis_hol)
    {
        m_xlabels.sx = m.sx;
        m_xlabels.tx = m.tx;
        m_xlabels.ty = m.ty;

        x_mark_tr.self().transformer(m_xlabels);
    }
    else
    {
        m_xlabels = m;
    }

    for (unsigned j = 0; j < xlabels.size(); j++)
    {
        draw::text* label = xlabels[j];
        label->apply_transform(m_xlabels, 1.0);
        canvas.draw(*label, colors::black);
    }

    for (unsigned j = 0; j < ylabels.size(); j++)
    {
        draw::text* label = ylabels[j];
        label->apply_transform(m, 1.0);
        canvas.draw(*label, colors::black);
    }

    lndash.add_dash(7.0, 3.0);

    lns.width(std_line_width(scale, 0.15));
    canvas.draw(lns, colors::black);

    x_mark_stroke.width(std_line_width(scale, 0.75));
    canvas.draw(x_mark_stroke, colors::black);

    y_mark_stroke.width(std_line_width(scale, 0.75));
    canvas.draw(y_mark_stroke, colors::black);

    boxvs.width(std_line_width(scale, 0.75));
    canvas.draw(boxvs, colors::black);

    if (!str_is_null(&m_x_axis.title))
    {
        double labx = m.sx * 0.5 + m.tx;
        double laby = y0;

        const char* text = m_x_axis.title.cstr();
        draw::text xlabel(text, label_text_size, 0.5, 0.0);
        xlabel.set_point(labx, laby);
        xlabel.apply_transform(identity_matrix, 1.0);

        canvas.draw(xlabel, colors::black);
    }

    if (!str_is_null(&m_y_axis.title))
    {
        double labx = x0;
        double laby = m.sy * 0.5 + m.ty;

        const char* text = m_y_axis.title.cstr();
        draw::text ylabel(text, label_text_size, 0.5, 1.0);
        ylabel.set_point(labx, laby);
        ylabel.angle(M_PI/2.0);
        ylabel.apply_transform(identity_matrix, 1.0);

        canvas.draw(ylabel, colors::black);
    }

    if (clip)
        canvas.reset_clipping();
}

void plot::set_axis_labels_angle(axis_e axis_dir, double angle)
{
    get_axis(axis_dir).set_labels_angle(angle);
    m_need_redraw = true;
    compute_user_trans();
}

void plot::set_units(bool use_units)
{
    if (m_use_units != use_units)
    {
        m_use_units = use_units;
        m_need_redraw = true;
        compute_user_trans();
    }
}

void plot::update_units()
{
    if (m_rect.is_defined())
    {
        const rect_base<double>& r = m_rect.rect();
        m_ux = units(r.x1, r.x2);
        m_uy = units(r.y1, r.y2);
    }
    else
    {
        m_ux = units();
        m_uy = units();
    }

    compute_user_trans();
}

void plot::set_limits(const agg::rect_base<double>& r)
{
    m_rect.set(r);
    update_units();
    m_need_redraw = true;
}

void plot::unset_limits()
{
    m_rect.clear();
    update_units();
    m_need_redraw = true;
}

void plot::layer_dispose_elements(plot::item_list* layer)
{
    unsigned n = layer->size();
    for (unsigned k = 0; k < n; k++)
    {
        RM::dispose(layer->at(k).vs);
    }
}

bool plot::push_layer()
{
    if (m_layers.size() >= max_layers)
        return false;

    item_list *new_layer = new(std::nothrow) item_list();
    if (new_layer)
    {
        before_draw();
        push_drawing_queue();
        m_layers.add(new_layer);
        return true;
    }

    return false;
}

bool plot::pop_layer()
{
    if (m_layers.size() <= 1)
        return false;

    unsigned n = m_layers.size();
    item_list* layer = m_layers[n-1];
    m_layers.inc_size(-1);
    layer_dispose_elements(layer);
    delete layer;

    clear_drawing_queue();
    m_need_redraw = true;

    return true;
}

void plot::clear_current_layer()
{
    item_list* current = current_layer();
    clear_drawing_queue();
    layer_dispose_elements(current);
    current->clear();
    m_changes_pending = m_changes_accu;
    m_changes_accu.clear();
}

int plot::current_layer_index()
{
    return m_layers.size();
}
