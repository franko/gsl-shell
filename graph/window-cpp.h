
extern "C" {
#include "lua.h"
}

#include "window.h"
#include "canvas-window-cpp.h"
#include "resource-manager.h"
#include "lua-plot-cpp.h"
#include "lua-cpp-utils.h"
#include "plot.h"
#include "rect.h"
#include "list.h"

#include "agg_color_rgba.h"
#include "agg_trans_affine.h"
#include "split-parser.h"

class window : public canvas_window {
public:
    int window_id;

    typedef agg::trans_affine bmatrix;

    struct ref {
        typedef tree::node<ref, direction_e> node;

        sg_plot* plot;
        int slot_id;

        plot_render_info inf;
        bmatrix matrix;

        unsigned char *layer_buf;
        agg::rendering_buffer layer_img;

        bool valid_rect;
        opt_rect<double> dirty_rect;

        ref(sg_plot* p = 0)
            : plot(p), matrix(), layer_buf(0), valid_rect(true), dirty_rect()
        {};

        ~ref() { delete[] layer_buf; }

        void dispose_buffer()
        {
            valid_rect = false;
            delete[] layer_buf;
            layer_buf = 0;
        }

        void save_image (agg::rendering_buffer& winbuf, agg::rect_base<int>& r,
                         int bpp, bool flip_y);

        static void compose(bmatrix& a, const bmatrix& b);
        static int calculate(node *t, const bmatrix& m, int id);
    };

private:
    void draw_slot_by_ref(ref& ref, bool dirty);
    void refresh_slot_by_ref(ref& ref, bool draw_all);
    void cleanup_tree_rec (lua_State *L, int window_index, ref::node* n);

    static ref *ref_lookup (ref::node *p, int slot_id);

    template <class Function>
    void plot_apply_rec(Function& f, ref::node* n);

    ref::node* m_tree;

public:
    window(agg::rgba8 bgcol= colors::white):
        canvas_window(bgcol), m_tree(0)
    {
        this->split(".");
    }

    ~window() { delete m_tree; }

    template <class Function> void plot_apply(Function& f) { this->plot_apply_rec(f, m_tree); }

    bool split(const char *spec);
    int attach(sg_plot *plot, const char *spec);
    void draw_slot(int slot_id, bool update_req);
    void refresh_slot(int slot_id);
    void start(lua_State *L, gslshell::ret_status& st);

    void save_slot_image(int slot_id);
    void restore_slot_image(int slot_id);

    void draw_slot(int slot_id);

    virtual void on_draw();
    virtual void on_resize(int sx, int sy);

private:
    struct slot_draw_function
    {
        slot_draw_function(window* w): win(w) { }
        void call(window::ref* ref) { win->draw_slot_by_ref(*ref, false); }
        window* win;
    };

    struct dispose_buffer_function {
        void call(window::ref* ref) { ref->dispose_buffer(); }
    };
};
