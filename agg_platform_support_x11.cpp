//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
//
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#include <memory>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <new>

#include "pthreadpp.h"
#include "agg_basics.h"
#include "util/agg_color_conv_rgb8.h"
#include "agg-pixfmt-config.h"
#include "platform_support_ext.h"
#include "rect.h"

struct x_connection {
    Display*             display;
    int                  screen;
    int                  depth;
    Visual*              visual;

    x_connection() : display(0), m_busy(false) {};
    ~x_connection() {
        this->close();
    };

    bool init();
    void close();

    void busy(bool s) {
        m_busy = s;
    };

private:
    bool m_busy;
};

bool x_connection::init()
{
    display = XOpenDisplay(NULL);
    if(display == 0)
    {
        fprintf(stderr, "Unable to open DISPLAY!\n");
        return false;
    }

    screen = XDefaultScreen(display);
    depth  = XDefaultDepth (display, screen);
    visual = XDefaultVisual(display, screen);

    return true;
}

void x_connection::close()
{
    if (display && !m_busy)
    {
        XCloseDisplay(display);
        display = 0;
    }
}

class buffer_image {
    unsigned        m_width;
    unsigned        m_height;

    unsigned        m_bpp;
    unsigned char * m_buffer;
    XImage *        m_img;

public:
    buffer_image(unsigned bpp, unsigned byte_order,
                 unsigned width, unsigned height, x_connection *xc = 0);

    ~buffer_image()
    {
        delete [] m_buffer;
        if (m_img)
        {
            m_img->data = 0;
            XDestroyImage(m_img);
        }
    };

    void attach(agg::rendering_buffer& rbuf, bool flip_y);
    void resize(unsigned width, unsigned height);

    XImage * ximage() {
        return m_img;
    };
};

agg::pix_format_e gslshell::sys_pixel_format = agg::pix_format_undefined;
unsigned gslshell::sys_bpp = 0;

buffer_image::buffer_image(unsigned bpp, unsigned byte_order,
                           unsigned width, unsigned height, x_connection *xc)
{
    unsigned row_size = width * (bpp / 8);
    unsigned buf_size = height * row_size;

    m_buffer = new unsigned char[buf_size];

    m_width  = width;
    m_height = height;
    m_bpp    = bpp;

    if (xc)
    {
        m_img = XCreateImage(xc->display, xc->visual, xc->depth,
                             ZPixmap, 0, (char*) m_buffer,
                             m_width, m_height, m_bpp, row_size);
        m_img->byte_order = byte_order;
    }
    else
        m_img = 0;
}

void buffer_image::resize(unsigned width, unsigned height)
{
    m_width  = width;
    m_height = height;

    m_img->width  = width;
    m_img->height = height;
    m_img->bytes_per_line = width * (m_bpp / 8);
}

void buffer_image::attach(agg::rendering_buffer& rbuf, bool flip_y)
{
    unsigned row_size = m_width * (m_bpp / 8);
    rbuf.attach(m_buffer, m_width, m_height, flip_y ? -row_size : row_size);
}

namespace agg
{
//------------------------------------------------------------------------
class platform_specific
{
    typedef agg::rect_base<int> rect;

public:
    platform_specific(pix_format_e format, bool flip_y);
    ~platform_specific() {};

    void free_x_resources();

    void caption(const char* capt);
    void put_image(const rendering_buffer* src, const rect *r = 0);

    void send_close_request(x_connection *xc);
    void close_connections();

    pix_format_e         m_format;
    pix_format_e         m_sys_format;
    int                  m_byte_order;
    bool                 m_flip_y;
    unsigned             m_bpp;
    unsigned             m_sys_bpp;
    x_connection         m_main_conn;
    x_connection         m_draw_conn;
    Window               m_window;
    GC                   m_gc;
    XSetWindowAttributes m_window_attributes;
    Atom                 m_close_atom;
    Atom                 m_wm_protocols_atom;

