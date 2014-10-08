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

#include <windows.h>
#include <string.h>
#include <new>
#include "pthreadpp.h"
#include "pixel_fmt.h"
#include "agg-pixfmt-config.h"
#include "platform_support_ext.h"
#include "platform/win32/agg_win32_bmp.h"
#include "util/agg_color_conv_rgb8.h"
#include "util/agg_color_conv_rgb16.h"

#include "rect.h"

namespace agg
{

static inline void pixel_map_attach (pixel_map& pm, rendering_buffer *rbuf,
                                     bool flip_y)
{
    int stride = pm.stride();
    rbuf->attach(pm.buf(), pm.width(), pm.height(), flip_y ? stride : -stride);
}

//------------------------------------------------------------------------
HINSTANCE g_windows_instance = 0;
int       g_windows_cmd_show = 0;


//------------------------------------------------------------------------
class platform_specific
{
public:
    platform_specific(pix_format_e format, bool flip_y);
    ~platform_specific();

    void create_pmap(unsigned width, unsigned height,
                     rendering_buffer* wnd);

    void display_pmap(HDC dc, const rendering_buffer* src,
                      const agg::rect_base<int> *rect = 0);

    bool save_pmap(const char* fn, unsigned idx,
                   const rendering_buffer* src);

    void close();

    pix_format_e  m_format;
    pix_format_e  m_sys_format;
    bool          m_flip_y;
    unsigned      m_bpp;
    unsigned      m_sys_bpp;
    HWND          m_hwnd;
    pixel_map     m_pmap_window;
    BITMAPINFO*   m_bmp_draw;
    pixel_map     m_pmap_img[platform_support::max_images];
    int           m_cur_x;
    int           m_cur_y;
    unsigned      m_input_flags;
    bool          m_redraw_flag;
    HDC           m_current_dc;
    LARGE_INTEGER m_sw_freq;
    LARGE_INTEGER m_sw_start;

    bool m_is_mapped;
    bool m_is_ready;

    pthread::mutex m_mutex;

    static void bitmap_info_resize (BITMAPINFO* bmp, unsigned w, unsigned h);
};


//------------------------------------------------------------------------
platform_specific::platform_specific(pix_format_e format, bool flip_y) :
    m_format(gslshell::pixel_format),
    m_sys_format(gslshell::sys_pixel_format),
    m_flip_y(gslshell::flip_y),
    m_bpp(LIBGRAPH_BPP),
    m_sys_bpp(gslshell::sys_bpp),
    m_hwnd(0),
    m_bmp_draw(0),
    m_cur_x(0),
    m_cur_y(0),
    m_input_flags(0),
    m_redraw_flag(true),
    m_current_dc(0),
    m_is_mapped(false),
    m_is_ready(false)
{
    ::QueryPerformanceFrequency(&m_sw_freq);
    ::QueryPerformanceCounter(&m_sw_start);
}

//------------------------------------------------------------------------
platform_specific::~platform_specific()
{
    if (m_bmp_draw)
        delete [] (unsigned char*) m_bmp_draw;
}

void platform_specific::close()
{
    ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}


//------------------------------------------------------------------------
void platform_specific::create_pmap(unsigned width,
                                    unsigned height,
                                    rendering_buffer* wnd)
{
    m_pmap_window.create(width, height, org_e(m_bpp));
    pixel_map_attach (m_pmap_window, wnd, m_flip_y);

    if (m_bmp_draw)
        delete [] (unsigned char*) m_bmp_draw;
    m_bmp_draw = pixel_map::create_bitmap_info(width, height,
                 org_e(m_sys_bpp));
}

//------------------------------------------------------------------------
void platform_specific::display_pmap(HDC dc, const rendering_buffer* src,
                                     const agg::rect_base<int> *ri)
{
    if(m_sys_format == m_format && ri == 0)
    {
        m_pmap_window.draw(dc);
    }
    else
    {
        agg::rect_base<int> r(0, 0, src->width(), src->height());
        if (ri)
            r = agg::intersect_rectangles(r, *ri);

        int w = r.x2 - r.x1, h = r.y2 - r.y1;

        bitmap_info_resize (m_bmp_draw, w, h);

        pixel_map pmap;
        pmap.attach_to_bmp(m_bmp_draw);

        rendering_buffer rbuf_tmp;
        pixel_map_attach (pmap, &rbuf_tmp, m_flip_y);

        rendering_buffer_ro src_view;
        rendering_buffer_get_const_view(src_view, *src, r, m_bpp / 8);

        if (m_format == m_sys_format)
        {
            rbuf_tmp.copy_from(src_view);
        }
        else
        {
            if (m_sys_format == pix_format_bgr24 && m_format == pix_format_rgb24)
            {
                my_color_conv(&rbuf_tmp, &src_view, color_conv_rgb24_to_bgr24());
            }
        }

        unsigned int wh = m_pmap_window.height();
        RECT wrect;
        wrect.left   = r.x1;
        wrect.right  = r.x2;
        wrect.bottom = wh - r.y1;
        wrect.top    = wh - r.y2;

        RECT brect;
        brect.left   = 0;
        brect.right  = w;
        brect.bottom = h;
        brect.top    = 0;

        pmap.draw(dc, &wrect, &brect);
    }
}



//------------------------------------------------------------------------
bool platform_specific::save_pmap(const char* fn, unsigned idx,
                                  const rendering_buffer* src)
{
    pixel_map& img = m_pmap_img[idx];
    return img.save_as_bmp(fn);
}

void platform_specific::bitmap_info_resize (BITMAPINFO* bmp, unsigned w, unsigned h)
{
    if (w == 0) w = 1;
    if (h == 0) h = 1;

    unsigned bits_per_pixel = bmp->bmiHeader.biBitCount;
    unsigned row_len = agg::pixel_map::calc_row_len(w, bits_per_pixel);

    bmp->bmiHeader.biWidth  = w;
    bmp->bmiHeader.biHeight = h;
    bmp->bmiHeader.biSizeImage = row_len * h;
}


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
    strcpy(m_caption, "Anti-Grain Geometry Application");
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
    if(m_specific->m_hwnd)
    {
        SetWindowText(m_specific->m_hwnd, m_caption);
    }
}

