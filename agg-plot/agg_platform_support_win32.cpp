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
#include <pthread.h>
#include <new>
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

        bool load_pmap(const char* fn, unsigned idx, 
                       rendering_buffer* dst);

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

      pthread_mutex_t m_mutex[1];

      static void bitmap_info_resize (BITMAPINFO* bmp, unsigned w, unsigned h);
    };


    //------------------------------------------------------------------------
    platform_specific::platform_specific(pix_format_e format, bool flip_y) :
        m_format(format),
        m_sys_format(pix_format_undefined),
        m_flip_y(flip_y),
        m_bpp(0),
        m_sys_bpp(0),
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

      switch(m_format)
        {
        case pix_format_bw:
            m_sys_format = pix_format_bw;
            m_bpp = 1;
            m_sys_bpp = 1;
            break;

        case pix_format_gray8:
            m_sys_format = pix_format_gray8;
            m_bpp = 8;
            m_sys_bpp = 8;
            break;

        case pix_format_gray16:
            m_sys_format = pix_format_gray8;
            m_bpp = 16;
            m_sys_bpp = 8;
            break;

        case pix_format_rgb565:
        case pix_format_rgb555:
            m_sys_format = pix_format_rgb555;
            m_bpp = 16;
            m_sys_bpp = 16;
            break;

        case pix_format_rgbAAA:
        case pix_format_bgrAAA:
        case pix_format_rgbBBA:
        case pix_format_bgrABB:
            m_sys_format = pix_format_bgr24;
            m_bpp = 32;
            m_sys_bpp = 24;
            break;

        case pix_format_rgb24:
        case pix_format_bgr24:
            m_sys_format = pix_format_bgr24;
            m_bpp = 24;
            m_sys_bpp = 24;
            break;

        case pix_format_rgb48:
        case pix_format_bgr48:
            m_sys_format = pix_format_bgr24;
            m_bpp = 48;
            m_sys_bpp = 24;
            break;

        case pix_format_bgra32:
        case pix_format_abgr32:
        case pix_format_argb32:
        case pix_format_rgba32:
            m_sys_format = pix_format_bgra32;
            m_bpp = 32;
            m_sys_bpp = 32;
            break;

        case pix_format_bgra64:
        case pix_format_abgr64:
        case pix_format_argb64:
        case pix_format_rgba64:
            m_sys_format = pix_format_bgra32;
            m_bpp = 64;
            m_sys_bpp = 32;
            break;
        }
        ::QueryPerformanceFrequency(&m_sw_freq);
        ::QueryPerformanceCounter(&m_sw_start);

	pthread_mutex_init (m_mutex, NULL);
    }

  //------------------------------------------------------------------------
  platform_specific::~platform_specific()
  {
    pthread_mutex_destroy (m_mutex);

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
  template <class RenBufDst, class RenBufSrc>
  static void convert_pmap(RenBufDst* dst, const RenBufSrc* src, 
			   pix_format_e format, bool copy_req)
  {
    switch(format)
      {
      case pix_format_gray8:
      case pix_format_bgr24:
	if (copy_req)
	  dst->copy_from(*src);
	break;

      case pix_format_gray16:
	my_color_conv(dst, src, color_conv_gray16_to_gray8());
	break;

      case pix_format_rgb565:
	my_color_conv(dst, src, color_conv_rgb565_to_rgb555());
	break;

      case pix_format_rgbAAA:
	my_color_conv(dst, src, color_conv_rgbAAA_to_bgr24());
	break;

      case pix_format_bgrAAA:
	my_color_conv(dst, src, color_conv_bgrAAA_to_bgr24());
	break;

      case pix_format_rgbBBA:
	my_color_conv(dst, src, color_conv_rgbBBA_to_bgr24());
	break;

      case pix_format_bgrABB:
	my_color_conv(dst, src, color_conv_bgrABB_to_bgr24());
	break;

      case pix_format_rgb24:
	my_color_conv(dst, src, color_conv_rgb24_to_bgr24());
	break;

      case pix_format_rgb48:
	my_color_conv(dst, src, color_conv_rgb48_to_bgr24());
	break;

      case pix_format_bgr48:
	my_color_conv(dst, src, color_conv_bgr48_to_bgr24());
	break;

      case pix_format_abgr32:
	my_color_conv(dst, src, color_conv_abgr32_to_bgra32());
	break;

      case pix_format_argb32:
	my_color_conv(dst, src, color_conv_argb32_to_bgra32());
	break;

      case pix_format_rgba32:
	my_color_conv(dst, src, color_conv_rgba32_to_bgra32());
	break;

      case pix_format_bgra64:
	my_color_conv(dst, src, color_conv_bgra64_to_bgra32());
	break;

      case pix_format_abgr64:
	my_color_conv(dst, src, color_conv_abgr64_to_bgra32());
	break;

      case pix_format_argb64:
	my_color_conv(dst, src, color_conv_argb64_to_bgra32());
	break;

      case pix_format_rgba64:
	my_color_conv(dst, src, color_conv_rgba64_to_bgra32());
	break;
      }
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

	pixel_map pmap_tmp;
	pmap_tmp.attach_to_bmp(m_bmp_draw);

	rendering_buffer rbuf_tmp;
	int bstride = pmap_tmp.stride();
	rbuf_tmp.attach(pmap_tmp.buf(), w, h, m_flip_y ? bstride : -bstride);

	rendering_buffer_ro src_view;
	rendering_buffer_get_const_view(src_view, *src, r, m_bpp / 8, m_flip_y);

	convert_pmap(&rbuf_tmp, &src_view, m_format, true);

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

	pmap_tmp.draw(dc, &wrect, &brect);
      }
  }



    //------------------------------------------------------------------------
    bool platform_specific::save_pmap(const char* fn, unsigned idx, 
                                      const rendering_buffer* src)
    {
	pixel_map& img = m_pmap_img[idx];

        if(m_sys_format == m_format)
        {
	  return img.save_as_bmp(fn);
        }

        pixel_map pmap;
        pmap.create(img.width(), img.height(), org_e(m_sys_bpp));

        rendering_buffer rbuf_tmp;
	pixel_map_attach (pmap, &rbuf_tmp, m_flip_y);

        convert_pmap(&rbuf_tmp, src, m_format, false);
        return pmap.save_as_bmp(fn);
    }



    //------------------------------------------------------------------------
    bool platform_specific::load_pmap(const char* fn, unsigned idx, 
                                      rendering_buffer* dst)
    {
	pixel_map& img = m_pmap_img[idx];

        pixel_map pmap;
        if(!pmap.load_from_bmp(fn)) return false;

        rendering_buffer rbuf_tmp;
	pixel_map_attach (pmap, &rbuf_tmp, m_flip_y);

        img.create(pmap.width(), pmap.height(), org_e(m_bpp), 0);
	pixel_map_attach (img, dst, m_flip_y);

        switch(m_format)
        {
        case pix_format_gray8:
            switch(pmap.bpp())
            {
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_gray8()); break;
            }
            break;

        case pix_format_gray16:
            switch(pmap.bpp())
            {
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_gray16()); break;
            }
            break;

        case pix_format_rgb555:
            switch(pmap.bpp())
            {
            case 16: color_conv(dst, &rbuf_tmp, color_conv_rgb555_to_rgb555()); break;
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_rgb555()); break;
            case 32: color_conv(dst, &rbuf_tmp, color_conv_bgra32_to_rgb555()); break;
            }
            break;

        case pix_format_rgb565:
            switch(pmap.bpp())
            {
            case 16: color_conv(dst, &rbuf_tmp, color_conv_rgb555_to_rgb565()); break;
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_rgb565()); break;
            case 32: color_conv(dst, &rbuf_tmp, color_conv_bgra32_to_rgb565()); break;
            }
            break;

        case pix_format_rgb24:
            switch(pmap.bpp())
            {
            case 16: color_conv(dst, &rbuf_tmp, color_conv_rgb555_to_rgb24()); break;
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_rgb24()); break;
            case 32: color_conv(dst, &rbuf_tmp, color_conv_bgra32_to_rgb24()); break;
            }
            break;

        case pix_format_bgr24:
            switch(pmap.bpp())
            {
            case 16: color_conv(dst, &rbuf_tmp, color_conv_rgb555_to_bgr24()); break;
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_bgr24()); break;
            case 32: color_conv(dst, &rbuf_tmp, color_conv_bgra32_to_bgr24()); break;
            }
            break;

        case pix_format_rgb48:
            switch(pmap.bpp())
            {
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_rgb48()); break;
            }
            break;

        case pix_format_bgr48:
            switch(pmap.bpp())
            {
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_bgr48()); break;
            }
            break;

        case pix_format_abgr32:
            switch(pmap.bpp())
            {
            case 16: color_conv(dst, &rbuf_tmp, color_conv_rgb555_to_abgr32()); break;
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_abgr32()); break;
            case 32: color_conv(dst, &rbuf_tmp, color_conv_bgra32_to_abgr32()); break;
            }
            break;

        case pix_format_argb32:
            switch(pmap.bpp())
            {
            case 16: color_conv(dst, &rbuf_tmp, color_conv_rgb555_to_argb32()); break;
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_argb32()); break;
            case 32: color_conv(dst, &rbuf_tmp, color_conv_bgra32_to_argb32()); break;
            }
            break;

        case pix_format_bgra32:
            switch(pmap.bpp())
            {
            case 16: color_conv(dst, &rbuf_tmp, color_conv_rgb555_to_bgra32()); break;
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_bgra32()); break;
            case 32: color_conv(dst, &rbuf_tmp, color_conv_bgra32_to_bgra32()); break;
            }
            break;

        case pix_format_rgba32:
            switch(pmap.bpp())
            {
            case 16: color_conv(dst, &rbuf_tmp, color_conv_rgb555_to_rgba32()); break;
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_rgba32()); break;
            case 32: color_conv(dst, &rbuf_tmp, color_conv_bgra32_to_rgba32()); break;
            }
            break;

        case pix_format_abgr64:
            switch(pmap.bpp())
            {
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_abgr64()); break;
            }
            break;

        case pix_format_argb64:
            switch(pmap.bpp())
            {
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_argb64()); break;
            }
            break;

        case pix_format_bgra64:
            switch(pmap.bpp())
            {
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_bgra64()); break;
            }
            break;

        case pix_format_rgba64:
            switch(pmap.bpp())
            {
            case 24: color_conv(dst, &rbuf_tmp, color_conv_bgr24_to_rgba64()); break;
            }
            break;

        }

        return true;
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
	  try
	    {
	      app->m_specific->create_pmap(LOWORD(lParam), HIWORD(lParam),
					   &app->rbuf_window());

	      app->trans_affine_resizing(LOWORD(lParam), HIWORD(lParam));
	      app->on_resize(LOWORD(lParam), HIWORD(lParam));
	      app->force_redraw();
	    }
	  catch (std::bad_alloc&)
	    {
	      ::PostQuitMessage(1);
	    }

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


	try
	  {
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
	  }
	catch (std::bad_alloc&)
	  {
	    return false;
	  }
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
		  pthread_mutex_unlock (m_specific->m_mutex);
		  status = ::GetMessage(&msg, 0, 0, 0);
		  pthread_mutex_lock (m_specific->m_mutex);
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
    const char* platform_support::img_ext() const { return ".bmp"; }


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
            char fn[1024];
            strcpy(fn, file);
            int len = strlen(fn);
            if(len < 4 || stricmp(fn + len - 4, ".BMP") != 0)
            {
                strcat(fn, ".bmp");
            }
            return m_specific->load_pmap(fn, idx, &m_rbuf_img[idx]);
        }
        return true;
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
  pthread_mutex_lock (m_specific->m_mutex); 
}

void
platform_support_ext::unlock()
{ 
  pthread_mutex_unlock (m_specific->m_mutex); 
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
