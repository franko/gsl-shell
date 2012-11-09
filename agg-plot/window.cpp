
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua-defs.h"
#include "window-cpp.h"
#include "window_registry.h"
#include "lua-draw.h"
#include "lua-graph.h"
#include "lua-cpp-utils.h"
#include "gs-types.h"
#include "colors.h"
#include "lua-plot-cpp.h"
#include "split-parser.h"
#include "lua-utils.h"
#include "platform_support_ext.h"

__BEGIN_DECLS

static int window_free            (lua_State *L);
static int window_split           (lua_State *L);
static int window_save_svg        (lua_State *L);

static const struct luaL_Reg window_functions[] = {
    {"window",        window_new},
    {NULL, NULL}
};

static const struct luaL_Reg window_methods[] = {
    {"show",           window_show          },
    {"attach",         window_attach        },
    {"layout",         window_split         },
    {"update",         window_update        },
    {"close",          window_close         },
    {"save_svg",       window_save_svg      },
    {"__gc",           window_free          },
    {NULL, NULL}
};

__END_DECLS

void window::ref::compose(bmatrix& a, const bmatrix& b)
{
    trans_affine_compose (a, b);
};

int window::ref::calculate(window::ref::node* t, const bmatrix& m, int id)
{
    ref *r = t->content();
    if (r)
    {
        r->slot_id = id++;
        r->matrix = m;
    }

    int nb = list<ref::node*>::length(t->tree());

    if (nb > 0)
    {
        double frac = 1 / (double) nb;

        direction_e dir;
        list<ref::node*> *ls = t->tree(dir);
        if (ls)
        {
            bmatrix lm;

            double* p = (dir == along_x ? &lm.tx : &lm.ty);
            double* s = (dir == along_x ? &lm.sx : &lm.sy);

            *s = frac;

            for ( ; ls; ls = ls->next(), *p += frac)
            {
                bmatrix sm(lm);
                window::ref::compose(sm, m);
                id = window::ref::calculate (ls->content(), sm, id);
            }
        }
    }

    return id;
}

void
window::ref::save_image (agg::rendering_buffer& win_buf,
                         agg::rect_base<int>& r,
                         int img_bpp, bool flip_y)
{
    int w = r.x2 - r.x1, h = r.y2 - r.y1;
    int row_len = w * (img_bpp / 8);

    if (layer_buf == 0)
    {
        unsigned int bufsize = row_len * h;
        layer_buf = new(std::nothrow) unsigned char[bufsize];
    }

    if (layer_buf != 0)
    {
        layer_img.attach(layer_buf, w, h, flip_y ? -row_len : row_len);
        rendering_buffer_get_region (layer_img, win_buf, r, img_bpp / 8);
    }
}

window::ref* window::ref_lookup (ref::node *p, int slot_id)
{
    list<ref::node*> *t = p->tree();
    for (/* */; t; t = t->next())
    {
        ref *ref = window::ref_lookup(t->content(), slot_id);
        if (ref)
            return ref;
    }

    ref *ref = p->content();
    if (ref)
    {
        if (ref->slot_id == slot_id)
            return ref;
    }

    return NULL;
}

void window::draw_slot_by_ref(window::ref& ref, bool draw_image)
{
    agg::trans_affine mtx(ref.matrix);
    this->scale(mtx);

    agg::rect_base<int> r = rect_of_slot_matrix<int>(mtx);
    m_canvas->clear_box(r);

    if (ref.plot)
    {
        AGG_LOCK();
        ref.plot->draw(*m_canvas, mtx, &ref.inf);
        AGG_UNLOCK();
    }

    if (draw_image)
        update_region(r);
}

void
window::draw_slot(int slot_id, bool clean_req)
{
    ref *ref = window::ref_lookup (this->m_tree, slot_id);
    if (ref && m_canvas)
    {
        bool redraw = clean_req || ref->plot->need_redraw();

        if (redraw)
        {
            draw_slot_by_ref(*ref, false);
            ref->dispose_buffer();
        }

        refresh_slot_by_ref(*ref, redraw);
        ref->valid_rect = true;
    }
}

void
window::save_slot_image(int slot_id)
{
    ref *ref = window::ref_lookup (this->m_tree, slot_id);
    if (ref != 0)
    {
        agg::trans_affine mtx(ref->matrix);
        this->scale(mtx);

        agg::rect_base<int> r = rect_of_slot_matrix<int>(mtx);
        ref->save_image(this->rbuf_window(), r, this->bpp(), this->flip_y());
    }
}

void
window::restore_slot_image(int slot_id)
{
    ref *ref = window::ref_lookup (this->m_tree, slot_id);
    if (ref != 0)
    {
        agg::trans_affine mtx(ref->matrix);
        this->scale(mtx);

        agg::rect_base<int> r = rect_of_slot_matrix<int>(mtx);

        if (ref->layer_buf == 0)
        {
            m_canvas->clear_box(r);
            draw_slot_by_ref (*ref, false);
            ref->save_image(this->rbuf_window(), r, this->bpp(), this->flip_y());
        }
        else
        {
            agg::rendering_buffer& img = ref->layer_img;
            agg::rendering_buffer& win = this->rbuf_window();
            rendering_buffer_put_region (win, img, r, this->bpp() / 8);
        }
    }
}

