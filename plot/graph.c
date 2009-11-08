

struct MultigrapherStruct
{
  /* multigrapher parameters (not updated over a multigrapher's lifetime) */
  plPlotter *plotter;		/* GNU libplot Plotter handle */
  const char *output_format;	/* type of libplot device driver [unused] */
  const char *bg_color;		/* color of background, if non-NULL */
  bool save_screen;		/* erase display when opening plotter? */
  /* graph parameters (constant over any single graph) */
  Transform x_trans, y_trans;   /* user->device coor transformations */
  Axis x_axis, y_axis;		/* information on each axis */
  grid_type grid_spec;		/* frame specification */
  double blankout_fraction;	/* 1.0 means blank whole box before drawing */
  bool no_rotate_y_label;	/* useful for pre-X11R6 X servers */
  double tick_size;		/* fractional tick size */
  double subtick_size;		/* fractional subtick size (for linear axes) */
  double frame_line_width;	/* fractional width of lines in the frame */
  double half_line_width;	/* approx. half of this, in libplot coors */
  const char *frame_color;	/* color for frame (and graph, if no -C) */
  const char *title;		/* graph title */
  const char *title_font_name;	/* font for graph title */
  double title_font_size;	/* fractional height of graph title */
  int clip_mode;		/* 0, 1, or 2 (cf. clipping in gnuplot) */
  /* following elements are updated during plotting of points; they're the
     chief repository for internal state */
  bool first_point_of_polyline;	/* true only at beginning of each polyline */
  double oldpoint_x, oldpoint_y; /* last-plotted point */
  int symbol;			/* symbol being plotted at each point */
  int linemode;			/* linemode used for polyline */
};

typedef struct MultigrapherStruct Multigrapher;

int
graph_show (lua_State *L)
{
  /* command-line parameters (constant over multigrapher operation) */
  const char *output_format = "meta";/* libplot output format */
  const char *bg_color = NULL;	/* color of background, if non-NULL */
  const char *bitmap_size = NULL;
  const char *emulate_color = NULL;
  const char *max_line_length = NULL;
  const char *meta_portable = NULL;
  const char *page_size = NULL;
  const char *rotation_angle = NULL;
  bool save_screen = false;	/* save screen, i.e. no erase before plot? */

  int log_axis = 0;
  int round_to_next_tick = 0;
  double min_x = 0.0, max_x = 0.0, spacing_x = 0.0;
  double min_y = 0.0, max_y = 0.0, spacing_y = 0.0;
  bool spec_min_x = false, spec_min_y = false;
  bool spec_max_x = false, spec_max_y = false;
  bool spec_spacing_x = false, spec_spacing_y = false;

  grid_type grid_spec = AXES_AND_BOX; /* frame type for current graph */
  bool no_rotate_y_label = false; /* used for pre-X11R6 servers */
  const char *frame_color = "black"; /* color of frame (and graph, if no -C)*/
  int clip_mode = 1;		/* clipping mode (cf. gnuplot) */
  /* following variables are portmanteau: x and y are included as bitfields*/
  int log_axis = 0;		/* log axes or linear axes? */
  int round_to_next_tick = 0;	/* round axis limits to nearest tick? */
  int switch_axis_end = 0;	/* axis at top/right instead of bottom/left? */
  int omit_ticks = 0;		/* omit ticks and tick labels from an axis? */

  /* graph dimensions, expressed as fractions of the width of the libplot
     graphics display [by convention square]; <0.0 means use libplot default */
  double frame_line_width = -0.001; /* width of lines in the graph frame */

  /* dimensions of graphing area, expressed as fractions of the width of
     the libplot graphics display [by convention square] */
  double margin_below = .2;	/* margin below the plot */
  double margin_left = .2;	/* margin left of the plot */
  double plot_height = .6;	/* height of the plot */
  double plot_width = .6;	/* width of the plot */

  /* dimensions, expressed as fractions of the size of the plotting area */
  double tick_size = .02;	/* size of tick marks (< 0.0 allowed) */
  double font_size = 0.0525;	/* fontsize */
  double title_font_size = 0.07; /* title fontsize */
  double blankout_fraction = 1.3; /* this fraction of size of plotting box
				   is erased before the plot is drawn */

  /* text-related */
  const char *font_name = NULL;	/* font name, NULL -> device default */
  const char *title_font_name = NULL; /* title font name, NULL -> default */
  const char *symbol_font_name = "ZapfDingbats"; /* symbol font name, NULL -> default */
  const char *x_label = NULL;	/* label for the x axis, NULL -> no label */
  const char *y_label = NULL;	/* label for the y axis, NULL -> no label */
  const char *top_label = NULL;	/* title above the plot, NULL -> no title */

  bool transpose_axes = false;

  MultiGrapher *multigrapher;

  double scale = 1.0, trans_x = 0.0, trans_y = 0.0;

  struct plot * p = plot_check (L, 1);

  multigrapher = new_multigrapher (output_format, bg_color, bitmap_size,
				   emulate_color, max_line_length,
				   meta_portable, page_size, rotation_angle,
				   save_screen);

  array_bounds (p->points, p->length,
		transpose_axes, clip_mode,
		&min_x, &min_y,	&max_x, &max_y,
		spec_min_x, spec_min_y, spec_max_x, spec_max_y);

  begin_graph (multigrapher, scale, trans_x, trans_y);

  set_graph_parameters (multigrapher,
			frame_line_width,
			frame_color,
			top_label,
			title_font_name, title_font_size, /*for title*/
			tick_size, grid_spec,
			min_x, max_x, spacing_x,
			min_y, max_y, spacing_y,
			spec_spacing_x, spec_spacing_y,
			plot_width, plot_height, 
			margin_below, margin_left,
			font_name, font_size, /* for abscissa label */
			x_label, 
			font_name, font_size, /* for ordinate label */
			y_label,
			no_rotate_y_label,
			/* these args are portmanteaux */
			log_axis, round_to_next_tick,
			switch_axis_end, omit_ticks, 
			/* more args */
			clip_mode,
			blankout_fraction,
			transpose_axes);

  /* draw the graph frame (grid, ticks, etc.); draw a `canvas' (a
     background opaque white rectangle) only if this isn't the
     first graph */
  draw_frame_of_graph (multigrapher, false);
	  
  /* plot the laboriously read-in array */
  plot_point_array (multigrapher, p->points, p->length);

  end_graph (multigrapher);
  
  /* finish up by deleting our multigrapher */
  delete_multigrapher (multigrapher);

  return 0;
}