//------------------------------------------------------------------------
void platform_support::start_timer()
{
    ::QueryPerformanceCounter(&(m_specific->m_sw_start));
}

//------------------------------------------------------------------------
double platform_support::elapsed_time() const
{
    LARGE_INTEGER stop;
    ::QueryPerformanceCounter(&stop);
    return double(stop.QuadPart -
                  m_specific->m_sw_start.QuadPart) * 1000.0 /
           double(m_specific->m_sw_freq.QuadPart);
}

void* platform_support::raw_display_handler()
{
    return m_specific->m_current_dc;
}


//------------------------------------------------------------------------
LRESULT CALLBACK window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC paintDC;

    void* user_data = reinterpret_cast<void*>(::GetWindowLong(hWnd, GWL_USERDATA));
    platform_support* app = 0;

    if(user_data)
    {
        app = reinterpret_cast<platform_support*>(user_data);
    }

    if(app == 0)
    {
        if(msg == WM_DESTROY)
        {
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }

    HDC dc = ::GetDC(app->m_specific->m_hwnd);
    app->m_specific->m_current_dc = dc;
    LRESULT ret = 0;

    switch(msg)
    {
        //--------------------------------------------------------------------
    case WM_CREATE:
        break;

        //--------------------------------------------------------------------
    case WM_SIZE:
        app->m_specific->create_pmap(LOWORD(lParam), HIWORD(lParam),
                                     &app->rbuf_window());

        app->trans_affine_resizing(LOWORD(lParam), HIWORD(lParam));
        app->on_resize(LOWORD(lParam), HIWORD(lParam));
        app->force_redraw();
        app->m_specific->m_is_ready = false;

        break;

        //--------------------------------------------------------------------
    case WM_ERASEBKGND:
        break;

        //--------------------------------------------------------------------
    case WM_PAINT:
        paintDC = ::BeginPaint(hWnd, &ps);
        app->m_specific->m_current_dc = paintDC;
        if(app->m_specific->m_redraw_flag)
        {
            app->on_draw();
            app->m_specific->m_redraw_flag = false;
        }
        app->m_specific->display_pmap(paintDC, &app->rbuf_window());
        app->on_post_draw(paintDC);
        app->m_specific->m_current_dc = 0;
        ::EndPaint(hWnd, &ps);

        app->m_specific->m_is_mapped = true;
        app->m_specific->m_is_ready = true;
        break;

        //--------------------------------------------------------------------
    case WM_COMMAND:
        break;

        //--------------------------------------------------------------------
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;

        //--------------------------------------------------------------------
    default:
        ret = ::DefWindowProc(hWnd, msg, wParam, lParam);
        break;
    }
    app->m_specific->m_current_dc = 0;
    ::ReleaseDC(app->m_specific->m_hwnd, dc);

    return ret;
}


//------------------------------------------------------------------------
void platform_support::message(const char* msg)
{
    ::MessageBox(m_specific->m_hwnd, msg, "AGG Message", MB_OK);
}