    buffer_image *       m_main_img;
    buffer_image *       m_draw_img;

    unsigned char*       m_buf_img[platform_support::max_images];

    bool m_update_flag;
    bool m_resize_flag;
    bool m_initialized;
    bool m_is_mapped;
    clock_t m_sw_start;

    pthread::mutex m_mutex;

    static bool initialized;
};

//------------------------------------------------------------------------
platform_specific::platform_specific(pix_format_e format, bool flip_y) :
    m_format(format),
    m_sys_format(pix_format_undefined),
    m_byte_order(LSBFirst),
    m_flip_y(flip_y),
    m_bpp(0),
    m_sys_bpp(0),
    m_main_conn(),
    m_draw_conn(),
    m_window(0),
    m_gc(0),
    m_close_atom(0),
    m_wm_protocols_atom(0),
    m_main_img(0),
    m_draw_img(0),
    m_update_flag(true),
    m_resize_flag(true),
    m_initialized(false),
    m_is_mapped(false)
{
    memset(m_buf_img, 0, sizeof(m_buf_img));

    switch(m_format)
    {
    default:
        break;
    case pix_format_gray8:
        m_bpp = 8;
        break;

    case pix_format_rgb565:
    case pix_format_rgb555:
        m_bpp = 16;
        break;

    case pix_format_rgb24:
    case pix_format_bgr24:
        m_bpp = 24;
        break;

    case pix_format_bgra32:
    case pix_format_abgr32:
    case pix_format_argb32:
    case pix_format_rgba32:
        m_bpp = 32;
        break;
    }
    m_sw_start = clock();
}

void platform_specific::close_connections()
{
    m_draw_conn.close();
    m_main_conn.close();
}

void platform_specific::free_x_resources()
{
    if (m_main_img)
        delete m_main_img;
    if (m_draw_img)
        delete m_draw_img;
    XFreeGC(m_main_conn.display, m_gc);
    close_connections();
}

void platform_specific::send_close_request(x_connection *xc)
{
    XEvent ev;

    ev.xclient.type = ClientMessage;
    ev.xclient.window = m_window;
    ev.xclient.message_type = m_wm_protocols_atom;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = m_close_atom;
    ev.xclient.data.l[1] = CurrentTime;
    ev.xclient.data.l[2] = 0l;
    ev.xclient.data.l[3] = 0l;
    ev.xclient.data.l[4] = 0l;
    XSendEvent(xc->display, m_window, False, NoEventMask, &ev);
    XSync(xc->display, False);
}

//------------------------------------------------------------------------
void platform_specific::caption(const char* capt)
{
    // Fixed by Enno Fennema (in original AGG library)
    Display *d = m_main_conn.display;
    XStoreName(d, m_window, capt);
    XSetIconName(d, m_window, capt);
}


//------------------------------------------------------------------------
void platform_specific::put_image(const rendering_buffer* src, const rect *ri)
{
    rect r(0, 0, src->width(), src->height());

    if (ri)
        r = agg::intersect_rectangles(r, *ri);

    int w = r.x2 - r.x1, h = r.y2 - r.y1;

    rendering_buffer rbuf_draw;
    m_draw_img->resize(w, h);
    m_draw_img->attach(rbuf_draw, m_flip_y);

    rendering_buffer_ro src_view;
    rendering_buffer_get_const_view(src_view, *src, r, m_bpp / 8);

    if (m_format == m_sys_format)
    {
        rbuf_draw.copy_from(src_view);
    }
    else
    {
        switch(m_sys_format)
        {
        default:
            break;
        case pix_format_rgb555:
            switch(m_format)
            {
            default:
                break;
            case pix_format_rgb555:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb555_to_rgb555());
                break;
            case pix_format_rgb565:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb565_to_rgb555());
                break;
            case pix_format_rgb24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb24_to_rgb555());
                break;
            case pix_format_bgr24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgr24_to_rgb555());
                break;
            case pix_format_rgba32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgba32_to_rgb555());
                break;
            case pix_format_argb32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_argb32_to_rgb555());
                break;
            case pix_format_bgra32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgra32_to_rgb555());
                break;
            case pix_format_abgr32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_abgr32_to_rgb555());
                break;
            }
            break;

        case pix_format_rgb565:
            switch(m_format)
            {
            default:
                break;
            case pix_format_rgb555:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb555_to_rgb565());
                break;
            case pix_format_rgb565:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb565_to_rgb565());
                break;
            case pix_format_rgb24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb24_to_rgb565());
                break;
            case pix_format_bgr24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgr24_to_rgb565());
                break;
            case pix_format_rgba32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgba32_to_rgb565());
                break;
            case pix_format_argb32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_argb32_to_rgb565());
                break;
            case pix_format_bgra32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgra32_to_rgb565());
                break;
            case pix_format_abgr32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_abgr32_to_rgb565());
                break;
            }
            break;

        case pix_format_rgba32:
            switch(m_format)
            {
            default:
                break;
            case pix_format_rgb555:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb555_to_rgba32());
                break;
            case pix_format_rgb565:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb565_to_rgba32());
                break;
            case pix_format_rgb24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb24_to_rgba32());
                break;
            case pix_format_bgr24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgr24_to_rgba32());
                break;
            case pix_format_rgba32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgba32_to_rgba32());
                break;
            case pix_format_argb32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_argb32_to_rgba32());
                break;
            case pix_format_bgra32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgra32_to_rgba32());
                break;
            case pix_format_abgr32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_abgr32_to_rgba32());
                break;
            }
            break;

        case pix_format_abgr32:
            switch(m_format)
            {
            default:
                break;
            case pix_format_rgb555:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb555_to_abgr32());
                break;
            case pix_format_rgb565:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb565_to_abgr32());
                break;
            case pix_format_rgb24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb24_to_abgr32());
                break;
            case pix_format_bgr24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgr24_to_abgr32());
                break;
            case pix_format_abgr32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_abgr32_to_abgr32());
                break;
            case pix_format_rgba32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgba32_to_abgr32());
                break;
            case pix_format_argb32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_argb32_to_abgr32());
                break;
            case pix_format_bgra32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgra32_to_abgr32());
                break;
            }
            break;

        case pix_format_argb32:
            switch(m_format)
            {
            default:
                break;
            case pix_format_rgb555:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb555_to_argb32());
                break;
            case pix_format_rgb565:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb565_to_argb32());
                break;
            case pix_format_rgb24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb24_to_argb32());
                break;
            case pix_format_bgr24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgr24_to_argb32());
                break;
            case pix_format_rgba32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgba32_to_argb32());
                break;
            case pix_format_argb32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_argb32_to_argb32());
                break;
            case pix_format_abgr32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_abgr32_to_argb32());
                break;
            case pix_format_bgra32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgra32_to_argb32());
                break;
            }
            break;

        case pix_format_bgra32:
            switch(m_format)
            {
            default:
                break;
            case pix_format_rgb555:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb555_to_bgra32());
                break;
            case pix_format_rgb565:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb565_to_bgra32());
                break;
            case pix_format_rgb24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgb24_to_bgra32());
                break;
            case pix_format_bgr24:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgr24_to_bgra32());
                break;
            case pix_format_rgba32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_rgba32_to_bgra32());
                break;
            case pix_format_argb32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_argb32_to_bgra32());
                break;
            case pix_format_abgr32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_abgr32_to_bgra32());
                break;
            case pix_format_bgra32:
                my_color_conv(&rbuf_draw, &src_view, color_conv_bgra32_to_bgra32());
                break;
            }
            break;
        }
    }

    Display *dsp = m_draw_conn.display;

    int x_dst = r.x1, y_dst = (m_flip_y ? src->height() - (r.y1 + h) : r.y1);
    XPutImage(dsp, m_window, m_gc, m_draw_img->ximage(),
              0, 0, x_dst, y_dst, w, h);
}

