if host_machine.system() == 'windows'
  platform_sources = ['support_win32.cpp', 'agg_platform_support_win32.cpp', 'agg_win32_bmp.cpp']
else
  platform_sources = ['support_x11.cpp', 'agg_platform_support_x11.cpp']
endif

aggplot_sources = platform_sources + [
    'printf_check.cpp',
    'fonts.cpp',
    'gamma.cpp',
    'agg_font_freetype.cpp',
    'plot.cpp',
    'plot-auto.cpp',
    'utils.cpp',
    'units.cpp',
    'colors.cpp',
    'markers.cpp',
    'draw_svg.cpp',
    'canvas_svg.cpp',
    'lua-draw.cpp',
    'lua-text.cpp',
    'text.cpp',
    'agg-parse-trans.cpp',
    'window_registry.cpp',
    'window.cpp',
    'lua-plot.cpp',
    'canvas-window.cpp',
    'bitmap-plot.cpp',
    'lua-graph.cpp',
]

libaggplot = static_library('aggplot',
    aggplot_sources,
    dependencies: [libagg_dep, threads_dep, freetype_dep, luajit_dep],
    include_directories: [gsl_shell_include, cpp_utils_include],
    cpp_args: gsl_shell_defines,
)

