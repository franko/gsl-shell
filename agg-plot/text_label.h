#ifndef AGGPLOT_TEXT_LABEL_H
#define AGGPLOT_TEXT_LABEL_H

#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_renderer_scanline.h"
#include "agg_font_freetype.h"

#include "sg_object.h"

class text_label
{
    enum { scale_x = 100 };

    typedef agg::font_engine_freetype_int32 font_engine_type;
    typedef agg::font_cache_manager<font_engine_type> font_manager_type;

    str m_text_buf;

    double m_text_width;
    double m_text_height;

    unsigned m_text_pos;
    double m_text_x, m_text_y;
    double m_advance_x, m_advance_y;

    font_engine_type& m_font_eng;
    font_manager_type& m_font_man;

    agg::trans_affine m_model_mtx;
    agg::trans_affine m_text_mtx;
    agg::conv_curve<font_manager_type::path_adaptor_type> m_text_curve;
    agg::conv_transform<agg::conv_curve<font_manager_type::path_adaptor_type> > m_text_trans;

  public:
    text_label(const char* text, double size):
    m_text_buf(text), m_text_height(size), m_text_pos(0),
    m_font_eng(gslshell::font_engine()), m_font_man(gslshell::font_manager()),
    m_text_curve(m_font_man.path_adaptor()), m_text_trans(m_text_curve, m_text_mtx)
    {
        set_font_size();
        m_text_width = get_text_width();
    }

    void model_mtx(const agg::trans_affine& m) { m_model_mtx = m; }

    void set_font_size()
    {
        m_font_eng.height(m_text_height);
        m_font_eng.width(m_text_height * scale_x);
    }

    double text_height() const { return m_text_height; }

    const str& text() const { return m_text_buf; }

    bool load_glyph()
    {
        m_text_x += m_advance_x;
        m_text_y += m_advance_y;

        if (m_text_pos >= m_text_buf.len())
            return false;

        char c = m_text_buf[m_text_pos];
        const agg::glyph_cache *glyph = m_font_man.glyph(c);
        m_font_man.add_kerning(&m_text_x, &m_text_y);
        m_font_man.init_embedded_adaptors(glyph, 0, 0);

        if(glyph->data_type == agg::glyph_data_outline)
        {
            m_text_mtx.tx = m_text_x / scale_x;
            m_text_mtx.ty = floor(m_text_y + 0.5);
            m_model_mtx.transform(&m_text_mtx.tx, &m_text_mtx.ty);

            m_advance_x = glyph->advance_x;
            m_advance_y = glyph->advance_y;

            return true;
        }

        return false;
    }

    void rewind(double hjustif, double vjustif)
    {
        m_text_x = scale_x * (- hjustif * m_text_width);
        m_text_y = - 0.6 * vjustif * m_text_height;
        m_advance_x = 0;
        m_advance_y = 0;
        m_text_pos = 0;

        m_text_mtx = m_model_mtx;
        agg::trans_affine_scaling scale_mtx(1.0 / double(scale_x), 1.0);
        trans_affine_compose (m_text_mtx, scale_mtx);

        set_font_size();
        load_glyph();
    }

    unsigned vertex(double* x, double* y)
    {
        unsigned cmd = m_text_trans.vertex(x, y);
        if (agg::is_stop(cmd))
        {
            m_text_pos++;
            if (load_glyph())
            {
                return vertex(x, y);
            }
            return agg::path_cmd_stop;
        }
        return cmd;
    }

    void approximation_scale(double as) { m_text_curve.approximation_scale(as); }

    double get_text_height() const { return m_text_height; }

    double get_text_width()
    {
        unsigned text_length = m_text_buf.len();
        double x = 0, y = 0;

        const char* text = m_text_buf.cstr();
        for (const char* p = text; p < text + text_length; p++)
        {
            const agg::glyph_cache* glyph = m_font_man.glyph(*p);

            if (glyph)
            {
                m_font_man.add_kerning(&x, &y);
                x += glyph->advance_x;
            }
        }

        return x / double(scale_x);
    }
};

#endif