bool platform_specific::initialized = false;

//------------------------------------------------------------------------
platform_support::platform_support(pix_format_e format, bool flip_y) :
    m_specific(new platform_specific(format, flip_y)),
    m_format(format),
    m_bpp(m_specific->m_bpp),
    m_window_flags(0),
    m_wait_mode(true),
    m_flip_y(flip_y),
    m_initial_width(10),
    m_initial_height(10)
{
    strcpy(m_caption, "AGG Application");
}

//------------------------------------------------------------------------
platform_support::~platform_support()
{
    delete m_specific;
}



//------------------------------------------------------------------------
void platform_support::caption(const char* cap)
{
    strcpy(m_caption, cap);
    if(m_specific->m_initialized)
    {
        m_specific->caption(cap);
    }
}


//------------------------------------------------------------------------
enum xevent_mask_e
{
    xevent_mask =
        ExposureMask|
        StructureNotifyMask
};


//------------------------------------------------------------------------
bool platform_support::init(unsigned width, unsigned height, unsigned flags)
{
    m_window_flags = flags;

    if (!m_specific->m_main_conn.init())
        return false;

    if (!m_specific->m_draw_conn.init())
    {
        m_specific->close_connections();
        return false;
    }

    x_connection *xc = &m_specific->m_main_conn;
    x_connection *dc = &m_specific->m_draw_conn;

    unsigned long r_mask = xc->visual->red_mask;
    unsigned long g_mask = xc->visual->green_mask;
    unsigned long b_mask = xc->visual->blue_mask;

    if(xc->depth < 15 || r_mask == 0 || g_mask == 0 || b_mask == 0)
    {
        fprintf(stderr,
                "There's no Visual compatible with minimal AGG requirements:\n"
                "At least 15-bit color depth and True- or DirectColor class.\n\n");
        m_specific->close_connections();
        return false;
    }

    int t = 1;
    int hw_byte_order = LSBFirst;
    if(*(char*)&t == 0) hw_byte_order = MSBFirst;

    // Perceive SYS-format by mask
    switch(xc->depth)
    {
    case 15:
        m_specific->m_sys_bpp = 16;
        if(r_mask == 0x7C00 && g_mask == 0x3E0 && b_mask == 0x1F)
        {
            m_specific->m_sys_format = pix_format_rgb555;
            m_specific->m_byte_order = hw_byte_order;
        }
        break;

    case 16:
        m_specific->m_sys_bpp = 16;
        if(r_mask == 0xF800 && g_mask == 0x7E0 && b_mask == 0x1F)
        {
            m_specific->m_sys_format = pix_format_rgb565;
            m_specific->m_byte_order = hw_byte_order;
        }
        break;

    case 24:
    case 32:
        m_specific->m_sys_bpp = 32;
        if(g_mask == 0xFF00)
        {
            if(r_mask == 0xFF && b_mask == 0xFF0000)
            {
                switch(m_specific->m_format)
                {
                case pix_format_rgba32:
                    m_specific->m_sys_format = pix_format_rgba32;
                    m_specific->m_byte_order = LSBFirst;
                    break;

                case pix_format_abgr32:
                    m_specific->m_sys_format = pix_format_abgr32;
                    m_specific->m_byte_order = MSBFirst;
                    break;

                default:
                    m_specific->m_byte_order = hw_byte_order;
                    m_specific->m_sys_format =
                        (hw_byte_order == LSBFirst) ?
                        pix_format_rgba32 :
                        pix_format_abgr32;
                    break;
                }
            }

            if(r_mask == 0xFF0000 && b_mask == 0xFF)
            {
                switch(m_specific->m_format)
                {
                case pix_format_argb32:
                    m_specific->m_sys_format = pix_format_argb32;
                    m_specific->m_byte_order = MSBFirst;
                    break;

                case pix_format_bgra32:
                    m_specific->m_sys_format = pix_format_bgra32;
                    m_specific->m_byte_order = LSBFirst;
                    break;

                default:
                    m_specific->m_byte_order = hw_byte_order;
                    m_specific->m_sys_format =
                        (hw_byte_order == MSBFirst) ?
                        pix_format_argb32 :
                        pix_format_bgra32;
                    break;
                }
            }
        }
        break;
    }

    if(m_specific->m_sys_format == pix_format_undefined)
    {
        fprintf(stderr,
                "RGB masks are not compatible with AGG pixel formats:\n"
                "R=%08x, R=%08x, B=%08x\n",
                (unsigned)r_mask, (unsigned)g_mask, (unsigned)b_mask);

        m_specific->close_connections();
        return false;
    }

    gslshell::sys_pixel_format = m_specific->m_sys_format;
    gslshell::sys_bpp = m_specific->m_sys_bpp;

    XSetWindowAttributes *win_attr = &m_specific->m_window_attributes;

    memset(win_attr, 0, sizeof(XSetWindowAttributes));

    win_attr->override_redirect = 0;

    unsigned long window_mask = 0;

    m_specific->m_window =
        XCreateWindow(xc->display, XDefaultRootWindow(xc->display),
                      0, 0, width, height,
                      0, xc->depth, InputOutput, CopyFromParent,
                      window_mask, win_attr);

    m_specific->m_gc = XCreateGC(xc->display, m_specific->m_window, 0, 0);

    m_specific->m_main_img =
        new(std::nothrow) buffer_image(m_specific->m_bpp, m_specific->m_byte_order, width, height);
    m_specific->m_draw_img =
        new(std::nothrow) buffer_image(m_specific->m_sys_bpp, m_specific->m_byte_order, width, height, dc);

    if (m_specific->m_main_img == 0 || m_specific->m_draw_img == 0)
    {
        XFreeGC(xc->display, m_specific->m_gc);
        XDestroyWindow(xc->display, m_specific->m_window);
        m_specific->close_connections();
        return false;
    }

    m_specific->m_main_img->attach(m_rbuf_window, m_flip_y);

    m_specific->caption(m_caption);
    m_initial_width = width;
    m_initial_height = height;

    if(!m_specific->m_initialized)
    {
        on_init();
        m_specific->m_initialized = true;
    }

    trans_affine_resizing(width, height);
    on_resize(width, height);
    m_specific->m_update_flag = true;

    XSizeHints *hints = XAllocSizeHints();
    if(hints)
    {
        if(flags & window_resize)
        {
            hints->min_width = 32;
            hints->min_height = 32;
            hints->max_width = 4096;
            hints->max_height = 4096;
        }
        else
        {
            hints->min_width  = width;
            hints->min_height = height;
            hints->max_width  = width;
            hints->max_height = height;
        }
        hints->flags = PMaxSize | PMinSize;

        XSetWMNormalHints(xc->display, m_specific->m_window, hints);

        XFree(hints);
    }


    XMapWindow(xc->display, m_specific->m_window);

    XSelectInput(xc->display, m_specific->m_window, xevent_mask);

    m_specific->m_close_atom = XInternAtom(xc->display, "WM_DELETE_WINDOW", false);

    m_specific->m_wm_protocols_atom = XInternAtom(xc->display, "WM_PROTOCOLS", true);

    XSetWMProtocols(xc->display, m_specific->m_window, &m_specific->m_close_atom, 1);

    return true;
}

