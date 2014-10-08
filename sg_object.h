
/* sg_object.h
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

#ifndef AGGPLOT_SG_OBJECT_H
#define AGGPLOT_SG_OBJECT_H

#include "agg_trans_affine.h"
#include "agg_bounding_rect.h"
#include "agg_conv_transform.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
#include "agg_rasterizer_scanline_aa.h"

#include "pixel_fmt.h"
#include "draw_svg.h"
#include "utils.h"
#include "resource-manager.h"
#include "strpp.h"

struct vertex_source {
    virtual void rewind(unsigned path_id) = 0;
    virtual unsigned vertex(double* x, double* y) = 0;
    virtual ~vertex_source() { }
};

// Scalable Graphics Object
struct sg_object : public vertex_source {

    virtual void apply_transform(const agg::trans_affine& m, double as) = 0;
    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;

    virtual bool affine_compose(agg::trans_affine& m) {
        return false;
    }

    virtual str write_svg(int id, agg::rgba8 c, double h) {
        str path;
        svg_property_list* ls = this->svg_path(path, h);
        str s = svg_fill_path(path, id, c, ls);
        svg_property_list::free(ls);
        return s;
    }

    virtual svg_property_list* svg_path(str& s, double h) {
        svg_coords_from_vs(this, s, h);
        return 0;
    }

    virtual ~sg_object() { }
};

struct approx_scale {
    template <class T> static void approximation_scale(T& obj, double as)
    {
        obj.approximation_scale(as);
    }
};

struct no_approx_scale {
    template <class T> static void approximation_scale(T& obj, double as) { }
};

template <class VertexSource, class ApproxManager=no_approx_scale>
class sg_object_gen : public sg_object {
protected:
    VertexSource m_base;

public:
    sg_object_gen(): m_base() {}

    template <class InitType> sg_object_gen(InitType& i) : m_base(i) { }

    template <class InitType1, class InitType2>
    sg_object_gen(InitType1& i1, InitType2& i2) : m_base(i1, i2) { }

    virtual void rewind(unsigned path_id) {
        m_base.rewind(path_id);
    }
    virtual unsigned vertex(double* x, double* y) {
        return m_base.vertex(x, y);
    }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
        ApproxManager::approximation_scale(m_base, as);
    }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
        agg::bounding_rect_single(m_base, 0, x1, y1, x2, y2);
    }

    const VertexSource& self() const {
        return m_base;
    };
    VertexSource& self()       {
        return m_base;
    };
};

/* this class does create an sg_object obtained combining an an AGG
   transformation like conv_stroke, conv_dash or any other transform
   with a sg_object source. This adapter implements therefore the
   virtual methods from the sg_object abstract class */
template <class ConvType, class ApproxManager>
class sg_adapter : public sg_object {
protected:
    ConvType m_output;
    sg_object* m_source;

public:
    sg_adapter(sg_object* src): m_output(*src), m_source(src) { }

    template <class InitType>
    sg_adapter(sg_object* src, InitType& val): m_output(*src, val), m_source(src)
    { }

    virtual ~sg_adapter() { }

    virtual void rewind(unsigned path_id) {
        m_output.rewind(path_id);
    }
    virtual unsigned vertex(double* x, double* y) {
        return m_output.vertex(x, y);
    }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
        ApproxManager::approximation_scale(m_output, as);
        this->m_source->apply_transform(m, as);
    }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
        this->m_source->bounding_box(x1, y1, x2, y2);
    }

    const ConvType& self() const {
        return m_output;
    };
    ConvType& self()       {
        return m_output;
    };
};

/* This class add a scaling transformation to an object. The scaling
   transfomation is an affine transform that adapt to the size of the
   canvas where the object is drawn. */
template <class ResourceManager = manage_owner>
class sg_object_scaling : public sg_object
{
    sg_object* m_source;
    agg::conv_transform<sg_object> m_trans;

public:
    sg_object_scaling(sg_object* src, agg::trans_affine& mtx=identity_matrix):
        m_source(src), m_trans(*m_source, mtx)
    {
        ResourceManager::acquire(m_source);
    }

    virtual ~sg_object_scaling() {
        ResourceManager::dispose(m_source);
    }

    virtual void rewind(unsigned path_id) {
        m_trans.rewind(path_id);
    }
    virtual unsigned vertex(double* x, double* y) {
        return m_trans.vertex(x, y);
    }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
        m_trans.transformer(m);
        m_source->apply_transform (m, as * m.scale());
    }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
        agg::bounding_rect_single (*m_source, 0, x1, y1, x2, y2);
    }
};

template <class ResourceManager>
class sg_object_ref : public sg_object {
public:
    sg_object_ref(sg_object* src) : m_source(src)
    {
        ResourceManager::acquire(this->m_source);
    }

    virtual ~sg_object_ref() {
        ResourceManager::dispose(this->m_source);
    }

    virtual void rewind(unsigned path_id) {
        this->m_source->rewind(path_id);
    }

    virtual unsigned vertex(double* x, double* y) {
        return this->m_source->vertex(x, y);
    }

    virtual void apply_transform(const agg::trans_affine& m, double as) {
        this->m_source->apply_transform(m, as);
    }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) {
        this->m_source->bounding_box(x1, y1, x2, y2);
    }

    virtual str write_svg(int id, agg::rgba8 c, double h) {
        return this->m_source->write_svg(id, c, h);
    }

    virtual svg_property_list* svg_path(str& s, double h) {
        return this->m_source->svg_path(s, h);
    }

    virtual bool affine_compose(agg::trans_affine& m) {
        return this->m_source->affine_compose(m);
    }

private:
    sg_object* m_source;
};

#endif