void
window::refresh_slot_by_ref(ref& ref, bool draw_all)
{
    agg::trans_affine mtx(ref.matrix);
    this->scale(mtx);

    opt_rect<double> rect;

    if (!ref.valid_rect || draw_all)
        rect.set(rect_of_slot_matrix<double>(mtx));

    AGG_LOCK();
    opt_rect<double> draw_rect;
    ref.plot->draw_queue(*m_canvas, mtx, ref.inf, draw_rect);
    rect.add<rect_union>(draw_rect);
    rect.add<rect_union>(ref.dirty_rect);
    ref.dirty_rect = draw_rect;
    AGG_UNLOCK();

    if (rect.is_defined())
    {
        const int m = 4;
        const agg::rect_base<double>& r = rect.rect();
        const agg::rect_base<int> ri(r.x1 - m, r.y1 - m, r.x2 + m, r.y2 + m);
        update_region (ri);
    }
}

void
window::on_draw()
{
    if (m_canvas)
    {
        slot_draw_function draw_func(this);
        this->plot_apply(draw_func);
    }
}

void
window::on_resize(int sx, int sy)
{
    this->canvas_window::on_resize(sx, sy);
    if (m_tree)
    {
        dispose_buffer_function dispose;
        this->plot_apply(dispose);
    }
}

struct refs_remove_function {
    refs_remove_function(lua_State *_L, int k): L(_L), window_index(k) {}
    void call(window::ref* ref) { if (ref->plot) window_refs_remove(L, ref->slot_id, window_index); }
    lua_State* L;
    int window_index;
};

bool
window::split(const char *spec)
{
    ::split<ref>::lexer lexbuf(spec);
    tree::node<ref, direction_e> *parse_tree = ::split<ref>::parse(lexbuf);
    delete m_tree;

    if (parse_tree)
        m_tree = parse_tree;
    else
        m_tree = new tree::leaf<ref, direction_e>();

    bmatrix m0;
    ref::calculate(m_tree, m0, 0);
    return (parse_tree != NULL);
}

template <class Function>
void window::plot_apply_rec(Function& f, ref::node* n)
{
    list<ref::node*> *ls;
    for (ls = n->tree(); ls != NULL; ls = ls->next())
        this->plot_apply_rec(f, ls->content());

    ref* ref = n->content();
    if (ref)
    {
        f.call(ref);
    }
}

static const char *
next_int (const char *str, int& val)
{
    while (*str == ' ')
        str++;
    if (*str == '\0')
        return NULL;

    char *eptr;
    val = strtol (str, &eptr, 10);

    if (eptr == str)
        return NULL;

    while (*eptr == ' ')
        eptr++;
    if (*eptr == ',')
        eptr++;
    return eptr;
}

/* Returns the slot_id or -1 in case of error. */
int window::attach(sg_plot* plot, const char *spec)
{
    ref::node *n = m_tree;
    const char *ptr;
    int k;

    for (ptr = next_int (spec, k); ptr; ptr = next_int (ptr, k))
    {
        list<ref::node*>* list = n->tree();

        if (! list)
            return -1;

        for (int j = 1; j < k; j++)
        {
            list = list->next();
            if (! list)
                return -1;
        }

        n = list->content();
    }

    ref* r = n->content();
    if (! r)
        return -1;

    r->plot = plot;

    return r->slot_id;
}

typedef void (window::*window_slot_method_type)(int slot_id);

int window_generic_oper (lua_State *L, window_slot_method_type method)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    int slot_id = luaL_checkinteger (L, 2);

    win->lock();
    if (win->status == canvas_window::running)
    {
        (win->*method)(slot_id);
    }
    win->unlock();

    return 0;
}

template <class param_type>
int window_generic_oper_ext (lua_State *L,
                             void (window::*method)(int, param_type),
                             param_type param)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    int slot_id = luaL_checkinteger (L, 2);

    win->lock();
    if (win->status == canvas_window::running)
    {
        (win->*method)(slot_id, param);
    }
    win->unlock();

    return 0;
}

void window::start (lua_State *L, gslshell::ret_status& st)
{
    this->lock();

    if (status != canvas_window::running)
    {
        typedef canvas_window::thread_info thread_info;
        std::auto_ptr<thread_info> inf(new thread_info(L, this));

        this->window_id = window_index_add (L, -1);
        inf->window_id = this->window_id;

        if (! this->start_new_thread (inf))
        {
            window_index_remove (L, this->window_id);
            this->unlock();
            st.error("error during thread initialization", "window creation");
        }
    }
    else
    {
        this->unlock();
        st.error("window is already active", "window creation");
    }
}

static void
show_window(lua_State* L, window* win)
{
    gslshell::ret_status st;
    win->start(L, st);

    if (st.error_msg())
        luaL_error (L, "%s (reported during %s)", st.error_msg(), st.context());
}