//------------------------------------------------------------------------
void platform_support::update_window()
{
    if (! m_specific->m_is_mapped)
        return;

    x_connection *xc = &m_specific->m_draw_conn;

    m_specific->put_image(&m_rbuf_window);

    // When m_wait_mode is true we can discard all the events
    // came while the image is being drawn. In this case
    // the X server does not accumulate mouse motion events.
    // When m_wait_mode is false, i.e. we have some idle drawing
    // we cannot afford to miss any events
    XSync(xc->display, m_wait_mode);
}

static Bool is_configure_or_expose(Display *d, XEvent *ev, XPointer arg)
{
    return (ev->type == ConfigureNotify || ev->type == Expose) ? True : False;
}

struct loop_state {
    XEvent config;
    XEvent expose;

    loop_state() {
        config.type = 0;
        expose.type = 0;
    }
};

static XEvent x_get_event(Display *d, loop_state& current)
{
    XEvent xev;
    if (current.expose.type == Expose) {
        xev = current.expose;
        current.expose.type = 0;
        return xev;
    }
    while (XCheckIfEvent(d, &xev, is_configure_or_expose, NULL) == True) {
        if (xev.type == ConfigureNotify) {
            current.config = xev;
        } else {
            current.expose = xev;
        }
    }
    if (current.config.type == ConfigureNotify) {
        xev = current.config;
        current.config.type = 0;
    } else if (current.expose.type == Expose) {
        xev = current.expose;
        current.expose.type = 0;
    } else {
        XNextEvent(d, &xev);
    }
    return xev;
}

