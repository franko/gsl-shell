aggplot_defines = []

platform_window_deps = []

if host_machine.system() == 'windows'
    platform_sources = ['support_win32.cpp', 'agg_platform_support_win32.cpp', 'agg_win32_bmp.cpp']
else
    if host_machine.system() == 'darwin'
        aggplot_defines += '-DDARWIN_MACOSX'
    endif
    platform_window_deps += dependency('x11')
    platform_sources = ['support_x11.cpp', 'agg_platform_support_x11.cpp']
endif

aggplot_sources = ['printf_check.cpp', 'fonts.cpp', 'gamma.cpp', 'agg_font_freetype.cpp', 'plot.cpp', 'plot-auto.cpp', 'utils.cpp', 'units.cpp', 'colors.cpp', 'markers.cpp', 'draw_svg.cpp', 'canvas_svg.cpp', 'lua-draw.cpp', 'lua-text.cpp', 'text.cpp', 'agg-parse-trans.cpp', 'window_registry.cpp', 'window.cpp', 'lua-plot.cpp', 'canvas-window.cpp', 'bitmap-plot.cpp', 'lua-graph.cpp']

agg_dep = dependency('libagg')
luajit_dep = dependency('luajit')
freetype_dep = dependency('freetype2')

libaggplot = static_library('aggplot',
    aggplot_sources + platform_sources,
    dependencies: [agg_dep, luajit_dep, freetype_dep, threads_dep] + platform_window_deps,
    cpp_args : aggplot_defines,
    include_directories: [cpp_utils_include, lua_gsl_include, gsl_shell_include]
)
