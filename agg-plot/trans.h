#ifndef AGGPLOT_TRANS_H
#define AGGPLOT_TRANS_H

#include "sg_object.h"
#include "markers.h"
#include "utils.h"
#include "resource-manager.h"
#include "draw_svg.h"

#include "agg_trans_affine.h"
#include "agg_path_storage.h"

#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_transform.h"
#include "agg_conv_contour.h"

#include "my_conv_simple_marker.h"

struct trans {

    typedef sg_object_scaling<manage_owner> scaling;
    typedef sg_object_scaling<manage_not_owner> scaling_a;

    typedef agg::conv_stroke<sg_object> conv_stroke;

    class stroke_a : public sg_adapter<conv_stroke, approx_scale> {
        typedef sg_adapter<conv_stroke, approx_scale> base_type;
    public:
        stroke_a(sg_object* src): base_type(src), m_width(1.0) { }

        void width(double w) {
            this->m_output.width(w);
            m_width = w;
        }

        virtual str write_svg(int id, agg::rgba8 c, double h) {
            str path;
            svg_property_list* ls = this->m_source->svg_path(path, h);
            str s = svg_stroke_path(path, m_width, id, c, ls);
            svg_property_list::free(ls);
            return s;
        }

    private:
        double m_width;
    };

    struct stroke : stroke_a {
        stroke(sg_object* src) : stroke_a(src) {}
        virtual ~stroke() {
            delete m_source;
        }
    };

    //------------------------------------------------ curve transform
    typedef agg::conv_curve<sg_object> curve_type;

    class curve_a : public sg_adapter<curve_type, approx_scale> {
        typedef sg_adapter<curve_type, approx_scale> base_type;
    public:
        curve_a(sg_object* src) : base_type(src) { }

        virtual svg_property_list* svg_path(str& s, double h) {
            svg_curve_coords_from_vs(this->m_source, s, h);
            return 0;
        }
    };

    struct curve : curve_a {
        curve(sg_object* src) : curve_a(src) {}
        virtual ~curve() {
            delete m_source;
        }
    };

    //------------------------------------------------ dash transform
    typedef agg::conv_dash<sg_object> dash_type;

    class dash_a : public sg_adapter<dash_type, no_approx_scale> {
        typedef sg_adapter<dash_type, no_approx_scale> base_type;
    public:
        dash_a(sg_object* src) : base_type(src), m_dasharray(16) { }

        virtual svg_property_list* svg_path(str& s, double h) {
            svg_property_list* ls = this->m_source->svg_path(s, h);
            svg_property_item item(stroke_dasharray, m_dasharray.cstr());
            ls = new svg_property_list(item, ls);
            return ls;
        }

        void add_dash(double a, double b) {
            this->m_output.add_dash(a, b);
            this->m_dasharray.append("", ',');
            this->m_dasharray.printf_add("%g,%g", a, b);
        }

    private:
        str m_dasharray;
    };

    struct dash : dash_a {
        dash(sg_object* src) : dash_a(src) {}
        virtual ~dash() {
            delete m_source;
        }
    };

    //------------------------------------------------ affine transform
    typedef agg::conv_transform<sg_object> trans_type;

    class affine_a :
        public sg_adapter<trans_type, no_approx_scale> {
        agg::trans_affine m_matrix;
        double m_norm;

        typedef sg_adapter<trans_type, no_approx_scale> base_type;

    public:
        affine_a(sg_object *src, const agg::trans_affine& mtx) :
            base_type(src, m_matrix), m_matrix(mtx)
        {
            m_norm = m_matrix.scale();
        }

        virtual void apply_transform(const agg::trans_affine& m, double as)
        {
            this->m_source->apply_transform(m, as * m_norm);
        };

        virtual bool affine_compose(agg::trans_affine& m)
        {
            trans_affine_compose (m_matrix, m);
            return true;
        }
    };

    struct affine : affine_a {
        affine(sg_object* src, const agg::trans_affine& m) : affine_a(src, m) {}
        virtual ~affine() {
            delete m_source;
        }
    };

    //------------------------------------------------ extend transform
    struct extend : sg_adapter<agg::conv_contour<sg_object>, approx_scale> {
        extend(sg_object* src):
            sg_adapter<agg::conv_contour<sg_object>, approx_scale>(src)
        { }

        virtual ~extend() {
            delete m_source;
        }
    };

    //------------------------------------------------ marker transform
    typedef my::conv_simple_marker<sg_object, sg_object> marker_type;

    class marker_a :
        public sg_adapter<marker_type, no_approx_scale> {

        typedef sg_adapter<marker_type, no_approx_scale> base_type;

    public:
        marker_a(sg_object* src, double size, sg_object* sym):
            base_type(src, *sym), m_size(size), m_scale(m_size), m_symbol(sym)
        {
            // we need to apply the scale transform here to ensure that
            // any call to bounding_box have the correct informations about
            // the symbol size, even if it is called before apply_transform
            m_symbol->apply_transform(m_scale, 1.0);
        }

        virtual str write_svg(int id, agg::rgba8 c, double h) {
            str marker_id;
            str marker_def = gen_svg_marker_def(id, c, marker_id);

            str path;
            svg_property_list* ls = m_source->svg_path(path, h);

            str marker_url = gen_marker_url(marker_id);
            const char* murl = marker_url.cstr();
            svg_property_item item1(marker_start, murl);
            svg_property_item item2(marker_mid, murl);
            svg_property_item item3(marker_end, murl);
            ls = new svg_property_list(item1, ls);
            ls = new svg_property_list(item2, ls);
            ls = new svg_property_list(item3, ls);

            str svg = svg_marker_path(path, m_size, id, ls);
            svg_property_list::free(ls);

            return str::print("%s\n   %s", marker_def.cstr(), svg.cstr());
        }

        virtual ~marker_a() {
            delete m_symbol;
        }

        virtual void apply_transform(const agg::trans_affine& m, double as)
        {
            m_symbol->apply_transform(m_scale, as);
            m_source->apply_transform(m, as);
        }

    private:
        double m_size;
        agg::trans_affine_scaling m_scale;
        sg_object* m_symbol;

        str gen_svg_marker_def(int id, agg::rgba8 c, str& marker_id) {

            const double pad = 2.0;

            double tx_save = m_scale.tx, ty_save = m_scale.ty;

            m_scale.tx = m_size / 2.0 + pad;
            m_scale.ty = m_size / 2.0 + pad;

            marker_id.printf("marker%i", id);

            const double S = m_size + 2*pad;
            const double wf = S / m_size;

            str marker_svg = m_symbol->write_svg(-1, c, S);

            str s = str::print("<defs><marker id=\"%s\" "
                               "refX=\"%g\" refY=\"%g\" "
                               "viewBox=\"0 0 %g %g\" orient=\"0\" "
                               "markerWidth=\"%g\" markerHeight=\"%g\">"
                               "%s"
                               "</marker></defs>",
                               marker_id.cstr(), S/2, S/2, S, S, wf, wf,
                               marker_svg.cstr());

            m_scale.tx = tx_save;
            m_scale.ty = ty_save;

            return s;
        }

        static str gen_marker_url(str& marker_id) {
            return str::print("url(#%s)", marker_id.cstr());
        }
    };

    struct marker : marker_a {
        marker(sg_object* src, double size, sg_object* sym):
            marker_a(src, size, sym)
        {}

        virtual ~marker() {
            delete m_source;
        }
    };
};

#endif