//------------------------------------------------------------------------
int platform_support::run()
{
    x_connection *xc = &m_specific->m_main_conn;
    x_connection *dc = &m_specific->m_draw_conn;

    XFlush(xc->display);

    bool quit = false;
    int ret = 0;

    platform_specific *ps = m_specific;

    loop_state event_loop;
    while(!quit)
    {
        if(ps->m_update_flag && ps->m_is_mapped)
        {
            on_draw();
            update_window();
            ps->m_update_flag = false;
        }

        xc->busy(true);

        XEvent x_event;
        if (ps->m_is_mapped)
        {
            ps->m_mutex.unlock();
            x_event = x_get_event(xc->display, event_loop);
            ps->m_mutex.lock();
        }
        else
        {
            XNextEvent(xc->display, &x_event);
        }

        xc->busy(false);

        switch(x_event.type)
        {
        case MapNotify:
        {
            on_draw();
            update_window();
            ps->m_is_mapped = true;
            ps->m_update_flag = false;
        }
        break;

        case ConfigureNotify:
        {
            if(x_event.xconfigure.width  != int(m_rbuf_window.width()) ||
                    x_event.xconfigure.height != int(m_rbuf_window.height()))
            {
                int width  = x_event.xconfigure.width;
                int height = x_event.xconfigure.height;

                delete ps->m_main_img;
                delete ps->m_draw_img;

                ps->m_main_img = new(std::nothrow) buffer_image(ps->m_bpp, ps->m_byte_order, width, height);
                ps->m_draw_img = new(std::nothrow) buffer_image(ps->m_sys_bpp, ps->m_byte_order, width, height, dc);

                if (ps->m_main_img == 0 || ps->m_draw_img == 0)
                {
                    if (ps->m_main_img) delete ps->m_main_img;
                    if (ps->m_draw_img) delete ps->m_draw_img;
                    quit = true;
                    ret = 1;
                    break;
                }

                ps->m_main_img->attach(m_rbuf_window, m_flip_y);

                trans_affine_resizing(width, height);
                on_resize(width, height);
                on_draw();
            }
        }
        break;

        case Expose:
            if (x_event.xexpose.count > 0) {
                break;
            }
            xc->busy(true);
            ps->put_image(&m_rbuf_window);
            XFlush(xc->display);
            XSync(xc->display, false);
            xc->busy(false);
            break;

        case ClientMessage:
            if((x_event.xclient.format == 32) &&
                    (x_event.xclient.data.l[0] == int(ps->m_close_atom)))
            {
                quit = true;
            }
            break;
        }
    }

    unsigned i = platform_support::max_images;
    while(i--)
    {
        if(ps->m_buf_img[i])
        {
            delete [] ps->m_buf_img[i];
        }
    }

    ps->m_is_mapped = false;
    ps->free_x_resources();

    return ret;
}