//------------------------------------------------------------------------
bool platform_support::init(unsigned width, unsigned height, unsigned flags)
{
    if(m_specific->m_sys_format == pix_format_undefined)
    {
        return false;
    }

    m_window_flags = flags;

    int wflags = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;

    WNDCLASS wc;
    wc.lpszClassName = "AGGAppClass";
    wc.lpfnWndProc = window_proc;
    wc.style = wflags;
    wc.hInstance = g_windows_instance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszMenuName = "AGGAppMenu";
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    ::RegisterClass(&wc);

    wflags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    if(m_window_flags & window_resize)
    {
        wflags |= WS_THICKFRAME | WS_MAXIMIZEBOX;
    }

    m_specific->m_hwnd = ::CreateWindow("AGGAppClass",
                                        m_caption,
                                        wflags,
                                        100,
                                        100,
                                        width,
                                        height,
                                        0,
                                        0,
                                        g_windows_instance,
                                        0);

    if(m_specific->m_hwnd == 0)
    {
        return false;
    }

    RECT rct;
    ::GetClientRect(m_specific->m_hwnd, &rct);

    ::MoveWindow(m_specific->m_hwnd,   // handle to window
                 100,                  // horizontal position
                 100,                  // vertical position
                 width + (width - (rct.right - rct.left)),
                 height + (height - (rct.bottom - rct.top)),
                 FALSE);

    ::SetWindowLong(m_specific->m_hwnd, GWL_USERDATA, (LONG)this);
    m_specific->create_pmap(width, height, &m_rbuf_window);
    m_initial_width = width;
    m_initial_height = height;
    on_init();
    m_specific->m_redraw_flag = true;
    ::ShowWindow(m_specific->m_hwnd, g_windows_cmd_show);
    ::SetForegroundWindow(m_specific->m_hwnd);
    return true;
}



//------------------------------------------------------------------------
int platform_support::run()
{
    MSG msg;

    for(;;)
    {
        if(m_wait_mode)
        {
            bool status;

            if (m_specific->m_is_ready)
            {
                m_specific->m_mutex.unlock();
                status = ::GetMessage(&msg, 0, 0, 0);
                m_specific->m_mutex.lock();
            }
            else
            {
                status = ::GetMessage(&msg, 0, 0, 0);
            }

            if(! status)
            {
                break;
            }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else
        {
            if(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                if(msg.message == WM_QUIT)
                {
                    m_specific->m_is_mapped = false;
                    break;
                }
                ::DispatchMessage(&msg);
            }
            else
            {
                on_idle();
            }
        }
    }

    return (int)msg.wParam;
}


//------------------------------------------------------------------------
const char* platform_support::img_ext() const {
    return ".bmp";
}


//------------------------------------------------------------------------
const char* platform_support::full_file_name(const char* file_name)
{
    return file_name;
}

//------------------------------------------------------------------------
bool platform_support::load_img(unsigned idx, const char* file)
{
    return false;
}



//------------------------------------------------------------------------
bool platform_support::save_img(unsigned idx, const char* file)
{
    if(idx < max_images)
    {
        char fn[1024];
        strcpy(fn, file);
        int len = strlen(fn);
        if(len < 4 || stricmp(fn + len - 4, ".BMP") != 0)
        {
            strcat(fn, ".bmp");
        }
        return m_specific->save_pmap(fn, idx, &m_rbuf_img[idx]);
    }
    return true;
}



//------------------------------------------------------------------------
bool platform_support::create_img(unsigned idx, unsigned width, unsigned height)
{
    if(idx < max_images)
    {
        if(width  == 0) width  = m_specific->m_pmap_window.width();
        if(height == 0) height = m_specific->m_pmap_window.height();

        pixel_map& pmap = m_specific->m_pmap_img[idx];
        pmap.create(width, height, org_e(m_specific->m_bpp));
        pixel_map_attach (pmap, &m_rbuf_img[idx], m_flip_y);
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
void platform_support::force_redraw()
{
    m_specific->m_redraw_flag = true;
    ::InvalidateRect(m_specific->m_hwnd, 0, FALSE);
}



//------------------------------------------------------------------------
void platform_support::update_window()
{
    HDC dc = ::GetDC(m_specific->m_hwnd);
    m_specific->display_pmap(dc, &m_rbuf_window);
    ::ReleaseDC(m_specific->m_hwnd, dc);
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

agg::pix_format_e gslshell::sys_pixel_format = agg::pix_format_bgr24;
unsigned gslshell::sys_bpp = 24;

void
platform_support_ext::prepare()
{
    if (agg::g_windows_instance == 0)
    {
        agg::g_windows_instance = GetModuleHandle(NULL);
        agg::g_windows_cmd_show = SW_SHOWNORMAL;
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
    m_specific->close();
}

void
platform_support_ext::update_region (const agg::rect_base<int>& r)
{
    if (! m_specific->m_is_mapped)
        return;

    HDC dc = ::GetDC(m_specific->m_hwnd);
    m_specific->display_pmap(dc, &rbuf_window(), &r);
    ::ReleaseDC(m_specific->m_hwnd, dc);
}

void
platform_support_ext::do_window_update()
{
    agg::rect_base<int> r(0, 0, rbuf_window().width(), rbuf_window().height());
    update_region(r);
}