int
window_new (lua_State *L)
{
    window *win = push_new_object<window>(L, GS_WINDOW, global_state);
    const char *spec = lua_tostring (L, 1);
    int defer_show = lua_toboolean(L, 2);

    if (spec)
    {
        if (!win->split(spec))
            return luaL_error(L, "invalid layout specification");
    }

    if (!defer_show)
    {
        show_window(L, win);
    }

    return 1;
}

int
window_show (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    show_window(L, win);
    return 0;
}

int
window_free (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);

    /* it is important here to lock to ensure that there isn't any thread starting
       a new graphical window right now */
    win->lock();

    /* This should never happens. Running windows are never garbage collected
       and before closing lua_State all the graphical windows are closed and
       their threads have naturally finished. */
    assert(win->status != window::running);

    win->unlock();
    win->~window();
    return 0;
}

int
window_split (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    const char *spec = luaL_checkstring (L, 2);

    win->lock();

    refs_remove_function refs_remove_func(L, 1);
    win->plot_apply(refs_remove_func);

    if (! win->split(spec))
    {
        if (win->status == canvas_window::running)
            win->do_window_update();
        win->unlock();
        return luaL_error(L, "invalid window subdivision specification");
    }

    if (win->status == canvas_window::running)
    {
        win->on_draw();
        win->do_window_update();
    }

    win->unlock();
    return 0;
}

int
window_attach (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    sg_plot* plot = object_check<sg_plot>(L, 2, GS_PLOT);
    const char *spec = luaL_checkstring (L, 3);

    win->lock();

    int slot_id = win->attach (plot, spec);

    if (slot_id >= 0)
    {
        if (win->status == canvas_window::running)
            win->draw_slot(slot_id, true);
        win->unlock();
        window_refs_add (L, slot_id, 1, 2);
    }
    else
    {
        win->unlock();
        luaL_error (L, "invalid slot specification");
    }

    return 0;
}

int
window_slot_update (lua_State *L)
{
    return window_generic_oper_ext (L, &window::draw_slot, true);
}

int
window_slot_refresh (lua_State *L)
{
    return window_generic_oper_ext (L, &window::draw_slot, false);
}

int
window_update (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);

    win->lock();
    if (win->status == canvas_window::running)
    {
        win->on_draw();
        win->do_window_update();
    }
    win->unlock();

    return 0;
}

int
window_save_slot_image (lua_State *L)
{
    return window_generic_oper (L, &window::save_slot_image);
}

int
window_restore_slot_image (lua_State *L)
{
    return window_generic_oper (L, &window::restore_slot_image);
}

int
window_close (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    win->lock();
    if (win->status == canvas_window::running)
        win->close_request();
    win->unlock();
    return 0;
}

int
window_close_wait (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    win->shutdown_close();
    return 0;
}

class svg_writer {
public:
    svg_writer(FILE* f, double w, double h):
    m_canvas(f, h), m_width(w), m_height(h)
    { }

    void write_header() { m_canvas.write_header(m_width, m_height); }
    void write_end() { m_canvas.write_end(); }

    void call(window::ref* ref)
    {
        char plot_name[64];
        sg_plot* p = ref->plot;
        if (p)
        {
            agg::trans_affine mtx = ref->matrix;
            agg::trans_affine_scaling scale(m_width, m_height);
            trans_affine_compose(mtx, scale);
            sprintf(plot_name, "plot%u", ref->slot_id + 1);
            m_canvas.write_group_header(plot_name);
            p->draw(m_canvas, mtx, NULL);
            m_canvas.write_group_end(plot_name);
        }
    }

private:
    canvas_svg m_canvas;
    double m_width, m_height;
};

static int
window_save_svg_try(lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    const char *filename = lua_tostring(L, 2);
    const double w = luaL_optnumber(L, 3, 600.0);
    const double h = luaL_optnumber(L, 4, 600.0);

    if (!filename) return type_error_return(L, 2, "string");

    unsigned fnlen = strlen(filename);
    if (fnlen <= 4 || strcmp(filename + (fnlen - 4), ".svg") != 0)
    {
        const char* basename = (fnlen > 0 ? filename : "unnamed");
        lua_pushfstring(L, "%s.svg", basename);
        filename = lua_tostring(L, -1);
    }

    FILE* f = fopen(filename, "w");
    if (!f)
    {
        lua_pushfstring(L, "cannot open filename: %s", filename);
        return (-1);
    }

    svg_writer svg_writer(f, w, h);
    svg_writer.write_header();
    win->plot_apply(svg_writer);
    svg_writer.write_end();
    fclose(f);

    return 0;
}

int
window_save_svg(lua_State *L)
{
    int nret = window_save_svg_try(L);
    if (nret < 0) return lua_error(L);
    return nret;
}

void
window_register (lua_State *L)
{
    luaL_newmetatable (L, GS_METATABLE(GS_WINDOW));
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_register (L, NULL, window_methods);
    lua_pop (L, 1);

    /* gsl module registration */
    luaL_register (L, NULL, window_functions);
}