//------------------------------------------------------------------------
const char* platform_support::img_ext() const {
    return ".ppm";
}

//------------------------------------------------------------------------
const char* platform_support::full_file_name(const char* file_name)
{
    return file_name;
}

//------------------------------------------------------------------------
bool platform_support::load_img(unsigned idx, const char* file)
{
    if(idx < max_images)
    {
        char buf[1024];
        strcpy(buf, file);
        int len = strlen(buf);
        if(len < 4 || strcasecmp(buf + len - 4, ".ppm") != 0)
        {
            strcat(buf, ".ppm");
        }

        FILE* fd = fopen(buf, "rb");
        if(fd == 0) return false;

        if((len = fread(buf, 1, 1022, fd)) == 0)
        {
            fclose(fd);
            return false;
        }
        buf[len] = 0;

        if(buf[0] != 'P' && buf[1] != '6')
        {
            fclose(fd);
            return false;
        }

        char* ptr = buf + 2;

        while(*ptr && !isdigit(*ptr)) ptr++;
        if(*ptr == 0)
        {
            fclose(fd);
            return false;
        }

        unsigned width = atoi(ptr);
        if(width == 0 || width > 4096)
        {
            fclose(fd);
            return false;
        }
        while(*ptr && isdigit(*ptr)) ptr++;
        while(*ptr && !isdigit(*ptr)) ptr++;
        if(*ptr == 0)
        {
            fclose(fd);
            return false;
        }
        unsigned height = atoi(ptr);
        if(height == 0 || height > 4096)
        {
            fclose(fd);
            return false;
        }
        while(*ptr && isdigit(*ptr)) ptr++;
        while(*ptr && !isdigit(*ptr)) ptr++;
        if(atoi(ptr) != 255)
        {
            fclose(fd);
            return false;
        }
        while(*ptr && isdigit(*ptr)) ptr++;
        if(*ptr == 0)
        {
            fclose(fd);
            return false;
        }
        ptr++;
        fseek(fd, long(ptr - buf), SEEK_SET);

        create_img(idx, width, height);
        bool ret = true;

        if(m_format == pix_format_rgb24)
        {
            fread(m_specific->m_buf_img[idx], 1, width * height * 3, fd);
        }
        else
        {
            unsigned char* buf_img = new unsigned char [width * height * 3];
            rendering_buffer rbuf_img;
            rbuf_img.attach(buf_img,
                            width,
                            height,
                            m_flip_y ?
                            -width * 3 :
                            width * 3);

            fread(buf_img, 1, width * height * 3, fd);

            switch(m_format)
            {
            case pix_format_rgb555:
                color_conv(m_rbuf_img+idx, &rbuf_img, color_conv_rgb24_to_rgb555());
                break;

            case pix_format_rgb565:
                color_conv(m_rbuf_img+idx, &rbuf_img, color_conv_rgb24_to_rgb565());
                break;

            case pix_format_bgr24:
                color_conv(m_rbuf_img+idx, &rbuf_img, color_conv_rgb24_to_bgr24());
                break;

            case pix_format_rgba32:
                color_conv(m_rbuf_img+idx, &rbuf_img, color_conv_rgb24_to_rgba32());
                break;

            case pix_format_argb32:
                color_conv(m_rbuf_img+idx, &rbuf_img, color_conv_rgb24_to_argb32());
                break;

            case pix_format_bgra32:
                color_conv(m_rbuf_img+idx, &rbuf_img, color_conv_rgb24_to_bgra32());
                break;

            case pix_format_abgr32:
                color_conv(m_rbuf_img+idx, &rbuf_img, color_conv_rgb24_to_abgr32());
                break;

            default:
                ret = false;
            }
            delete [] buf_img;
        }

        fclose(fd);
        return ret;
    }
    return false;
}


//------------------------------------------------------------------------
bool platform_support::save_img(unsigned idx, const char* file)
{
    return false;
}


//------------------------------------------------------------------------
bool platform_support::create_img(unsigned idx, unsigned width, unsigned height)
{
    if(idx < max_images)
    {
        if(width  == 0) width  = rbuf_window().width();
        if(height == 0) height = rbuf_window().height();
        delete [] m_specific->m_buf_img[idx];
        m_specific->m_buf_img[idx] =
            new unsigned char[width * height * (m_bpp / 8)];

        m_rbuf_img[idx].attach(m_specific->m_buf_img[idx],
                               width,
                               height,
                               m_flip_y ?
                               -width * (m_bpp / 8) :
                               width * (m_bpp / 8));
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
void platform_support::force_redraw()
{
    m_specific->m_update_flag = true;
}


//------------------------------------------------------------------------
void platform_support::message(const char* msg)
{
    fprintf(stderr, "%s\n", msg);
}

//------------------------------------------------------------------------
void platform_support::start_timer()
{
    m_specific->m_sw_start = clock();
}

//------------------------------------------------------------------------
double platform_support::elapsed_time() const
{
    clock_t stop = clock();
    return double(stop - m_specific->m_sw_start) * 1000.0 / CLOCKS_PER_SEC;
}


//------------------------------------------------------------------------
void platform_support::on_init() {}
void platform_support::on_resize(int sx, int sy) {}
void platform_support::on_idle() {}
void platform_support::on_mouse_move(int x, int y, unsigned flags) {}
void platform_support::on_mouse_button_down(int x, int y, unsigned flags) {}
void platform_support::on_mouse_button_up(int x, int y, unsigned flags) {}
void platform_support::on_key(int x, int y, unsigned key, unsigned flags) {}
void platform_support::on_ctrl_change() {}
void platform_support::on_draw() {}
void platform_support::on_post_draw(void* raw_handler) {}
}

#if defined(LIBGRAPH_DEBUG)
#include <unistd.h>

static int
my_x_error_handler (Display *d, XErrorEvent *e)
{
    char err_msg[256];
    printf("WARNING: X protocol error.\n");
    XGetErrorText(d, e->error_code, err_msg, 256);
    printf("%s\n", err_msg);
    printf("waiting");
    for (int k = 0; k < 60 * 15; k++)
    {
        sleep(1);
        printf(".");
        fflush(stdout);
    }
    return 0;
}
#endif

void
platform_support_ext::prepare()
{
    if (! agg::platform_specific::initialized)
    {
        XInitThreads();
#if defined(LIBGRAPH_DEBUG)
        XSetErrorHandler(my_x_error_handler);
#endif
        agg::platform_specific::initialized = true;
    }
}

void
platform_support_ext::lock()
{
    m_specific->m_mutex.lock();
}

void
platform_support_ext::unlock()
{
    m_specific->m_mutex.unlock();
}

bool
platform_support_ext::is_mapped()
{
    return m_specific->m_is_mapped;
}

void
platform_support_ext::close_request()
{
    m_specific->send_close_request(&m_specific->m_draw_conn);
}

void
platform_support_ext::update_region (const agg::rect_base<int>& r)
{
    if (! m_specific->m_is_mapped)
        return;

    x_connection *xd = &m_specific->m_draw_conn;

    m_specific->put_image(&rbuf_window(), &r);

    // When m_wait_mode is true we can discard all the events
    // came while the image is being drawn. In this case
    // the X server does not accumulate mouse motion events.
    // When m_wait_mode is false, i.e. we have some idle drawing
    // we cannot afford to miss any events
    XSync(xd->display, wait_mode());
}

void
platform_support_ext::do_window_update()
{
    agg::rect_base<int> r(0, 0, rbuf_window().width(), rbuf_window().height());
    update_region(r);
}
