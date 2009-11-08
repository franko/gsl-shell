/* This file is part of the GNU plotutils package.  Copyright (C) 1989,
   1990, 1991, 1995, 1996, 1997, 1998, 1999, 2000, 2005, 2008, Free
   Software Foundation, Inc.

   The GNU plotutils package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU plotutils package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

/* This file contains the point plotter half of GNU graph.  The point
   plotter could easily be linked with other software.  It translates a
   sequence of points, regarded as defining a polyline or a sequence of
   polylines, to a sequence of libplot calls.  There is support for
   multigraphing, i.e. producing a plot consisting of more than a single
   graph.  Each graph may be drawn from more than one file, i.e., input
   stream, and each input stream may provide more than a single polyline.
   
   A `point' is a structure.  Each point structure contains the following
   fields:

      x and y coordinates of the point
      a `have_x_errorbar' flag (true or false)
      a `have_y_errorbar' flag (true or false)
      xmin and xmax (meaningful only if have_x_errorbar is set)
      ymin and ymax (meaningful only if have_y_errorbar is set)
      a `pendown' flag

      a symbol type (a small integer, interpreted as a marker type)
      a symbol size (a fraction of the size of the plotting box)
      a symbol font name (relevant only for symbol types >= 32)
      a linemode (a small integer)
      a linewidth (a fraction of the size of the display device)
      a polyline fill-fraction (in the interval [0,1], <0 means no fill)
      a use_color flag (true or false)

   The point plotter constructs a polyline from each successive run of
   points that have the pendown flag set.  It assumes that the final seven
   fields are assumed to be the same for each point in such a run, i.e., it
   takes their values from the first point of the run.  At the location of
   each point on the polyline, the appropriate marker symbol, if any, will
   be plotted.  Symbol types greater than or equal to 32 are interpreted as
   single characters to be plotted, rather than symbols.

   Points without the pendown flag set cause the polyline to be broken, and
   a new one to begin, before the symbol (if any) is plotted.  

   The plotter supports five basic linemodes: 1 through 5.  The
   interpretation of `linemode' depends on the polyline's use_color flag.

	   linemode		If monochrome 		If color

	       1		solid			red
	       2		dotted			green
	       3		dotdashed		blue
	       4		shortdashed		magenta
	       5		longdashed		cyan

   In the monochrome case, the pattern simply repeats: 6,7,8,9,10 are
   equivalent to 1,2,3,4,5, etc.  In the colored case, the sequence of
   colors also repeats.  But linemodes 1,2,3,4,5 are drawn solid, while
   6,7,8,9,10 are drawn dotted, 11,12,13,14,15 are drawn dotdashed, etc.
   So there are 25 distinct colored linemodes, and 5 distinct monochrome
   (black) ones.

   The color of a symbol will be the same as the color of the polyline on
   which it is plotted.

   linemodes -1, -2, etc. have a special interpretation.  They are
   `disconnected' linemodes: no polyline will appear, but if color is
   being used, the color of the plotted symbols (if any) will be
   linemode-dependent.  -1,-2,-3,-4,5 signify red,green,blue,magenta,cyan
   (the same sequence as for 1,2,3,4,5); thereafter the sequence repeats.

   linemode 0 is special (for backward compatibility).  No line is drawn;
   symbol #1 (a point) will be used.  So using linemode 0 is the same as
   using linemode -1, symbol 1.

   The point plotter is invoked by calling the following, in order.

   new_multigrapher() creates a new point plotter.
   begin_graph()
   set_graph_parameters() initializes global structures used by 
   	the draw_frame_of_graph() and plot_point() routines.  These include
	the structures that specify the linear transformation from user
	coordinates to the coordinates used by libplot, and structures
	that specify the style of the plot frame.  
   draw_frame_of_graph() plots the graph frame.  [Optional.]
   plot_point() uses libplot routines to plot a single point, together 
   	with (possibly)	a line extending to it from the last point, and 
	a symbol. [Alternatively, plot_point_array() can be used, to plot 
	an array of points.]
   end_graph()
   ..
   [The begin_graph()..end_graph() block can be repeated indefinitely
    if desired, to create a multigraph.  set_graph_parameters() allows
    for repositioning of later graphs.]
   ..
   delete_multigrapher() deletes the point plotter.

   There is also a function end_polyline_and_flush(), which is useful for
   real-time display. */

#include "sys-defines.h"
#include "libcommon.h"
#include "plot.h"
#include "extern.h"

/* we use floating point libplot coordinates in the range [0,PLOT_SIZE] */
#define PLOT_SIZE 4096.0

#define FUZZ 0.000001		/* bd. on floating pt. roundoff error */

#define NEAR_EQUALITY(a, b, scale) (fabs((a) - (b)) < (FUZZ * fabs(scale)))

typedef unsigned int outcode;	/* for Cohen-Sutherland clipper */
enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

#define TRIAL_NUMBER_OF_TICK_INTERVALS 5
#define MAX_NUM_SUBTICKS 29	/* max num. of linearly spaced subticks */
#define RELATIVE_SUBTICK_SIZE 0.4 /* subtick_size / tick_size */
/* if a log axis spans >5.0 orders of magnitude, don't plot log subsubticks */
#define MAX_DECADES_WITH_LOG_SUBSUBTICKS 5.0 

/* inter-tick spacing types, returned by scale1() and spacing_type() */
#define S_ONE 0
#define S_TWO 1
#define S_FIVE 2
#define S_TWO_FIVE 3	/* we don't use this one, but user may request it */
#define S_UNKNOWN -2

/* valid graph axis layout types; A_LOG2, anyone? */
#define A_LINEAR 0
#define A_LOG10 1

/* The x_trans and y_trans elements of a Multigrapher specify the current
   linear transformation from user coordinates to device coordinates.  They
   are used both in the plotting of a graph frame, and in the plotting of
   data points within a graph. */

typedef struct
{
  /* Input (user) coordinates, all floating point.  These are the
     coordinates used in the original data points (or their base-10 logs,
     for an axis of log type).  We'll map them to the unit interval
     [0.0,1.0]. */
  double input_min, input_max;	/* min, max */
  double input_range;		/* max - min, precomputed for speed */
  /* If we're reversing axes, we'll then map [0.0,1.0] to [1.0,0.0] */
  bool reverse;
  /* We'll map [0.0,1.0] to another (smaller) interval, linearly */
  double squeezed_min, squeezed_max; /* min, max */
  double squeezed_range;	/* max - min */
  /* Output [i.e., libplot] coordinates.  The interval [0.0,1.0] will be
     mapped to this range, and the squeezed interval to a sub-range.  This
     is so that the box within which points are plotted will be smaller
     than the full area of the graphics display. */
  double output_min, output_max; /* min */
  double output_range;		/* max - min */
} Transform;

/* Affine transformation macros */

/* X Scale: convert from user x value to normalized x coordinate (floating
   point, 0.0 to 1.0). */
#define XS(x) (((x) - multigrapher->x_trans.input_min)/multigrapher->x_trans.input_range)
/* X Reflect: map [0,1] to [1,0], if that's called for */
#define XR(x) (multigrapher->x_trans.reverse ? 1.0 - (x) : (x))
/* X Squeeze: map [0,1] range for normalized x coordinate into a smaller
   interval, the x range for the plotting area within the graphics display */
#define XSQ(x) (multigrapher->x_trans.squeezed_min + (x) * multigrapher->x_trans.squeezed_range)
/* X Plot: convert from normalized x coordinate to floating point libplot
   coordinate. */
#define XP(x) (multigrapher->x_trans.output_min + (x) * multigrapher->x_trans.output_range)
/* X Value: convert from user x value (floating point) to floating point
   libplot coordinate. */
#define XV(x) XP(XSQ(XR(XS(x))))
/* X Normalize: the same, but do not perform reflection if any.  (We use
   this for plotting of axes and their labels.) */
#define XN(y) XP(XSQ(XS(y)))

/* Y Scale: convert from user y value to normalized y coordinate (floating
   point, 0.0 to 1.0). */
#define YS(y) (((y) - multigrapher->y_trans.input_min)/multigrapher->y_trans.input_range)
/* Y Reflect: map [0,1] to [1,0], if that's called for */
#define YR(y) (multigrapher->y_trans.reverse ? 1.0 - (y) : (y))
/* Y Squeeze: map [0,1] range for normalized y coordinate into a smaller
   interval, the y range for the plotting area within the graphics display */
#define YSQ(y) (multigrapher->y_trans.squeezed_min + (y) * multigrapher->y_trans.squeezed_range)
/* Y Plot: convert from normalized y coordinate to floating point libplot
   coordinate. */
#define YP(y) (multigrapher->y_trans.output_min + (y) * multigrapher->y_trans.output_range)
/* Y Value: convert from user y value (floating point) to floating point
   libplot coordinate.  (We use this for plotting of points.) */
#define YV(y) YP(YSQ(YR(YS(y))))
/* Y Normalize: the same, but do not perform reflection if any.  (We use
   this for plotting of axes and their labels.) */
#define YN(y) YP(YSQ(YS(y)))

/* Size Scale: convert distances, or sizes, from normalized coors to
   libplot coordinates.  (Used for tick, symbol, and font sizes.)  The min
   should really be precomputed. */
#define SS(x) \
(DMIN(multigrapher->x_trans.output_range * multigrapher->x_trans.squeezed_range, \
multigrapher->y_trans.output_range * multigrapher->y_trans.squeezed_range) * (x))

/* The `x_axis' and `y_axis' elements of a Multigrapher, which are of type
   `Axis', specify the layout of the two axes of a graph.  They are used in
   the drawing of a graph frame.  All elements that are doubles are
   expressed in user coordinates (unless the axis is logarithmic, in which
   case logs are taken before this structure is filled in). */

/* The `x_axis' and `y_axis' elements are filled in by calls to
   prepare_axis() when set_graph_parameters() is called.  The only
   exceptions to this are the elements `max_width' and `non_user_ticks',
   which are filled in by draw_frame_of_graph(), as the frame for a graph
   is being drawn. */

typedef struct 
{
  const char *font_name;	/* fontname for axis label and tick labels */
  double font_size;		/* font size for axis label and tick labels */
  const char *label;		/* axis label (a string) */
  int type;			/* axis layout type (A_LINEAR or A_LOG10) */
  double tick_spacing;		/* distance between ticks */
  int min_tick_count, max_tick_count; /* tick location = count * spacing */
  bool have_lin_subticks;	/* does axis have linearly spaced subticks? */
  double lin_subtick_spacing;	/* distance between linearly spaced subticks */
  int min_lin_subtick_count, max_lin_subtick_count; 
  bool have_normal_subsubticks; /* does axis have logarithmic subsubticks?*/
  bool user_specified_subsubticks; /* axis has user-spec'd subsubticks? */
  double subsubtick_spacing;	/* spacing for user-specified ones */
  double other_axis_loc;	/* location of intersection w/ other axis */
  double alt_other_axis_loc;	/* alternative loc. (e.g. right end vs. left)*/
  bool switch_axis_end;		/* other axis at right/top, not left/bottom? */
  bool omit_ticks;		/* just plain omit them (and their labels) ? */
  double max_label_width;	/* max width of labels placed on axis, in
				   libplot coors (we update this during
				   plotting, for y axis only) */
  int labelled_ticks;		/* number of labelled ticks, subticks, and
				   subsubticks drawn on the axis
				   (we update this during plotting, so we
				   can advise the user to specify a tick
				   spacing by hand if labelled_ticks <= 2) */
} Axis;

   	
/* The Multigrapher structure.  A pointer to one of these is passed as the
   first argument to each Multigrapher method (e.g., plot_point()). */

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
  
/* forward references */
static int clip_line (Multigrapher *multigrapher, double *x0_p, double *y0_p, double *x1_p, double *y1_p);
static int spacing_type (double spacing);
static outcode compute_outcode (Multigrapher *multigrapher, double x, double y, bool tolerant);
static void plot_abscissa_log_subsubtick (Multigrapher *multigrapher, double xval);
static void plot_errorbar (Multigrapher *multigrapher, const Point *p);
static void plot_ordinate_log_subsubtick (Multigrapher *multigrapher, double xval);
static void prepare_axis (Axis *axisp, Transform *trans, double min, double max, double spacing, const char *font_name, double font_size, const char *label, double subsubtick_spacing, bool user_specified_subsubticks, bool round_to_next_tick, bool log_axis, bool reverse_axis, bool switch_axis_end,  bool omit_ticks);
static void print_tick_label (char *labelbuf, const Axis *axis, const Transform *transform, double val);
static void scale1 (double min, double max, double *tick_spacing, int *tick_spacing_type);
static void set_line_style (Multigrapher *multigrapher, int style, bool use_color);
static void transpose_portmanteau (int *val);


/* print_tick_label() prints a label on an axis tick.  The format depends
 * on whether the axis is a log axis or a linear axis; also, the magnitude
 * of the axis labels.
 */

static void
print_tick_label (char *labelbuf, const Axis *axis, const Transform *transform, double val)
{
  int prec;
  char *eloc, *ptr;
  char labelbuf_tmp[64], incrbuf[64];
  double spacing;
  bool big_exponents;
  double min, max;

  /* two possibilities: large/small exponent magnitudes */

  min = (axis->type == A_LOG10 
	 ? pow (10.0, transform->input_min) : transform->input_min);
  max = (axis->type == A_LOG10 
	 ? pow (10.0, transform->input_max) : transform->input_max);
	  
  big_exponents = (((min != 0.0 && fabs (log10 (fabs (min))) >= 4.0)
		    || (max != 0.0 && fabs (log10 (fabs (max))) >= 4.0))
		   ? true : false);

  if (big_exponents)
    /* large exponents, rewrite as foo x 10^bar, using escape sequences */
    {
      char *src = labelbuf_tmp, *dst = labelbuf;
      int exponent;
      char floatbuf[64];
      char *fptr = floatbuf;
      double prefactor;
      
      sprintf (labelbuf_tmp, "%e", val);
      if ((eloc = strchr (labelbuf_tmp, (int)'e')) == NULL)
	return;

      if (axis->type == A_LOG10 && !axis->user_specified_subsubticks)
	/* a hack: this must be a power of 10, so just print "10^bar" */
	{
	  sscanf (++eloc, "%d", &exponent);	      
	  sprintf (dst, "10\\sp%d\\ep", exponent);
	  return;
	}

      /* special case: zero prints as `0', not 0.0x10^whatever */
      if (val == 0.0)
	{
	  *dst++ = '0';
	  *dst = '\0';
	  return;
	}

      while (src < eloc)
	*fptr++ = *src++;
      *fptr = '\0';
      sscanf (floatbuf, "%lf", &prefactor); /* get foo */
      sscanf (++src, "%d", &exponent); /* get bar */
      
      spacing = (axis->type == A_LINEAR
		 ? axis->tick_spacing
		 : axis->subsubtick_spacing); /* user-specified, for log axis*/
      sprintf (incrbuf, "%f", 
	       spacing / pow (10.0, (double)exponent));
      ptr = strchr (incrbuf, (int)'.');
      prec = 0;
      if (ptr != NULL)
	{
	  int count = 0;
	  
	  while (*(++ptr))
	    {
	      count++;
	      if (*ptr != '0')
		prec = count;
	    }
	}
      
      /* \sp ... \ep is start_superscript ... end_superscript, and \r6 is
	 right-shift by 1/6 em.  \mu is the `times' character. */
      sprintf (dst, "%.*f\\r6\\mu10\\sp%d\\ep", 
	       prec, prefactor, exponent);

      return;
    }

  else	/* small-size exponent magnitudes */
    {
      if (axis->type == A_LOG10 && !axis->user_specified_subsubticks)
	/* a hack: this must be a (small) power of 10, so we'll just use
	   %g format (same as %f, no trailing zeroes) */
	{
	  sprintf (labelbuf, "%.9g", val);
	  return;
	}
      
      /* always use no. of digits of precision present in increment */
      spacing = (axis->type == A_LINEAR
		 ? axis->tick_spacing
		 : axis->subsubtick_spacing); /* user-specified, for log axis*/
      sprintf (incrbuf, "%.9f", spacing);
      ptr = strchr (incrbuf, (int)'.');
      prec = 0;
      if (ptr != NULL)
	{
	  int count = 0;
	  
	  while (*(++ptr))
	    {
	      count++;
	      if (*ptr != '0')
		prec = count;
	    }
	}
      sprintf (labelbuf, "%.*f", prec, val);
      return;
    }
}

/* Algorithm SCALE1, for selecting an inter-tick spacing that will yield a
 * good-looking linear-format axis.  The spacing is always 1.0, 2.0, or 5.0
 * times a power of ten.
 *
 * Reference: Lewart, C. R., "Algorithms SCALE1, SCALE2, and
 *	SCALE3 for Determination of Scales on Computer Generated
 *	Plots", Communications of the ACM, 10 (1973), 639-640.  
 *      Also cited as ACM Algorithm 463.
 *
 * We call this routine even when the axis is logarithmic rather than
 * linear.  In that case the arguments are the logs of the actual
 * arguments, so that it computes an optimal inter-tick factor rather than
 * an optimal inter-tick spacing.
 */

/* ARGS: min,max = data min, max
   	 tick_spacing = inter-tick spacing
	 tick_spacing_type = 0,1,2 i.e. S_ONE,S_TWO,S_FIVE */
static void
scale1 (double min, double max, double *tick_spacing, int *tick_spacing_type)
{
  int k;
  double nal;
  double a, b;
  
  /* valid interval lengths */
  static const double vint[] =
    {
      1.0, 2.0, 5.0, 10.0
    };
  
  /* Corresponding breakpoints.  The published algorithm uses geometric
     means, i.e. sqrt(2), sqrt(10), sqrt(50), but using sqrt(10)=3.16...
     will (if nticks=5, as we choose it to be) cause intervals of length
     1.5 to yield an inter-tick distance of 0.2 rather than 0.5.  So we
     could reduce it to 2.95.  Similarly we could reduce sqrt(50) to 6.95
     so that intervals of length 3.5 will yield an inter-tick distance of
     1.0 rather than 0.5. */
  static const double sqr[] =
    {
      M_SQRT2, 3.16228, 7.07107
    };

  /* compute trial inter-tick interval length */
  a = (max - min) / TRIAL_NUMBER_OF_TICK_INTERVALS;
  a *= (max > min) ? 1.0 : -1.0; /* paranoia, max>min always */
  if (a <= 0.0)			
    {
      fprintf(stderr, "%s: error: the trial inter-tick spacing '%g' is bad\n",
	      progname, a);
      exit (EXIT_FAILURE);
    }
  nal = floor(log10(a));
  b = a * pow (10.0, -nal);	/* 1.0 <= b < 10.0 */

  /* round to closest permissible inter-tick interval length */
  k = 0;
  do
    {
      if (b < sqr[k])
	break;
      k++;
    }
  while (k < 3);

  *tick_spacing = (max > min ? 1.0 : -1.0) * vint[k] * pow (10.0, nal);
  /* for increment type, 0,1,2 means 1,2,5 times a power of 10 */
  *tick_spacing_type = (k == 3 ? 0 : k);
  return;
}

/* Determine whether an inter-tick spacing (in practice, one specified by
   the user) is 1.0, 2.0, or 5.0 times a power of 10. */
static int 
spacing_type (double incr)
{
  int i;
  int i_tenpower = (int)(floor(log10(incr)));
  double tenpower = 1.0;
  bool neg_power = false;

  if (i_tenpower < 0)
    {
      neg_power = true;
      i_tenpower = -i_tenpower;
    }

  for (i = 0; i < i_tenpower; i++)
    tenpower *= 10;
  if (neg_power)
    tenpower = 1.0 / tenpower;

  if (NEAR_EQUALITY(incr, tenpower, tenpower))
    return S_ONE;
  else if (NEAR_EQUALITY(incr, 2 * tenpower, tenpower))
    return S_TWO;
  else if (NEAR_EQUALITY(incr, 2.5 * tenpower, tenpower))
    return S_TWO_FIVE;
  else if (NEAR_EQUALITY(incr, 5 * tenpower, tenpower))
    return S_FIVE;
  else
    return S_UNKNOWN;
}


/* prepare_axis() fills in the Axis structure for an axis, and some of
 * the linear transformation variables in the Transform structure also.
 */

/* ARGS: user_specified_subticks = linear ticks on a log axis?
   	 round_to_next_tick = round limits to next tick mark?
	 log_axis = log axis?
	 reverse_axis = reverse min, max?
	 switch_axis_end = intersection w/ other axis in alt. pos.?
	 omit_ticks = suppress all ticks and tick labels? */
static void 
prepare_axis (Axis *axisp, Transform *trans, double min, double max, double spacing, const char *font_name, double font_size, const char *label, double subsubtick_spacing, bool user_specified_subsubticks, bool round_to_next_tick, bool log_axis, bool reverse_axis, bool switch_axis_end, bool omit_ticks)
{
  double range;
  int tick_spacing_type = 0;
  double tick_spacing, lin_subtick_spacing;
  int min_tick_count, max_tick_count;
  int min_lin_subtick_count, max_lin_subtick_count;
  bool have_lin_subticks;

  if (min > max)
    /* paranoia, max < min is swapped at top level */
    {
      fprintf(stderr, "%s: error: min > max for an axis, which is not allowed\n",
	      progname);
      exit (EXIT_FAILURE);
    }

  if (min == max)		/* expand in a clever way */
    {
      max = floor (max + 1.0);
      min = ceil (min - 1.0);
    }

  if (log_axis)		/* log axis, data are stored in logarithmic form */
    /* compute a tick spacing; user can't specify it */
    {
      scale1 (min, max, &tick_spacing, &tick_spacing_type);
      if (tick_spacing <= 1.0)
	{
	  tick_spacing = 1.0;
	  tick_spacing_type = S_ONE;
	}
    }
  else				/* linear axis */
    {
      if (spacing == 0.0)	/* i.e., not specified by user */
	scale1 (min, max, &tick_spacing, &tick_spacing_type);
      else			/* luser is boss, don't use SCALE1 */
	{
	  tick_spacing = spacing;
	  tick_spacing_type = spacing_type (spacing);
	}
    }

  range = max - min;		/* range is not negative */

  if (round_to_next_tick)	/* expand both limits to next tick */
    {
      if (user_specified_subsubticks)
	/* Special Case.  If user specified the `spacing' argument to -x or
	   -y on a logarithmic axis, our usual tick-generating and
	   tick-plotting algorithms are disabled.  So we don't bother with
	   min_tick_count or several other fields of the axis struct;
	   instead we just compute a new (rounded) max, min, and range.
	   Since most data are stored as logs, this is complicated. */
	{
	  double true_min = pow (10.0, min), true_max = pow (10.0, max);
	  double true_range = true_max - true_min;
	  int min_count, max_count;
	  
	  min_count = (int)(floor ((true_min + FUZZ * true_range)
				   / subsubtick_spacing));
	  max_count = (int)(ceil ((true_max - FUZZ * true_range) 
				  / subsubtick_spacing));
	  /* avoid core dump, do *not* reduce minimum to zero! */
	  if (min_count > 0)
	    min = log10 (min_count * subsubtick_spacing);
	  max = log10 (max_count * subsubtick_spacing);	  
	  range = max - min;
	  min_tick_count = max_tick_count = 0; /* keep gcc happy */
	}
      else	/* normal `expand limits to next tick' case */
	{
	  min_tick_count = (int)(floor((min + FUZZ * range)/ tick_spacing));
	  max_tick_count = (int)(ceil((max - FUZZ * range)/ tick_spacing));
	  /* max_tick_count > min_tick_count always */
	  /* tickval = tick_spacing * count, 
	     for all count in [min_count,max_count]; must have >=2 ticks */
	  min = tick_spacing * min_tick_count;
	  max = tick_spacing * max_tick_count;
	  range = max - min;
	}
    }
  else		/* don't expand limits to next tick */
    {
      min_tick_count = (int)(ceil((min - FUZZ * range)/ tick_spacing));
      max_tick_count = (int)(floor((max + FUZZ * range)/ tick_spacing)); 
      /* max_tick_count <= min_tick_count is possible */
      /* tickval = incr * count, 
	 for all count in [min_count,max_count]; can have 0,1,2,3... ticks */
    }
  
  /* Allow 5 subticks per tick if S_FIVE or S_TWO_FIVE, 2 if S_TWO.  Case
     S_ONE is special; we try 10, 5, and 2 in succession */
  switch (tick_spacing_type)
    {
    case S_FIVE:
    case S_TWO_FIVE:
      lin_subtick_spacing = tick_spacing / 5;      
      break;
    case S_TWO:
      lin_subtick_spacing = tick_spacing / 2;
      break;
    case S_ONE:
      lin_subtick_spacing = tick_spacing / 10;
      min_lin_subtick_count = (int)(ceil((min - FUZZ * range)/ lin_subtick_spacing));
      max_lin_subtick_count = (int)(floor((max + FUZZ * range)/ lin_subtick_spacing)); 
      if (max_lin_subtick_count - min_lin_subtick_count > MAX_NUM_SUBTICKS)
	{
	  lin_subtick_spacing = tick_spacing / 5;
	  min_lin_subtick_count = (int)(ceil((min - FUZZ * range)/ lin_subtick_spacing));
	  max_lin_subtick_count = (int)(floor((max + FUZZ * range)/ lin_subtick_spacing)); 
	  if (max_lin_subtick_count - min_lin_subtick_count > MAX_NUM_SUBTICKS)
	    lin_subtick_spacing = tick_spacing / 2;
	}
      break;
    default:
      /* in default case, i.e. S_UNKNOWN, we won't plot linear subticks */
      lin_subtick_spacing = tick_spacing; /* not actually needed, since not plotted */
      break;
    }

  /* smallest possible inter-subtick factor for a log axis is 10.0 */
  if (log_axis && lin_subtick_spacing <= 1.0)
    lin_subtick_spacing = 1.0;

  min_lin_subtick_count = (int)(ceil((min - FUZZ * range)/ lin_subtick_spacing));
  max_lin_subtick_count = (int)(floor((max + FUZZ * range)/ lin_subtick_spacing)); 
  have_lin_subticks 
    = ((tick_spacing_type != S_UNKNOWN /* S_UNKNOWN -> no subticks */
	&& (max_lin_subtick_count - min_lin_subtick_count) <= MAX_NUM_SUBTICKS)
       ? true : false);

  /* fill in parameters for axis-specific affine transformation */
  trans->input_min = min;
  trans->input_max = max;
  trans->input_range = range;	/* precomputed for speed */
  trans->reverse = reverse_axis;

  /* fill in axis-specific plot frame variables */
  axisp->switch_axis_end = switch_axis_end;
  axisp->omit_ticks = omit_ticks;
  axisp->label = label;
  axisp->font_name = font_name;
  axisp->font_size = font_size;
  axisp->max_label_width = 0.0;
  axisp->type = log_axis ? A_LOG10 : A_LINEAR;
  axisp->tick_spacing = tick_spacing;
  axisp->min_tick_count = min_tick_count;
  axisp->max_tick_count = max_tick_count;
  axisp->have_lin_subticks = have_lin_subticks;
  axisp->lin_subtick_spacing = lin_subtick_spacing;
  axisp->min_lin_subtick_count = min_lin_subtick_count;
  axisp->max_lin_subtick_count = max_lin_subtick_count;
  axisp->user_specified_subsubticks = user_specified_subsubticks;
  axisp->subsubtick_spacing = subsubtick_spacing;
  axisp->labelled_ticks = 0;	/* updated during drawing of frame */

  if (log_axis)		/* logarithmic axis */
    /* do we have special logarithmic subsubticks, and should we label them? */
    {
      if (max - min <= 
	  MAX_DECADES_WITH_LOG_SUBSUBTICKS + FUZZ) 
	/* not too many orders of magnitude, so plot normal log subsubticks */
	axisp->have_normal_subsubticks = true;
      else
	/* too many orders of magnitude, don't plot log subsubticks */
	axisp->have_normal_subsubticks = false;
    }
  else				/* linear axes don't have log subsubticks */
    axisp->have_normal_subsubticks = false;
}

/* The following routines [new_multigrapher(), begin_graph(),
 * set_graph_parameters(), draw_frame_of_graph(), plot_point(),
 * end_graph(), delete_multigrapher()] are the basic routines of the
 * point-plotter .  See descriptions at the head of this file.  */

/* Create a new Multigrapher.  The arguments, after the first, are the
   libplot Plotter parameters that the `graph' user can set on the command
   line. */

Multigrapher *
new_multigrapher (const char *output_format, const char *bg_color, const char *bitmap_size, const char *emulate_color, const char *max_line_length, const char *meta_portable, const char *page_size, const char *rotation_angle, bool save_screen)
{
  plPlotterParams *plotter_params;
  plPlotter *plotter;
  Multigrapher *multigrapher;
   	
  multigrapher = (Multigrapher *)xmalloc (sizeof (Multigrapher));

  /* set Plotter parameters */
  plotter_params = pl_newplparams ();
  pl_setplparam (plotter_params, "BG_COLOR", (void *)bg_color);
  pl_setplparam (plotter_params, "BITMAPSIZE", (void *)bitmap_size);
  pl_setplparam (plotter_params, "EMULATE_COLOR", (void *)emulate_color);
  pl_setplparam (plotter_params, "MAX_LINE_LENGTH", (void *)max_line_length);
  pl_setplparam (plotter_params, "META_PORTABLE", (void *)meta_portable);
  pl_setplparam (plotter_params, "PAGESIZE", (void *)page_size);
  pl_setplparam (plotter_params, "ROTATION", (void *)rotation_angle);

  /* create Plotter and open it */
  plotter = pl_newpl_r (output_format, NULL, stdout, stderr, plotter_params);
  if (plotter == (plPlotter *)NULL)
    return (Multigrapher *)NULL;
  pl_deleteplparams (plotter_params);
  multigrapher->plotter = plotter;
  if (pl_openpl_r (plotter) < 0)
    return (Multigrapher *)NULL;
  multigrapher->bg_color = bg_color;

  /* if called for, erase it; set up the user->device coor map */
  if (!save_screen || bg_color)
    pl_erase_r (plotter);
  pl_fspace_r (plotter, 0.0, 0.0, (double)PLOT_SIZE, (double)PLOT_SIZE);

  return multigrapher;
}

int
delete_multigrapher (Multigrapher *multigrapher)
{
  int retval;

  retval = pl_closepl_r (multigrapher->plotter);
  if (retval >= 0)
    retval = pl_deletepl_r (multigrapher->plotter);

  free (multigrapher);
  return retval;
}


void
begin_graph (Multigrapher *multigrapher, double scale, double trans_x, double trans_y)
{
  pl_savestate_r (multigrapher->plotter);
  pl_fconcat_r (multigrapher->plotter,
		scale, 0.0, 0.0, scale,
		trans_x * PLOT_SIZE, trans_y * PLOT_SIZE);
}

void
end_graph (Multigrapher *multigrapher)
{
  pl_restorestate_r (multigrapher->plotter);
}


/* ARGS:
     Multigrapher *multigrapher
     double frame_line_width 	= fractional width of lines in the frame
     const char *frame_color 	= color for frame (and plot if no -C option)
     const char *title 		= graph title
     const char *title_font_name = font for graph title (string)
     double title_font_size 	= font size for graph title
     double tick_size 		= fractional size of ticks
     grid_type grid_spec 	= gridstyle (and tickstyle) spec
     double x_min, x_max, x_spacing
     double y_min, y_max, y_spacing
     bool spec_x_spacing
     bool spec_y_spacing
     double width, height, up, right
     const char *x_font_name
     double x_font_size
     const char *x_label
     const char *y_font_name 
     double y_font_size
     const char *y_label
     bool no_rotate_y_label

     -- portmanteaux args --
     int log_axis		= whether axes should be logarithmic
     int round_to_next_tick	= round limits to the next tick mark
     int switch_axis_end	= put axes at right/top, not left/bottom?
     int omit_ticks		= omit all ticks, tick labels from an axis?
     -- other args --
     int clip_mode		= 0, 1, or 2
     double blankout_fraction	= 1.0 means blank out whole box before plot
     bool transpose_axes */
void 
set_graph_parameters (Multigrapher *multigrapher, double frame_line_width, const char *frame_color, const char *title, const char *title_font_name, double title_font_size, double tick_size, grid_type grid_spec, double x_min, double x_max, double x_spacing, double y_min, double y_max, double y_spacing, bool spec_x_spacing, bool spec_y_spacing, double width, double height, double up, double right, const char *x_font_name, double x_font_size, const char *x_label, const char *y_font_name, double y_font_size, const char *y_label, bool no_rotate_y_label, int log_axis, int round_to_next_tick, int switch_axis_end, int omit_ticks, int clip_mode, double blankout_fraction, bool transpose_axes)
{
  double x_subsubtick_spacing = 0.0, y_subsubtick_spacing = 0.0;
  /* local portmanteau variables */
  int reverse_axis = 0;		/* min > max on an axis? */
  int user_specified_subsubticks = 0; /* i.e. linear ticks on a log axis? */

  if (log_axis & X_AXIS)
    {
      if (spec_x_spacing)
	/* spacing is handled specially for log axes */
	{
	  spec_x_spacing = false;
	  user_specified_subsubticks |= X_AXIS;
	  x_subsubtick_spacing = x_spacing;
	}
    }
  
  if (log_axis & Y_AXIS)
    {
      if (spec_y_spacing)
	{
	  /* spacing is handled specially for log axes */
	  spec_y_spacing = false;
	  user_specified_subsubticks |= Y_AXIS;
	  y_subsubtick_spacing = y_spacing;
	}
    }
  
  /* check for reversed axes (min > max) */
  if (x_max < x_min)
    {
      reverse_axis |= X_AXIS;
      {
	double temp;
	
	temp = x_min;
	x_min = x_max;
	x_max = temp;
      }
    }
  if (x_max == x_min)
    {
      fprintf (stderr, 
	       "%s: identical upper and lower x limits are separated\n",
	       progname);
      /* separate them */
      x_max += 1.0;
      x_min -= 1.0;
    }
  /* check for reversed axes (min > max) */
  if (y_max < y_min)
    {
      reverse_axis |= Y_AXIS;
      {
	double temp;
	
	temp = y_min;
	y_min = y_max;
	y_max = temp;
      }
    }
  if (y_max == y_min)
    {
      fprintf (stderr, 
	       "%s: identical upper and lower y limits are separated\n",
	       progname);
      /* separate them */
      y_max += 1.0;
      y_min -= 1.0;
    }
  
  /* At this point, min < max for each axis, if the user specified the two
     limits on an axis; reverse_axis portmanteau variable keeps track of
     whether either axis was discovered to be reversed. */
  
  /* silently accept negative spacing as equivalent as positive */
  if (spec_x_spacing)
    {
      if (x_spacing == 0.0)
	{
	  fprintf (stderr, 
		   "%s: error: the spacing between ticks on an axis is zero\n",
		   progname);
	  exit (EXIT_FAILURE);
	}
      x_spacing = fabs (x_spacing);
    }
  if (spec_y_spacing)
    {
      if (y_spacing == 0.0)
	{
	  fprintf (stderr, 
		   "%s: error: the spacing between ticks on an axis is zero\n",
		   progname);
	  exit (EXIT_FAILURE);
	}
      y_spacing = fabs (y_spacing);
    }
	      
  /* now transpose the two axes (i.e. their portmanteau variables, labels,
     limits etc.) if transpose_axes was set */
  if (transpose_axes)
    {
      const char *temp_string;
      double temp_double;
      
      transpose_portmanteau (&log_axis);
      transpose_portmanteau (&round_to_next_tick);
      transpose_portmanteau (&switch_axis_end);
      transpose_portmanteau (&omit_ticks);

      transpose_portmanteau (&reverse_axis);
      transpose_portmanteau (&user_specified_subsubticks);
      
      temp_string = x_label;
      x_label = y_label;
      y_label = temp_string;
      
      temp_double = x_min;
      x_min = y_min;
      y_min = temp_double;
      
      temp_double = x_max;
      x_max = y_max;
      y_max = temp_double;
      
      temp_double = x_spacing;
      x_spacing = y_spacing;
      y_spacing = temp_double;
      
      temp_double = x_subsubtick_spacing;
      x_subsubtick_spacing = y_subsubtick_spacing;
      y_subsubtick_spacing = temp_double;
    }
	      
  /* fill in the Multigrapher struct */

  multigrapher->frame_line_width = frame_line_width;
  multigrapher->frame_color = frame_color;
  multigrapher->no_rotate_y_label = no_rotate_y_label;
  multigrapher->blankout_fraction = blankout_fraction;

  if (title != NULL)
    multigrapher->title = xstrdup (title);
  else
    multigrapher->title = NULL;
  if (title_font_name != NULL)
      multigrapher->title_font_name = xstrdup (title_font_name);
  else
    multigrapher->title_font_name = NULL;
  multigrapher->title_font_size = title_font_size;
  multigrapher->tick_size = tick_size;
  multigrapher->subtick_size = RELATIVE_SUBTICK_SIZE * tick_size;
  multigrapher->grid_spec = grid_spec;
  multigrapher->clip_mode = clip_mode;

  /* fill in the Transform and Axis elements for each coordinate */
  prepare_axis (&multigrapher->x_axis, &multigrapher->x_trans,
		x_min, x_max, x_spacing,
		x_font_name, x_font_size, x_label, 
		x_subsubtick_spacing,
		(bool)(user_specified_subsubticks & X_AXIS), 
		(bool)(round_to_next_tick & X_AXIS),
		(bool)(log_axis & X_AXIS), 
		(bool)(reverse_axis & X_AXIS),
		(bool)(switch_axis_end & X_AXIS),
		(bool)(omit_ticks & X_AXIS));
  prepare_axis (&multigrapher->y_axis, &multigrapher->y_trans,
		y_min, y_max, y_spacing,
		y_font_name, y_font_size, y_label, 
		y_subsubtick_spacing,
		(bool)(user_specified_subsubticks & Y_AXIS), 
		(bool)(round_to_next_tick & Y_AXIS),
		(bool)(log_axis & Y_AXIS), 
		(bool)(reverse_axis & Y_AXIS),
		(bool)(switch_axis_end & Y_AXIS),
		(bool)(omit_ticks & Y_AXIS));
  
  /* fill in additional parameters in the two Transform structures */
  multigrapher->x_trans.squeezed_min = right;
  multigrapher->x_trans.squeezed_max = right + width;
  multigrapher->x_trans.squeezed_range = width;  
  multigrapher->y_trans.squeezed_min = up;
  multigrapher->y_trans.squeezed_max = up + height;
  multigrapher->y_trans.squeezed_range = height;

  /* specify interval range for each coordinate, in libplot units */
  multigrapher->x_trans.output_min = 0.0;
  multigrapher->x_trans.output_max = (double)PLOT_SIZE;
  multigrapher->x_trans.output_range = multigrapher->x_trans.output_max - multigrapher->x_trans.output_min;
  multigrapher->x_trans.output_min = 0.0;
  multigrapher->y_trans.output_max = (double)PLOT_SIZE;
  multigrapher->y_trans.output_range = multigrapher->y_trans.output_max - multigrapher->y_trans.output_min;

  /* fill in fields in Axis structs dealing with location of other axis */
  if (multigrapher->grid_spec != AXES_AT_ORIGIN)
    /* Normal case */
    {
      /* axes are at left/bottom */
      multigrapher->x_axis.other_axis_loc = multigrapher->x_trans.input_min;
      multigrapher->y_axis.other_axis_loc = multigrapher->y_trans.input_min;
      /* secondary axes (used only if --switch-axis-end is specified) */
      multigrapher->x_axis.alt_other_axis_loc = multigrapher->x_trans.input_max;
      multigrapher->y_axis.alt_other_axis_loc = multigrapher->y_trans.input_max;
    }
  else
    /* Special case: grid type #4, AXES_AT_ORIGIN */
    {
      /* In this case (grid type #4), we don't allow the user to move the
         axis position by using the --switch-axis-end option.  Each axis is
         at the value 0 (the origin) if the value 0 is between the limits
         of the opposing axis.  Otherwise, the position is at the end
         closer to the value of 0. */
      multigrapher->x_axis.other_axis_loc 
	= (multigrapher->x_trans.input_min * multigrapher->x_trans.input_max <= 0.0) ? 0.0 : 
	  (multigrapher->x_trans.input_min > 0.0 ? multigrapher->x_trans.input_min : multigrapher->x_trans.input_max);
      multigrapher->y_axis.other_axis_loc 
	= (multigrapher->y_trans.input_min * multigrapher->y_trans.input_max <= 0.0) ? 0.0 : 
	  (multigrapher->y_trans.input_min > 0.0 ? multigrapher->y_trans.input_min : multigrapher->y_trans.input_max);
      /* secondary axes are the same */
      multigrapher->x_axis.alt_other_axis_loc = multigrapher->x_axis.other_axis_loc;
      multigrapher->y_axis.alt_other_axis_loc = multigrapher->y_axis.other_axis_loc;
      multigrapher->x_axis.switch_axis_end = (((multigrapher->x_trans.input_max - multigrapher->x_axis.other_axis_loc)
				 < (multigrapher->x_axis.other_axis_loc - multigrapher->x_trans.input_min))
				? true : false);
      multigrapher->y_axis.switch_axis_end = (((multigrapher->y_trans.input_max - multigrapher->y_axis.other_axis_loc)
				 < (multigrapher->y_axis.other_axis_loc - multigrapher->y_trans.input_min))
				? true : false);
    }

  /* The following is a version of (multigrapher->frame_line_width)/2
     (expressed in terms of libplot coordinates) which the plotter uses as
     an offset, to get highly accurate positioning of ticks and labels. */
  if (frame_line_width < 0.0 
      || pl_havecap_r (multigrapher->plotter, "WIDE_LINES") == 0)
    multigrapher->half_line_width = 0.0;/* N.B. <0.0 -> default width, pres. small */
  else
    multigrapher->half_line_width = 0.5 * frame_line_width * multigrapher->x_trans.output_range;

  /* initialize the plotter state variables */
  multigrapher->first_point_of_polyline = true;
  multigrapher->oldpoint_x = 0.0; 
  multigrapher->oldpoint_y = 0.0;
}

   	
/* draw_frame_of_graph() plots the graph frame (grid plus axis labels and
   title), using the multigrapher's variables (the multigrapher->x_axis,
   multigrapher->y_axis, multigrapher->x_trans, multigrapher->y_trans
   structures).  It comprises several almost-independent tasks:

   0. draw opaque white rectangle (``canvas''), if requested
   1. draw title, if any, above drawing box
   2. draw axes, and a full drawing box if requested
   3. draw ticks, grid lines, and tick labels on abscissa;
      also subticks, if abscissa is a linear axis
   4. draw ticks, grid lines, and tick labels on ordinate,
      also subticks, if ordinate is a linear axis
   5. draw sub-tick marks, sub-grid lines, and sub-labels,
      if abscissa is a logarithmic axis
   6. draw sub-tick marks, sub-grid lines, and sub-labels,
      if ordinate is a logarithmic axis
   7. draw axis label on abscissa
   8. draw axis label on ordinate (normally rotated 90 degrees)

   A savestate()--restorestate() pair is wrapped around these nine tasks.
   They are not quite independent because in (4) and (6) we keep track of
   the maximum width of the tick labels on the ordinate, to position the
   rotated ordinate label properly in (8).

   At the conclusion of the eight tasks, we warn the user if (i) he/she
   didn't specify a tick spacing for a logarithmic axis by hand, and (ii)
   our default algorithm for drawing ticks on a logarithmic axis has drawn
   too few ticks (i.e. <= 2 ticks) on the axis.

   Task #0 (drawing a white canvas) isn't always performed; only if the
   argument draw_canvas is set.  It isn't set when we call this function
   for the first time; see graph.c.  */

void
draw_frame_of_graph (Multigrapher *multigrapher, bool draw_canvas)
{
  static bool tick_warning_printed = false; /* when too few labelled ticks */

  /* wrap savestate()--restorestate() around all 9 tasks */
  pl_savestate_r (multigrapher->plotter); 

  /* set color for graph frame */
  if (multigrapher->frame_color)
    pl_pencolorname_r (multigrapher->plotter, multigrapher->frame_color);

  /* set line width as a fraction of size of display, <0.0 means default */
  pl_flinewidth_r (multigrapher->plotter, 
		   multigrapher->frame_line_width * (double)PLOT_SIZE);
  
  /* axes (or box) will be drawn in solid line style */
  pl_linemod_r (multigrapher->plotter, "solid");

  /* turn off filling */
  pl_filltype_r (multigrapher->plotter, 0);

  /* 0.  DRAW AN OPAQUE WHITE BOX */

  if (draw_canvas)
    {
      pl_savestate_r (multigrapher->plotter);
      /* use user-specified background color (if any) instead of white */
      if (pl_havecap_r (multigrapher->plotter, "SETTABLE_BACKGROUND") != 0 
	  && multigrapher->bg_color)
	pl_colorname_r (multigrapher->plotter, multigrapher->bg_color);
      else
	pl_colorname_r (multigrapher->plotter, "white");

      pl_filltype_r (multigrapher->plotter, 1);	/* turn on filling */
      pl_fbox_r (multigrapher->plotter, 
		 XP(XSQ(0.5 - 0.5 * multigrapher->blankout_fraction)), 
		 YP(YSQ(0.5 - 0.5 * multigrapher->blankout_fraction)),
		 XP(XSQ(0.5 + 0.5 * multigrapher->blankout_fraction)),
		 YP(YSQ(0.5 + 0.5 * multigrapher->blankout_fraction)));
      pl_restorestate_r (multigrapher->plotter);
    }

  /* 1.  DRAW THE TITLE, I.E. THE TOP LABEL */

  if (multigrapher->grid_spec != NO_AXES 
      && !multigrapher->y_axis.switch_axis_end /* no title if x axis is at top of plot */
      && multigrapher->title != NULL && *multigrapher->title != '\0')
    {
      double title_font_size;

      /* switch to our font for drawing title */
      pl_fontname_r (multigrapher->plotter, multigrapher->title_font_name);
      title_font_size = pl_ffontsize_r (multigrapher->plotter, 
					SS(multigrapher->title_font_size));

      pl_fmove_r (multigrapher->plotter, 
		  XP(XSQ(0.5)), 
		  YP(YSQ(1.0 
			 + (((multigrapher->grid_spec == AXES_AND_BOX 
			      || multigrapher->grid_spec == AXES)
			     && (multigrapher->tick_size <= 0.0) ? 1.0 : 0.5)
			    * fabs(multigrapher->tick_size))))
		  + 0.65 * title_font_size
		  + multigrapher->half_line_width);
      /* title centered, bottom spec'd */
      pl_alabel_r (multigrapher->plotter, 'c', 'b', multigrapher->title); 
    }

  /* 2.  DRAW AXES FOR THE PLOT */

  switch (multigrapher->grid_spec)
    {
    case AXES_AND_BOX_AND_GRID:
    case AXES_AND_BOX:
      /* draw a box, not just a pair of axes */
      pl_fbox_r (multigrapher->plotter, 
		 XP(XSQ(0.0)), YP(YSQ(0.0)), XP(XSQ(1.0)), YP(YSQ(1.0)));
      break;
    case AXES:
      {
	double xstart, ystart, xmid, ymid, xend, yend;
	
	xstart = (multigrapher->x_axis.switch_axis_end 
		  ? XN(multigrapher->x_axis.other_axis_loc) - multigrapher->half_line_width
		  : XN(multigrapher->x_axis.alt_other_axis_loc) + multigrapher->half_line_width);
	ystart = (multigrapher->y_axis.switch_axis_end 
		  ? YN(multigrapher->y_axis.alt_other_axis_loc)
		  : YN(multigrapher->y_axis.other_axis_loc));
	xmid = (multigrapher->x_axis.switch_axis_end 
		? XN(multigrapher->x_axis.alt_other_axis_loc)
		: XN(multigrapher->x_axis.other_axis_loc));
	ymid = ystart;
	xend = xmid;
	yend = (multigrapher->y_axis.switch_axis_end 
		? YN(multigrapher->y_axis.other_axis_loc) - multigrapher->half_line_width
		: YN(multigrapher->y_axis.alt_other_axis_loc) + multigrapher->half_line_width);
	
	pl_fmove_r (multigrapher->plotter, xstart, ystart);
	pl_fcont_r (multigrapher->plotter, xmid, ymid);
	pl_fcont_r (multigrapher->plotter, xend, yend);
      }
      break;
    case AXES_AT_ORIGIN:
      {
	double xpos, ypos;
	
	xpos = (multigrapher->x_axis.switch_axis_end 
		? XN(multigrapher->x_axis.other_axis_loc)
		: XN(multigrapher->x_axis.alt_other_axis_loc));
	ypos = (multigrapher->y_axis.switch_axis_end 
		? YN(multigrapher->y_axis.alt_other_axis_loc)
		: YN(multigrapher->y_axis.other_axis_loc));
	
	pl_fline_r (multigrapher->plotter, 
		    xpos, YP(YSQ(0.0)) - multigrapher->half_line_width,
		    xpos, YP(YSQ(1.0)) + multigrapher->half_line_width);
	pl_fline_r (multigrapher->plotter, 
		    XP(XSQ(0.0)) - multigrapher->half_line_width, ypos, 
		    XP(XSQ(1.0)) + multigrapher->half_line_width, ypos);
      }
      break;
    case NO_AXES:
    default:
      break;
    }

  /* 3.  PLOT TICK MARKS, GRID LINES, AND TICK LABELS ON ABSCISSA */

  if (multigrapher->grid_spec != NO_AXES && !multigrapher->x_axis.omit_ticks
      && !multigrapher->x_axis.user_specified_subsubticks)
    {
      int i;
      double xval, xrange = multigrapher->x_trans.input_max - multigrapher->x_trans.input_min;
      /* there is no way you could use longer labels on tick marks! */
      char labelbuf[2048];

      /* switch to our font for drawing x axis label and tick labels */
      pl_fontname_r (multigrapher->plotter, multigrapher->x_axis.font_name);
      pl_ffontsize_r (multigrapher->plotter, SS(multigrapher->x_axis.font_size));
      
      for (i = multigrapher->x_axis.min_tick_count; i <= multigrapher->x_axis.max_tick_count; i++) 
	/* tick range can be empty */
	{
	  xval = i * multigrapher->x_axis.tick_spacing;
	  	  
	  /* discard tick locations outside plotting area */
	  if (xval < multigrapher->x_trans.input_min - FUZZ * xrange 
	      || xval > multigrapher->x_trans.input_max + FUZZ * xrange)
	    continue;

	  /* Plot the abscissa tick labels. */
	  if (!multigrapher->y_axis.switch_axis_end
	      && !(multigrapher->grid_spec == AXES_AT_ORIGIN
		   /* don't plot label if it could run into an axis */
		   && NEAR_EQUALITY (xval, multigrapher->x_axis.other_axis_loc, 
				     multigrapher->x_trans.input_range)
		   && (multigrapher->y_axis.other_axis_loc != multigrapher->y_trans.input_min)
		   && (multigrapher->y_axis.other_axis_loc != multigrapher->y_trans.input_max)))
	    /* print labels below bottom boundary */
	    {
	      pl_fmove_r (multigrapher->plotter, 
			  XV (xval),
			  YN (multigrapher->y_axis.other_axis_loc)
			  - (SS ((multigrapher->tick_size >= 0.0 ? 0.75 : 1.75) * fabs(multigrapher->tick_size))
			     + multigrapher->half_line_width));
	      print_tick_label (labelbuf, 
				&multigrapher->x_axis, &multigrapher->x_trans,
				(multigrapher->x_axis.type == A_LOG10) ? pow (10.0, xval) : xval);
	      pl_alabel_r (multigrapher->plotter, 'c', 't', labelbuf);
	      multigrapher->x_axis.labelled_ticks++;
	    }
	  else
	    /* print labels above top boundary */
	    if (multigrapher->y_axis.switch_axis_end
	      && !(multigrapher->grid_spec == AXES_AT_ORIGIN
		   /* don't plot label if it could run into an axis */
		   && NEAR_EQUALITY (xval, multigrapher->x_axis.other_axis_loc, 
				     multigrapher->x_trans.input_range)
		   && (multigrapher->y_axis.other_axis_loc != multigrapher->y_trans.input_min)
		   && (multigrapher->y_axis.other_axis_loc != multigrapher->y_trans.input_max)))
	      {
		pl_fmove_r (multigrapher->plotter, 
			    XV (xval),
			    YN (multigrapher->y_axis.alt_other_axis_loc)
			    + (SS ((multigrapher->tick_size >= 0.0 ? 0.75 : 1.75) * fabs(multigrapher->tick_size))
			     + multigrapher->half_line_width));
		print_tick_label (labelbuf, 
				  &multigrapher->x_axis, &multigrapher->x_trans,
				  (multigrapher->x_axis.type == A_LOG10) ? pow (10.0, xval) : xval);
		pl_alabel_r (multigrapher->plotter, 'c', 'b', labelbuf);
		multigrapher->x_axis.labelled_ticks++;
	      }
	  
	  /* Plot the abscissa tick marks, and vertical grid lines. */
	  switch (multigrapher->grid_spec)
	    {
	    case AXES_AND_BOX_AND_GRID:
	      pl_linemod_r (multigrapher->plotter, "dotted");
	      pl_fmove_r (multigrapher->plotter, XV(xval), YP(YSQ(0.0)));
	      pl_fcont_r (multigrapher->plotter, XV(xval), YP(YSQ(1.0)));
	      pl_linemod_r (multigrapher->plotter, "solid");
	      /* fall through */
	    case AXES_AND_BOX:
	      if (!multigrapher->y_axis.switch_axis_end)
		{
		  pl_fmove_r (multigrapher->plotter, 
			      XV (xval), 
			      YN (multigrapher->y_axis.alt_other_axis_loc));
		  pl_fcont_r (multigrapher->plotter, 
			      XV (xval), 
			      YN (multigrapher->y_axis.alt_other_axis_loc)
			      - (SS (multigrapher->tick_size)
				 + (multigrapher->tick_size > 0.0 ? multigrapher->half_line_width
				    : -multigrapher->half_line_width)));
		}
	      else
		{
		  pl_fmove_r (multigrapher->plotter, 
			      XV (xval), 
			      YN (multigrapher->y_axis.other_axis_loc));
		  pl_fcont_r (multigrapher->plotter, 
			      XV (xval), 
			      YN (multigrapher->y_axis.other_axis_loc)
			      + (SS (multigrapher->tick_size)
				 + (multigrapher->tick_size > 0.0 ? multigrapher->half_line_width
				    : -multigrapher->half_line_width)));
		}
	      /* fall through */
	    case AXES:
	    case AXES_AT_ORIGIN:
	      if (!multigrapher->y_axis.switch_axis_end)
		{
		  pl_fmove_r (multigrapher->plotter, 
			      XV (xval), 
			      YN (multigrapher->y_axis.other_axis_loc));
		  pl_fcont_r (multigrapher->plotter, 
			      XV (xval), 
			      YN (multigrapher->y_axis.other_axis_loc)
			      + (SS (multigrapher->tick_size)
				 + (multigrapher->tick_size > 0.0 ? multigrapher->half_line_width
				    : -multigrapher->half_line_width)));
		}
	      else
		{
		  pl_fmove_r (multigrapher->plotter, 
			      XV (xval), 
			      YN (multigrapher->y_axis.alt_other_axis_loc));
		  pl_fcont_r (multigrapher->plotter, 
			      XV (xval), 
			      YN (multigrapher->y_axis.alt_other_axis_loc)
			      - (SS (multigrapher->tick_size)
				 + (multigrapher->tick_size > 0.0 ? multigrapher->half_line_width
				    : -multigrapher->half_line_width)));
		}
	      break;
	    default:		/* shouldn't happen */
	      break;
	    }
	}
      
      if (multigrapher->x_axis.have_lin_subticks)
	{
	  double subtick_size;	/* libplot coordinates */
	  
	  /* linearly spaced subticks on log axes are as long as reg. ticks */
	  subtick_size = (multigrapher->x_axis.type == A_LOG10 
			  ? SS(multigrapher->tick_size) : SS(multigrapher->subtick_size));

	  /* Plot the linearly spaced subtick marks on the abscissa */
	  for (i = multigrapher->x_axis.min_lin_subtick_count; i <= multigrapher->x_axis.max_lin_subtick_count; i++) 
	    /* tick range can be empty */
	    {
	      xval = i * multigrapher->x_axis.lin_subtick_spacing;
	  	  
	      /* discard subtick locations outside plotting area */
	      if (xval < multigrapher->x_trans.input_min - FUZZ * xrange 
		  || xval > multigrapher->x_trans.input_max + FUZZ * xrange)
		continue;

	      switch (multigrapher->grid_spec)
		{
		case AXES_AND_BOX_AND_GRID:
		case AXES_AND_BOX:
		  /* draw on both sides */
		  if (!multigrapher->y_axis.switch_axis_end)
		    {
		      pl_fmove_r (multigrapher->plotter, 
				  XV (xval),
				  YN (multigrapher->y_axis.alt_other_axis_loc));
		      pl_fcont_r (multigrapher->plotter, 
				  XV (xval), 
				  YN (multigrapher->y_axis.alt_other_axis_loc)
				  - (subtick_size
				     + (subtick_size > 0.0 ? multigrapher->half_line_width
					: -multigrapher->half_line_width)));
		    }
		  else
		    {
		      pl_fmove_r (multigrapher->plotter, 
				  XV (xval), 
				  YN (multigrapher->y_axis.other_axis_loc));
		      pl_fcont_r (multigrapher->plotter, 
				  XV (xval), 
				  YN (multigrapher->y_axis.other_axis_loc)
				  + (subtick_size
				     + (subtick_size > 0.0 ? multigrapher->half_line_width
					: -multigrapher->half_line_width)));
		    }
		  /* fall through */
		case AXES:
		case AXES_AT_ORIGIN:
		  if (!multigrapher->y_axis.switch_axis_end)
		    /* draw on only one side */
		    {
		      pl_fmove_r (multigrapher->plotter, 
				  XV (xval), 
				  YN (multigrapher->y_axis.other_axis_loc));
		      pl_fcont_r (multigrapher->plotter, 
				  XV (xval), 
				  YN (multigrapher->y_axis.other_axis_loc)
				  + (subtick_size
				     + (subtick_size > 0.0 ? multigrapher->half_line_width
					: -multigrapher->half_line_width)));
		    }
		  else
		    {
		      pl_fmove_r (multigrapher->plotter, 
				  XV (xval), 
				  YN (multigrapher->y_axis.alt_other_axis_loc));
		      pl_fcont_r (multigrapher->plotter, 
				  XV (xval), 
				  YN (multigrapher->y_axis.alt_other_axis_loc)
				  - (subtick_size
				     + (subtick_size > 0.0 ? multigrapher->half_line_width
					: -multigrapher->half_line_width)));
		    }
		  break;
		default:		/* shouldn't happen */
		  break;
		}
	    }
	}

      /* plot a vertical dotted line at x = 0 */
      if (multigrapher->grid_spec != AXES_AT_ORIGIN 
	  && multigrapher->x_axis.type == A_LINEAR
	  && multigrapher->x_trans.input_min * multigrapher->x_trans.input_max < 0.0)
	{
	  pl_linemod_r (multigrapher->plotter, "dotted");
	  pl_fline_r (multigrapher->plotter, 
		      XV(0.0), YP(YSQ(0.0)), XV(0.0), YP(YSQ(1.0)));
	  pl_linemod_r (multigrapher->plotter, "solid");	  
	}
    }
  
  /* 4.  PLOT TICK MARKS, GRID LINES, AND TICK LABELS ON ORDINATE */

  if (multigrapher->grid_spec != NO_AXES && !multigrapher->y_axis.omit_ticks
      && !multigrapher->y_axis.user_specified_subsubticks)
    {
      int i;
      double yval, yrange = multigrapher->y_trans.input_max - multigrapher->y_trans.input_min;
      /* there is no way you could use longer labels on tick marks! */
      char labelbuf[2048];

      /* switch to our font for drawing y axis label and tick labels */
      pl_fontname_r (multigrapher->plotter, multigrapher->y_axis.font_name);
      pl_ffontsize_r (multigrapher->plotter, SS(multigrapher->y_axis.font_size));
      
      for (i = multigrapher->y_axis.min_tick_count; i <= multigrapher->y_axis.max_tick_count; i++) 
	/* range can be empty */
	{
	  yval = i * multigrapher->y_axis.tick_spacing;
	  
	  /* discard tick locations outside plotting area */
	  if (yval < multigrapher->y_trans.input_min - FUZZ * yrange 
	      || yval > multigrapher->y_trans.input_max + FUZZ * yrange)
	    continue;

	  /* Plot the ordinate tick labels. */
	  if (!multigrapher->x_axis.switch_axis_end
	      && !(multigrapher->grid_spec == AXES_AT_ORIGIN
		   /* don't plot label if it could run into an axis */
		   && NEAR_EQUALITY (yval, multigrapher->y_axis.other_axis_loc, 
				     multigrapher->y_trans.input_range)
		   && (multigrapher->x_axis.other_axis_loc != multigrapher->x_trans.input_min)
		   && (multigrapher->x_axis.other_axis_loc != multigrapher->x_trans.input_max)))
	    /* print labels to left of left boundary */
	    {
	      double new_width;

	      pl_fmove_r (multigrapher->plotter, 
			  XN (multigrapher->x_axis.other_axis_loc)
			  - (SS((multigrapher->tick_size >= 0.0 ? 0.75 : 1.75) 
				* fabs(multigrapher->tick_size))
			     + multigrapher->half_line_width),
			  YV (yval));
	      print_tick_label (labelbuf, 
				&multigrapher->y_axis, &multigrapher->y_trans,
				(multigrapher->y_axis.type == A_LOG10) ? pow (10.0, yval) : yval);
	      new_width = pl_flabelwidth_r (multigrapher->plotter, labelbuf);
	      pl_alabel_r (multigrapher->plotter, 'r', 'c', labelbuf);
	      multigrapher->y_axis.max_label_width = DMAX(multigrapher->y_axis.max_label_width, new_width);
	      multigrapher->y_axis.labelled_ticks++;
	    }
	  else
	    /* print labels to right of right boundary */
	    if (multigrapher->x_axis.switch_axis_end
		&& !(multigrapher->grid_spec == AXES_AT_ORIGIN
		     /* don't plot label if it could run into an axis */
		     && NEAR_EQUALITY (yval, multigrapher->y_axis.other_axis_loc, 
				       multigrapher->y_trans.input_range)
		     && (multigrapher->x_axis.other_axis_loc != multigrapher->x_trans.input_min)
		     && (multigrapher->x_axis.other_axis_loc != multigrapher->x_trans.input_max)))
	    {
	      double new_width;

	      pl_fmove_r (multigrapher->plotter, 
			  XN (multigrapher->x_axis.alt_other_axis_loc)
			  + (SS((multigrapher->tick_size >= 0.0 ? 0.75 : 1.75) 
				* fabs(multigrapher->tick_size))
			     + multigrapher->half_line_width),
			  YV (yval));
	      print_tick_label (labelbuf, 
				&multigrapher->y_axis, &multigrapher->y_trans,
				(multigrapher->y_axis.type == A_LOG10) ? pow (10.0, yval) : yval);
	      new_width = pl_flabelwidth_r (multigrapher->plotter, labelbuf);
	      pl_alabel_r (multigrapher->plotter, 'l', 'c', labelbuf);
	      multigrapher->y_axis.max_label_width = DMAX(multigrapher->y_axis.max_label_width, new_width);
	      multigrapher->y_axis.labelled_ticks++;
	    }
	  
	  /* Plot the tick marks on the y-axis, and horizontal grid lines. */
	  switch (multigrapher->grid_spec)
	    {
	    case AXES_AND_BOX_AND_GRID:
	      pl_linemod_r (multigrapher->plotter, "dotted");
	      pl_fmove_r (multigrapher->plotter, XP(XSQ(0.0)), YV (yval));
	      pl_fcont_r (multigrapher->plotter, XP(XSQ(1.0)), YV (yval));
	      pl_linemod_r (multigrapher->plotter, "solid");
	      /* fall through */
	    case AXES_AND_BOX:
	      if (!multigrapher->x_axis.switch_axis_end)
		{
		  pl_fmove_r (multigrapher->plotter, 
			      XN (multigrapher->x_axis.alt_other_axis_loc),
			      YV (yval));
		  pl_fcont_r (multigrapher->plotter, 
			      XN (multigrapher->x_axis.alt_other_axis_loc)
			      - (SS (multigrapher->tick_size) 
				 + (multigrapher->tick_size > 0.0 ? multigrapher->half_line_width
				    : -multigrapher->half_line_width)),
			      YV (yval));
		}
	      else
		{
		  pl_fmove_r (multigrapher->plotter, 
			      XN (multigrapher->x_axis.other_axis_loc),
			      YV (yval));
		  pl_fcont_r (multigrapher->plotter, 
			      XN (multigrapher->x_axis.other_axis_loc)
			      + (SS (multigrapher->tick_size) 
				 + (multigrapher->tick_size > 0.0 ? multigrapher->half_line_width
				    : -multigrapher->half_line_width)),
			      YV (yval));
		}
	      /* fall through */
	    case AXES:
	    case AXES_AT_ORIGIN:
	      if (!multigrapher->x_axis.switch_axis_end)
		{
		  pl_fmove_r (multigrapher->plotter, 
			      XN (multigrapher->x_axis.other_axis_loc),
			      YV (yval));
		  pl_fcont_r (multigrapher->plotter, 
			      XN (multigrapher->x_axis.other_axis_loc)
			      + (SS (multigrapher->tick_size) 
				 + (multigrapher->tick_size > 0.0 ? multigrapher->half_line_width
				    : -multigrapher->half_line_width)),
			      YV (yval));
		}
	      else
		{
		  pl_fmove_r (multigrapher->plotter, 
			      XN (multigrapher->x_axis.alt_other_axis_loc),
			      YV (yval));
		  pl_fcont_r (multigrapher->plotter, 
			      XN (multigrapher->x_axis.alt_other_axis_loc)
			      - (SS (multigrapher->tick_size) 
				 + (multigrapher->tick_size > 0.0 ? multigrapher->half_line_width
				    : -multigrapher->half_line_width)),
			      YV (yval));
		}
	      break;
	    default:		/* shouldn't happen */
	      break;
	    }
	}

      if (multigrapher->y_axis.have_lin_subticks)
	{
	  double subtick_size;	/* libplot coordinates */

	  /* linearly spaced subticks on a log axis are as long as regular ticks */
	  subtick_size = (multigrapher->y_axis.type == A_LOG10 
			  ? SS(multigrapher->tick_size) : SS(multigrapher->subtick_size));

	  /* Plot the linearly spaced subtick marks on the ordinate */
	  for (i = multigrapher->y_axis.min_lin_subtick_count; i <= multigrapher->y_axis.max_lin_subtick_count; i++) 
	    /* range can be empty */
	    {
	      yval = i * multigrapher->y_axis.lin_subtick_spacing;
	      
	      /* discard subtick locations outside plotting area */
	      if (yval < multigrapher->y_trans.input_min - FUZZ * yrange 
		  || yval > multigrapher->y_trans.input_max + FUZZ * yrange)
		continue;

	      /* Plot the tick marks on the y-axis, and horizontal grid lines. */
	      switch (multigrapher->grid_spec)
		{
		case AXES_AND_BOX_AND_GRID:
		case AXES_AND_BOX:
		  if (!multigrapher->x_axis.switch_axis_end)
		    {
		      pl_fmove_r (multigrapher->plotter, 
				  XN (multigrapher->x_axis.alt_other_axis_loc),
				  YV (yval));
		      pl_fcont_r (multigrapher->plotter, 
				  XN (multigrapher->x_axis.alt_other_axis_loc)
				  - (subtick_size
				     + (subtick_size > 0.0 ? multigrapher->half_line_width
					: -multigrapher->half_line_width)),
				  YV (yval));
		    }
		  else
		    {
		      pl_fmove_r (multigrapher->plotter, 
				  XN (multigrapher->x_axis.other_axis_loc),
				  YV (yval));
		      pl_fcont_r (multigrapher->plotter, 
				  XN (multigrapher->x_axis.other_axis_loc)
				  + (subtick_size
				     + (subtick_size > 0.0 ? multigrapher->half_line_width
					: -multigrapher->half_line_width)),
				  YV (yval));
		    }
		  /* fall through */
		case AXES:
		case AXES_AT_ORIGIN:
		  if (!multigrapher->x_axis.switch_axis_end)
		    {
		      pl_fmove_r (multigrapher->plotter, 
				  XN (multigrapher->x_axis.other_axis_loc),
				  YV (yval));
		      pl_fcont_r (multigrapher->plotter, 
				  XN (multigrapher->x_axis.other_axis_loc)
				  + (subtick_size
				     + (subtick_size > 0.0 ? multigrapher->half_line_width
					: -multigrapher->half_line_width)),
				  YV (yval));
		    }
		  else
		    {
		      pl_fmove_r (multigrapher->plotter, 
				  XN (multigrapher->x_axis.alt_other_axis_loc),
				  YV (yval));
		      pl_fcont_r (multigrapher->plotter, 
				  XN (multigrapher->x_axis.alt_other_axis_loc)
				  - (subtick_size
				     + (subtick_size > 0.0 ? multigrapher->half_line_width
					: -multigrapher->half_line_width)),
				  YV (yval));
		    }
		  break;
		default:		/* shouldn't happen */
		  break;
		}
	    }
	}
	  
      /* plot a horizontal dotted line at y = 0 */
      if (multigrapher->grid_spec != AXES_AT_ORIGIN 
	  && multigrapher->y_axis.type == A_LINEAR
	  && multigrapher->y_trans.input_min * multigrapher->y_trans.input_max < 0.0)
	{
	  pl_linemod_r (multigrapher->plotter, "dotted");
	  pl_fline_r (multigrapher->plotter, 
		      XP(XSQ(0.0)), YV(0.0), XP(XSQ(1.0)), YV(0.0));
	  pl_linemod_r (multigrapher->plotter, "solid");	  
	}
    }

  /* 5.  DRAW LOGARITHMIC SUBSUBTICKS AND THEIR LABELS ON ABSCISSA */

  /* first, draw normal logarithmic subsubticks if any */
  if (multigrapher->grid_spec != NO_AXES && multigrapher->x_axis.have_normal_subsubticks
      && !multigrapher->x_axis.user_specified_subsubticks && !multigrapher->x_axis.omit_ticks)
    {
      int i, m, imin, imax;
      double xval, xrange = multigrapher->x_trans.input_max - multigrapher->x_trans.input_min;

      /* compute an integer range (of powers of 10) large enough to include
	 the entire desired axis */
      imin = (int)(floor (multigrapher->x_trans.input_min - FUZZ * xrange));
      imax = (int)(ceil (multigrapher->x_trans.input_max + FUZZ * xrange));

      for (i = imin; i < imax; i++)
	{
	  for (m = 1; m <= 9 ; m++)
	    {
	      xval = i + log10 ((double)m);

	      /* Plot subsubtick and label, if desired. */
	      /* N.B. if tick is outside axis range, nothing will be printed */
	      plot_abscissa_log_subsubtick (multigrapher, xval);
	    }
	}
    }

  /* second, draw user-specified logarithmic subsubticks instead, if any */
  if (multigrapher->grid_spec != NO_AXES && multigrapher->x_axis.user_specified_subsubticks
      && !multigrapher->x_axis.omit_ticks)
    {
      int i, imin, imax;
      double xval, xrange = multigrapher->x_trans.input_max - multigrapher->x_trans.input_min;
      
      /* compute an integer range large enough to include the entire
	 desired axis */
      imin = (int)(floor (pow (10.0, multigrapher->x_trans.input_min - FUZZ * xrange) 
			  / multigrapher->x_axis.subsubtick_spacing));
      imax = (int)(ceil (pow (10.0, multigrapher->x_trans.input_max + FUZZ * xrange) 
			 / multigrapher->x_axis.subsubtick_spacing));
      
      /* draw user-specified subsubticks */
      for (i = imin; i <= imax; i++)
	{
	  xval = log10 (i * multigrapher->x_axis.subsubtick_spacing);

	  /* Plot subsubtick and label, if desired. */
	  /* N.B. if tick is outside axis range, nothing will be printed */
	  plot_abscissa_log_subsubtick (multigrapher, xval);
	}
    }

  /* 6.  DRAW LOGARITHMIC SUBSUBTICKS AND THEIR LABELS ON ORDINATE */
  
  /* first, draw normal logarithmic subsubticks if any */
  if (multigrapher->grid_spec != NO_AXES && multigrapher->y_axis.have_normal_subsubticks
      && !multigrapher->y_axis.user_specified_subsubticks && !multigrapher->y_axis.omit_ticks)
    {
      int i, m, imin, imax;
      double yval, yrange = multigrapher->y_trans.input_max - multigrapher->y_trans.input_min;

      /* compute an integer range (of powers of 10) large enough to include
	 the entire desired axis */
      imin = (int)(floor (multigrapher->y_trans.input_min - FUZZ * yrange));
      imax = (int)(ceil (multigrapher->y_trans.input_max + FUZZ * yrange));

      /* draw normal subticks */
      for (i = imin; i < imax; i++)
	{
	  for (m = 1; m <= 9; m++)
	    {
	      yval = i + log10 ((double)m);

	      /* Plot subsubtick and label, if desired. */
	      /* N.B. if tick is outside axis range, nothing will be printed */
	      plot_ordinate_log_subsubtick (multigrapher, yval);
	    }
	}
    }

  /* second, draw user-specified logarithmic subsubticks instead, if any */
  if (multigrapher->grid_spec != NO_AXES && multigrapher->y_axis.user_specified_subsubticks
      && !multigrapher->y_axis.omit_ticks)
    {
      int i, imin, imax;
      double yval, yrange = multigrapher->y_trans.input_max - multigrapher->y_trans.input_min;
      
      /* compute an integer range large enough to include the entire
	 desired axis */
      imin = (int)(floor (pow (10.0, multigrapher->y_trans.input_min - FUZZ * yrange) 
			  / multigrapher->y_axis.subsubtick_spacing));
      imax = (int)(ceil (pow (10.0, multigrapher->y_trans.input_max + FUZZ * yrange) 
			 / multigrapher->y_axis.subsubtick_spacing));
      
      /* draw user-specified subsubticks */
      for (i = imin; i <= imax; i++)
	{
	  yval = log10 (i * multigrapher->y_axis.subsubtick_spacing);

	  /* Plot subsubtick and label, if desired. */
	  /* N.B. if tick is outside axis range, nothing will be printed */
	  plot_ordinate_log_subsubtick (multigrapher, yval);
	}
    }

  /* 7.  DRAW THE ABSCISSA LABEL */

      if ((multigrapher->grid_spec != NO_AXES)
      && multigrapher->x_axis.label != NULL && multigrapher->x_axis.label != '\0')
    {
      double x_axis_font_size;
      double xloc;

      /* switch to our font for drawing x axis label and tick labels */
      pl_fontname_r (multigrapher->plotter, multigrapher->x_axis.font_name);
      x_axis_font_size = pl_ffontsize_r (multigrapher->plotter, 
					 SS(multigrapher->x_axis.font_size));

      if (multigrapher->grid_spec != AXES_AT_ORIGIN)
	/* center the label on the axis */
	xloc = 0.5 * (multigrapher->x_trans.input_max + multigrapher->x_trans.input_min);
      else
	{
	  if ((multigrapher->y_axis.other_axis_loc == multigrapher->y_trans.input_min)
	      || (multigrapher->y_axis.other_axis_loc == multigrapher->y_trans.input_max))

	    xloc = 0.5 * (multigrapher->x_trans.input_max + multigrapher->x_trans.input_min);
	  else
	    /* center label in the larger of the two halves */
	    xloc = 
	      multigrapher->x_trans.input_max-multigrapher->x_axis.other_axis_loc >= multigrapher->x_axis.other_axis_loc-multigrapher->x_trans.input_min ?
		0.5 * (multigrapher->x_trans.input_max + multigrapher->x_axis.other_axis_loc) :
		  0.5 * (multigrapher->x_axis.other_axis_loc + multigrapher->x_trans.input_min);
	}
      
      if (!multigrapher->y_axis.switch_axis_end) /* axis on bottom, label below it */
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XV (xloc), 
		      YN (multigrapher->y_axis.other_axis_loc)
		      - (SS ((multigrapher->tick_size >= 0.0 ? 0.875 : 2.125) 
			     * fabs(multigrapher->tick_size))
			 + (6 * x_axis_font_size)/5
			 + multigrapher->half_line_width));
	  pl_alabel_r (multigrapher->plotter, 
		       'c', 't', multigrapher->x_axis.label);
	}
      else			/* axis on top, label above it */
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XV (xloc), 
		      YN (multigrapher->y_axis.alt_other_axis_loc)
		      + (SS ((multigrapher->tick_size >= 0.0 ? 0.875 : 2.125) 
			     * fabs(multigrapher->tick_size))
			 + (6 * x_axis_font_size)/5
			 + multigrapher->half_line_width));
	  pl_alabel_r (multigrapher->plotter, 
		       'c', 'b', multigrapher->x_axis.label);
	}
    }

  /* 8.  DRAW THE ORDINATE LABEL */

  if ((multigrapher->grid_spec != NO_AXES)
      && (multigrapher->y_axis.label != NULL && *(multigrapher->y_axis.label) != '\0'))
    {
      double y_axis_font_size;
      double yloc;

      /* switch to our font for drawing y axis label and tick labels */
      pl_fontname_r (multigrapher->plotter, multigrapher->y_axis.font_name);
      y_axis_font_size = pl_ffontsize_r (multigrapher->plotter, 
					 SS(multigrapher->y_axis.font_size));

      if (multigrapher->grid_spec != AXES_AT_ORIGIN)
	/* center the label on the axis */
	yloc = 0.5 * (multigrapher->y_trans.input_min + multigrapher->y_trans.input_max);
      else
	{
	  if ((multigrapher->x_axis.other_axis_loc == multigrapher->x_trans.input_min)
	      || (multigrapher->x_axis.other_axis_loc == multigrapher->x_trans.input_max))
	    yloc = 0.5 * (multigrapher->y_trans.input_min + multigrapher->y_trans.input_max);
	  else
	    /* center label in the larger of the two halves */
	    yloc = 
	      multigrapher->y_trans.input_max-multigrapher->y_axis.other_axis_loc >= multigrapher->y_axis.other_axis_loc-multigrapher->y_trans.input_min ?
		0.5 * (multigrapher->y_trans.input_max + multigrapher->y_axis.other_axis_loc) :
		  0.5 * (multigrapher->y_axis.other_axis_loc + multigrapher->y_trans.input_min);
	}
      
/* a relic of temps perdus */
#define libplot_has_font_metrics 1

      if (!multigrapher->x_axis.switch_axis_end)
	{
	  pl_fmove_r (multigrapher->plotter,
		      XN (multigrapher->x_axis.other_axis_loc)
		      - (libplot_has_font_metrics ?
			 (SS((multigrapher->tick_size >= 0.0 ? 0.75 : 1.75) 
			     * fabs(multigrapher->tick_size)) 
			  + 1.15 * multigrapher->y_axis.max_label_width
			  + 0.5 * y_axis_font_size
			  + multigrapher->half_line_width)
			 : (SS((multigrapher->tick_size >= 0.0 ? 0.75 : 1.75) 
			       * fabs(multigrapher->tick_size)) /* backup */
			    + 1.0 * y_axis_font_size
			    + multigrapher->half_line_width)),
		      YV(yloc));
	  
	  if (libplot_has_font_metrics
	      && !multigrapher->no_rotate_y_label) /* can rotate label */
	    {
	      pl_textangle_r (multigrapher->plotter, 90);
	      pl_alabel_r (multigrapher->plotter, 
			   'c', 'x', multigrapher->y_axis.label);
	      pl_textangle_r (multigrapher->plotter, 0);
	    }
	  else
	    /* non-rotated axis label, right justified */
	    pl_alabel_r (multigrapher->plotter, 
			 'r', 'c', multigrapher->y_axis.label);
	}
      else
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.alt_other_axis_loc)
		      + (libplot_has_font_metrics ?
			 (SS((multigrapher->tick_size >= 0.0 ? 0.75 : 1.75) 
			     * fabs(multigrapher->tick_size)) 
			  + 1.15 * multigrapher->y_axis.max_label_width 
			  + 0.5 * y_axis_font_size
			  + multigrapher->half_line_width)
			 : (SS((multigrapher->tick_size >= 0.0 ? 0.75 : 1.75) 
			       * fabs(multigrapher->tick_size)) /* backup */
			    + 1.0 * y_axis_font_size
			    + multigrapher->half_line_width)), 
		      YV(yloc));
	  
	  if (libplot_has_font_metrics
	      && !multigrapher->no_rotate_y_label) /* can rotate label */
	    {
	      pl_textangle_r (multigrapher->plotter, 90);
	      pl_alabel_r (multigrapher->plotter, 
			   'c', 't', multigrapher->y_axis.label);
	      pl_textangle_r (multigrapher->plotter, 0);
	    }
	  else
	    /* non-rotated axis label, left justified */
	    pl_alabel_r (multigrapher->plotter, 
			 'l', 'c', multigrapher->y_axis.label);
	}
    }

  /* END OF TASKS */

  /* flush frame to device */
  pl_flushpl_r (multigrapher->plotter);

  pl_restorestate_r (multigrapher->plotter);

  if (multigrapher->grid_spec != NO_AXES)
    {
      if (!tick_warning_printed && 
	  ((!multigrapher->x_axis.omit_ticks && multigrapher->x_axis.labelled_ticks <= 2)
	   || (!multigrapher->y_axis.omit_ticks && multigrapher->y_axis.labelled_ticks <= 2)))
	{
	  fprintf (stderr,
		   "%s: the tick spacing is adjusted, as there were too few labelled axis ticks\n",
		   progname);
	  tick_warning_printed = true;
	}
    }
}
  


/* plot_abscissa_log_subsubtick() and plot_ordinate_log_subsubtick() are
   called to plot both normal log subticks and special (user-requested)
   ones */

/* ARGS: xval = log of location */
static void
plot_abscissa_log_subsubtick (Multigrapher *multigrapher, double xval)
{
  double xrange = multigrapher->x_trans.input_max - multigrapher->x_trans.input_min;
  /* there is no way you could use longer labels on tick marks! */
  char labelbuf[2048];
  double tick_size = SS(multigrapher->tick_size); /* for positioning labels */
  double subsubtick_size = SS(multigrapher->subtick_size);
      
  /* switch to our font for drawing x axis label and tick labels */
  pl_fontname_r (multigrapher->plotter, multigrapher->x_axis.font_name);
  pl_ffontsize_r (multigrapher->plotter, SS(multigrapher->x_axis.font_size));
  
  /* discard subsubtick locations outside plotting area */
  if (xval < multigrapher->x_trans.input_min - FUZZ * xrange
      || xval > multigrapher->x_trans.input_max + FUZZ * xrange)
    return;
  
  /* label subsubtick if it seems appropriate */
  if (multigrapher->x_axis.user_specified_subsubticks)
    {
      print_tick_label (labelbuf, 
			&multigrapher->x_axis, &multigrapher->x_trans, 
			pow (10.0, xval));
      if (!multigrapher->y_axis.switch_axis_end)
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XV (xval),
		      YN (multigrapher->y_axis.other_axis_loc)
		      - ((tick_size >= 0 ? 0.75 : 1.75)
			 * fabs((double)tick_size)
			 + multigrapher->half_line_width));
	  pl_alabel_r (multigrapher->plotter, 'c', 't', labelbuf);
	  multigrapher->x_axis.labelled_ticks++;
	}
      else
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XV (xval),
		      YN (multigrapher->y_axis.alt_other_axis_loc)
		      + ((tick_size >= 0 ? 0.75 : 1.75) 
			 * fabs((double)tick_size)
			 + multigrapher->half_line_width));
	  pl_alabel_r (multigrapher->plotter, 'c', 'b', labelbuf);
	  multigrapher->x_axis.labelled_ticks++;
	}
    }
  
  /* draw subsubtick */
  switch (multigrapher->grid_spec)
    {
    case AXES_AND_BOX_AND_GRID:
      pl_linemod_r (multigrapher->plotter, "dotted");
      pl_fmove_r (multigrapher->plotter, XV (xval), YP(YSQ(0.0)));
      pl_fcont_r (multigrapher->plotter, XV (xval), YP(YSQ(1.0)));
      pl_linemod_r (multigrapher->plotter, "solid");
      /* fall through */
    case AXES_AND_BOX:
      if (!multigrapher->y_axis.switch_axis_end)
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XV (xval), 
		      YN (multigrapher->y_axis.alt_other_axis_loc));
	  pl_fcont_r (multigrapher->plotter, 
		      XV (xval),
		      YN (multigrapher->y_axis.alt_other_axis_loc)
		      - (subsubtick_size
			 + (subsubtick_size > 0.0
			    ? multigrapher->half_line_width
			    : -multigrapher->half_line_width)));
	}
      else
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XV (xval), 
		      YN (multigrapher->y_axis.other_axis_loc));
	  pl_fcont_r (multigrapher->plotter, 
		      XV (xval),
		      YN (multigrapher->y_axis.other_axis_loc)
		      + (subsubtick_size
			 + (subsubtick_size > 0.0
			    ? multigrapher->half_line_width
			    : -multigrapher->half_line_width)));
	}
      /* fall through */
    case AXES:
    case AXES_AT_ORIGIN:
      if (!multigrapher->y_axis.switch_axis_end)
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XV (xval), 
		      YN (multigrapher->y_axis.other_axis_loc));
	  pl_fcont_r (multigrapher->plotter, 
		      XV (xval), 
		      YN (multigrapher->y_axis.other_axis_loc)
		      + (subsubtick_size
			 + (subsubtick_size > 0.0 
			    ? multigrapher->half_line_width
			    : -multigrapher->half_line_width)));
	}
      else
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XV (xval), 
		      YN (multigrapher->y_axis.alt_other_axis_loc));
	  pl_fcont_r (multigrapher->plotter, 
		      XV (xval), 
		      YN (multigrapher->y_axis.alt_other_axis_loc)
		      - (subsubtick_size
			 + (subsubtick_size > 0.0 
			    ? multigrapher->half_line_width
			    : -multigrapher->half_line_width)));
	}
      break;
    default:			/* shouldn't happen */
      break;
    }
}

/* ARGS: yval = log of location */
static void
plot_ordinate_log_subsubtick (Multigrapher *multigrapher, double yval)
{
  double yrange = multigrapher->y_trans.input_max - multigrapher->y_trans.input_min;
  /* there is no way you could use longer labels on tick marks! */
  char labelbuf[2048];
  double tick_size = SS(multigrapher->tick_size); /* for positioning labels */
  double subsubtick_size = SS(multigrapher->subtick_size);
    
  /* switch to our font for drawing y axis label and tick labels */
  pl_fontname_r (multigrapher->plotter, multigrapher->y_axis.font_name);
  pl_ffontsize_r (multigrapher->plotter, SS(multigrapher->y_axis.font_size));
  
  /* discard subsubtick locations outside plotting area */
  if (yval < multigrapher->y_trans.input_min - FUZZ * yrange
      || yval > multigrapher->y_trans.input_max + FUZZ * yrange)
    return;
  
  /* label subsubtick if it seems appropriate */
  if (multigrapher->y_axis.user_specified_subsubticks)		 
    {
      double new_width;
      
      print_tick_label (labelbuf, 
			&multigrapher->y_axis, &multigrapher->y_trans, 
			pow (10.0, yval));
      if (!multigrapher->x_axis.switch_axis_end)
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XN(multigrapher->x_axis.other_axis_loc)
		      - ((tick_size >= 0 ? 0.75 : 1.75) 
			 * fabs((double)tick_size)
			 + multigrapher->half_line_width),
		      YV (yval));
	  new_width = pl_flabelwidth_r (multigrapher->plotter, labelbuf);
	  pl_alabel_r (multigrapher->plotter, 'r', 'c', labelbuf);
	  multigrapher->y_axis.max_label_width = DMAX(multigrapher->y_axis.max_label_width, new_width);
	  multigrapher->y_axis.labelled_ticks++;
	}
      else
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XN(multigrapher->x_axis.alt_other_axis_loc)
		      + ((tick_size >= 0 ? 0.75 : 1.75) 
			 * fabs((double)tick_size)
			 + multigrapher->half_line_width),
		      YV (yval));
	  new_width = pl_flabelwidth_r (multigrapher->plotter, labelbuf);
	  pl_alabel_r (multigrapher->plotter, 'l', 'c', labelbuf);
	  multigrapher->y_axis.max_label_width = DMAX(multigrapher->y_axis.max_label_width, new_width);
	  multigrapher->y_axis.labelled_ticks++;
	}
    }
  
  /* draw subsubtick */
  switch (multigrapher->grid_spec)
    {
    case AXES_AND_BOX_AND_GRID:
      pl_linemod_r (multigrapher->plotter, "dotted");
      pl_fmove_r (multigrapher->plotter, XP(XSQ(0.0)), YV (yval));
      pl_fcont_r (multigrapher->plotter, XP(XSQ(1.0)), YV (yval));
      pl_linemod_r (multigrapher->plotter, "solid");
      /* fall through */
    case AXES_AND_BOX:
      if (!multigrapher->x_axis.switch_axis_end)		      
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.alt_other_axis_loc),
		      YV (yval));
	  pl_fcont_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.alt_other_axis_loc)
		      - (subsubtick_size
			 + (subsubtick_size > 0.0 
			    ? multigrapher->half_line_width
			    : -multigrapher->half_line_width)),
		      YV (yval));
	}
      else
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.other_axis_loc),
		      YV (yval));
	  pl_fcont_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.other_axis_loc)
		      + (subsubtick_size
			 + (subsubtick_size > 0.0 
			    ? multigrapher->half_line_width
			    : -multigrapher->half_line_width)),
		      YV (yval));
	}
      /* fall through */
    case AXES:
    case AXES_AT_ORIGIN:
      if (!multigrapher->x_axis.switch_axis_end)
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.other_axis_loc),
		      YV (yval));
	  pl_fcont_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.other_axis_loc)
		      + (subsubtick_size
			 + (subsubtick_size > 0.0 
			    ? multigrapher->half_line_width
			    : -multigrapher->half_line_width)),
		      YV (yval));
	}
      else
	{
	  pl_fmove_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.alt_other_axis_loc),
		      YV (yval));
	  pl_fcont_r (multigrapher->plotter, 
		      XN (multigrapher->x_axis.alt_other_axis_loc)
		      - (subsubtick_size
			 + (multigrapher->tick_size > 0.0 
			    ? multigrapher->half_line_width
			    : -multigrapher->half_line_width)),
		      YV (yval));
	}
      break;
    default:	/* shouldn't happen */
      break;
    }
}


/* set_line_style() maps from line modes to physical line modes.  See
 * explanation at head of file. */

static void
set_line_style (Multigrapher *multigrapher, int style, bool use_color)
{
  if (!use_color)		/* monochrome */
    {
      if (style > 0)
	/* don't issue pl_linemod_r() if style<=0, since no polyline will
           be drawn */
	{
	  int i;

	  i = (style - 1) % NO_OF_LINEMODES;
	  pl_linemod_r (multigrapher->plotter, linemodes[i]);      
	}
      
      /* use same color as used for plot frame */
      pl_colorname_r (multigrapher->plotter, multigrapher->frame_color);
    }
  else				/* color */
    {
      int i, j;

      if (style > 0)		/* solid lines, various colors */
	{
	  i = ((style - 1) / NO_OF_LINEMODES) % NO_OF_LINEMODES;
	  j = (style - 1) % NO_OF_LINEMODES;
	  pl_linemod_r (multigrapher->plotter, linemodes[i]);            
	}

      else if (style == 0)	/* use first color, as if -m 1 was spec'd */
				/* (no line will be drawn) */
	j = 0;

      else			/* neg. pl_linemode_r (no line will be drawn)*/
	j = (-style - 1) % (NO_OF_LINEMODES - 1);

      pl_colorname_r (multigrapher->plotter, colorstyle[j]);
    }
}


/* plot_point_array() calls plot_point() on each point in an array of
 * points.
 */

void
plot_point_array (Multigrapher *multigrapher, const Point *p, int length)
{
  int index;

  for (index = 0; index < length; index++)
    plot_point (multigrapher, &(p[index]));
}

/* plot_point() plots a single point, including the appropriate symbol and
 * errorbar(s) if any.  It may call either pl_fcont_r() or pl_fmove_r(),
 * depending on whether the pendown flag is set or not.  Gnuplot-style
 * clipping (clip mode = 0,1,2) is supported.
 *
 * plot_point() makes heavy use of the multigrapher->x_trans and
 * multigrapher->y_trans structures, which specify the linear
 * transformation from user coordinates to device coordinates.  It also
 * updates the multigrapher's internal state variables.  */

void
plot_point (Multigrapher *multigrapher, const Point *point)
{
  double local_x0, local_y0, local_x1, local_y1;
  int clipval;

  /* If new polyline is beginning, take its line style, color/monochrome
     attribute, and line width and fill fraction attributes from the first
     point of the polyline.  We assume all such attribute fields are the
     same for all points in the polyline (our point reader arranges this
     for us). */
  if (!(point->pendown) || multigrapher->first_point_of_polyline)
    {
      int intfill;
      
      set_line_style (multigrapher, point->linemode, point->use_color);

      /* N.B. linewidth < 0.0 means use libplot default */
      pl_flinewidth_r (multigrapher->plotter, 
		       point->line_width * (double)PLOT_SIZE);
      
      if (point->fill_fraction < 0.0)
	intfill = 0;		/* transparent */
      else			/* guaranteed to be <= 1.0 */
	intfill = 1 + IROUND((1.0 - point->fill_fraction) * 0xfffe);
      pl_filltype_r (multigrapher->plotter, intfill);
    }

  /* determine endpoints of new line segment (for the first point of a
     polyline, use a zero-length line segment) */
  if (multigrapher->first_point_of_polyline)
    {
      local_x0 = point->x;
      local_y0 = point->y;
    }
  else
    {
      local_x0 = multigrapher->oldpoint_x;
      local_y0 = multigrapher->oldpoint_y;
    }
  local_x1 = point->x;
  local_y1 = point->y;

  /* save current point for use as endpoint of next line segment */
  multigrapher->oldpoint_x = point->x;
  multigrapher->oldpoint_y = point->y;

  /* apply Cohen-Sutherland clipper to new line segment */
  clipval = clip_line (multigrapher, 
		       &local_x0, &local_y0, &local_x1, &local_y1);

  if (!(clipval & ACCEPTED))	/* rejected in toto */
    {
      pl_fmove_r (multigrapher->plotter, 
		  XV (point->x), YV (point->y)); /* move with pen up */      
      multigrapher->first_point_of_polyline = false;
      return;
    }

  /* not rejected, ideally move with pen down */
  if (point->pendown && (point->linemode > 0))
    {
      switch (multigrapher->clip_mode) /* gnuplot style clipping (0,1, or 2) */
	{
	case 0:
	  if ((clipval & CLIPPED_FIRST) || (clipval & CLIPPED_SECOND))
	    /* clipped on at least one end, so move with pen up */
	    pl_fmove_r (multigrapher->plotter, XV (point->x), YV (point->y));
	  else
	    /* line segment within box, so move with pen down */
	    {
	      if (!multigrapher->first_point_of_polyline)
		pl_fcont_r (multigrapher->plotter, 
			    XV (point->x), YV (point->y));
	      else
		pl_fmove_r (multigrapher->plotter, 
			    XV (point->x), YV (point->y));
	    }
	  break;
	case 1:
	default:
	  if ((clipval & CLIPPED_FIRST) && (clipval & CLIPPED_SECOND))
	    /* both OOB, so move with pen up */
	    pl_fmove_r (multigrapher->plotter, XV (point->x), YV (point->y));
	  else			
	    /* at most one point is OOB */
	    {
	      if (clipval & CLIPPED_FIRST) /*current pt. OOB, new pt. not OOB*/
		{
		  if (!multigrapher->first_point_of_polyline)
		    {
		      /* move to clipped current point, draw line segment */
		      pl_fmove_r (multigrapher->plotter, 
				  XV (local_x0), YV (local_y0));
		      pl_fcont_r (multigrapher->plotter, 
				  XV (point->x), YV (point->y));
		    }
		  else
		    pl_fmove_r (multigrapher->plotter, 
				XV (point->x), YV (point->y));
		}
	      else		/* current point not OOB, new point OOB */
		{
		  if (!multigrapher->first_point_of_polyline)
		    {
		      /* draw line segment to clipped new point */
		      pl_fcont_r (multigrapher->plotter, 
				  XV (local_x1), YV (local_y1));
		      /* N.B. lib's notion of position now differs from ours */
		    }
		  else
		    pl_fmove_r (multigrapher->plotter, 
				XV (point->x), YV (point->y));
		}
	    }
	  break;
	case 2:
	  if ((clipval & CLIPPED_FIRST) || multigrapher->first_point_of_polyline)
	    /* move to clipped current point if necc. */
	    pl_fmove_r (multigrapher->plotter, XV (local_x0), YV (local_y0));
	  
	  /* draw line segment to clipped new point */
	  pl_fcont_r (multigrapher->plotter, XV (local_x1), YV (local_y1));
	  
	  if (clipval & CLIPPED_SECOND)
	    /* new point OOB, so move to new point, breaking polyline */
	    pl_fmove_r (multigrapher->plotter, XV (point->x), YV (point->y)); 
	  break;
	}
    }
  else				/* linemode=0 or pen up; so move with pen up */
    pl_fmove_r (multigrapher->plotter, XV (point->x), YV (point->y)); 

  multigrapher->first_point_of_polyline = false;
  
  /* if target point is OOB, return without plotting symbol or errorbar */
  if (clipval & CLIPPED_SECOND)
    return;

  /* plot symbol and errorbar, doing a pl_savestate_r()--pl_restorestate()
     to keep from breaking the polyline under construction (if any) */
  if (point->symbol >= 32)	/* yow, a character */
    {
      /* will do a font change, so save & restore state */
      pl_savestate_r (multigrapher->plotter);
      plot_errorbar (multigrapher, point);
      pl_fontname_r (multigrapher->plotter, point->symbol_font_name);
      pl_fmarker_r (multigrapher->plotter, XV(point->x), YV(point->y), 
		    point->symbol, SS(point->symbol_size));
      pl_restorestate_r (multigrapher->plotter);
    }

  else if (point->symbol > 0)	/* a marker symbol */
    {
      if (point->linemode > 0)
	/* drawing a line, so (to keep from breaking it) save & restore state*/
	{
	  pl_savestate_r (multigrapher->plotter);
	  plot_errorbar (multigrapher, point); /* may or may not have one */
	  pl_fmarker_r (multigrapher->plotter, XV(point->x), YV(point->y), 
			point->symbol, SS(point->symbol_size));
	  pl_restorestate_r (multigrapher->plotter);
	}
      else
	/* not drawing a line, so just place the marker */
	{
	  plot_errorbar (multigrapher, point);
	  pl_fmarker_r (multigrapher->plotter, XV(point->x), YV(point->y), 
			point->symbol, SS(point->symbol_size));
	}
    }
  
  else if (point->symbol == 0 && point->linemode == 0)
    /* backward compatibility: -m 0 (even with -S 0) plots a dot */
    {
      plot_errorbar (multigrapher, point);
      pl_fmarker_r (multigrapher->plotter, 
		    XV(point->x), YV(point->y), M_DOT, SS(point->symbol_size));
    }

  else				/* no symbol, but may be an errorbar */
    plot_errorbar (multigrapher, point);

  return;
}


/* clip_line() takes two points, the endpoints of a line segment, and
 * destructively passes back two points: the endpoints of the line segment
 * clipped by Cohen-Sutherland to the rectangular plotting area.  Return
 * value contains bitfields ACCEPTED, CLIPPED_FIRST, and CLIPPED_SECOND.
 */

static int
clip_line (Multigrapher *multigrapher, double *x0_p, double *y0_p, double *x1_p, double *y1_p)
{
  double x0 = *x0_p;
  double y0 = *y0_p;
  double x1 = *x1_p;
  double y1 = *y1_p;
  outcode outcode0 = compute_outcode (multigrapher, x0, y0, true);
  outcode outcode1 = compute_outcode (multigrapher, x1, y1, true);  
  bool accepted;
  int clipval = 0;
  
  for ( ; ; )
    {
      if (!(outcode0 | outcode1)) /* accept */
	{
	  accepted = true;
	  break;
	}
      else if (outcode0 & outcode1) /* reject */
	{
	  accepted = false;
	  break;
	}
      else
	{
	  /* at least one endpoint is outside; choose one that is */
	  outcode outcode_out = (outcode0 ? outcode0 : outcode1);
	  double x, y;		/* intersection with clip edge */
	  
	  if (outcode_out & RIGHT)	  
	    {
	      x = multigrapher->x_trans.input_max;
	      y = y0 + (y1 - y0) * (multigrapher->x_trans.input_max - x0) / (x1 - x0);
	    }
	  else if (outcode_out & LEFT)
	    {
	      x = multigrapher->x_trans.input_min;
	      y = y0 + (y1 - y0) * (multigrapher->x_trans.input_min - x0) / (x1 - x0);
	    }
	  else if (outcode_out & TOP)
	    {
	      x = x0 + (x1 - x0) * (multigrapher->y_trans.input_max - y0) / (y1 - y0);
	      y = multigrapher->y_trans.input_max;
	    }
	  else
	    {
	      x = x0 + (x1 - x0) * (multigrapher->y_trans.input_min - y0) / (y1 - y0);
	      y = multigrapher->y_trans.input_min;
	    }
	  
	  if (outcode_out == outcode0)
	    {
	      x0 = x;
	      y0 = y;
	      outcode0 = compute_outcode (multigrapher, x0, y0, true);
	    }
	  else
	    {
	      x1 = x; 
	      y1 = y;
	      outcode1 = compute_outcode (multigrapher, x1, y1, true);
	    }
	}
    }

  if (accepted)
    {
      clipval |= ACCEPTED;
      if ((x0 != *x0_p) || (y0 != *y0_p))
	clipval |= CLIPPED_FIRST;
      if ((x1 != *x1_p) || (y1 != *y1_p))
	clipval |= CLIPPED_SECOND;
      *x0_p = x0;
      *y0_p = y0;
      *x1_p = x1;
      *y1_p = y1;
    }

  return clipval;
}

/* Compute usual Cohen-Sutherland outcode, containing bitfields LEFT,
   RIGHT, BOTTOM, TOP.  Nine possibilities: 
   {LEFT, interior, RIGHT} x {BOTTOM, interior, TOP}.
   The `tolerant' flag specifies how we handle points on the boundary. */
static outcode
compute_outcode (Multigrapher *multigrapher, double x, double y, bool tolerant)
{
  outcode code = 0;
  double xfuzz = FUZZ * multigrapher->x_trans.input_range;
  double yfuzz = FUZZ * multigrapher->y_trans.input_range;  
  int sign = (tolerant == true ? 1 : -1);
  
  if (x > multigrapher->x_trans.input_max + sign * xfuzz)
    code |= RIGHT;
  else if (x < multigrapher->x_trans.input_min - sign * xfuzz)
    code |= LEFT;
  if (y > multigrapher->y_trans.input_max + sign * yfuzz)
    code |= TOP;
  else if (y < multigrapher->y_trans.input_min - sign * yfuzz)
    code |= BOTTOM;
  
  return code;
}

static void
transpose_portmanteau (int *val)
{
  bool xtrue, ytrue;
  int newval;
  
  xtrue = ((*val & X_AXIS) ? true : false);
  ytrue = ((*val & Y_AXIS) ? true : false);
  
  newval = (xtrue ? Y_AXIS : 0) | (ytrue ? X_AXIS : 0);
  *val = newval;
}

static void 
plot_errorbar (Multigrapher *multigrapher, const Point *p)
{
  if (p->have_x_errorbar || p->have_y_errorbar)
    /* save & restore state, since we invoke pl_linemod_r() */
    {
      pl_savestate_r (multigrapher->plotter);
      pl_linemod_r (multigrapher->plotter, "solid");
	
      if (p->have_x_errorbar)
	{
	  pl_fline_r (multigrapher->plotter, 
		      XV(p->xmin), YV(p->y) - 0.5 * SS(p->symbol_size),
		      XV(p->xmin), YV(p->y) + 0.5 * SS(p->symbol_size));
	  pl_fline_r (multigrapher->plotter, 
		      XV(p->xmin), YV(p->y), XV(p->xmax), YV(p->y));
	  pl_fline_r (multigrapher->plotter, 
		      XV(p->xmax), YV(p->y) - 0.5 * SS(p->symbol_size),
		      XV(p->xmax), YV(p->y) + 0.5 * SS(p->symbol_size));
	}
      if (p->have_y_errorbar)
	{
	  pl_fline_r (multigrapher->plotter, 
		      XV(p->x) - 0.5 * SS(p->symbol_size), YV(p->ymin),
		      XV(p->x) + 0.5 * SS(p->symbol_size), YV(p->ymin));
	  pl_fline_r (multigrapher->plotter, 
		      XV(p->x), YV(p->ymin), XV(p->x), YV(p->ymax));
	  pl_fline_r (multigrapher->plotter, 
		      XV(p->x) - 0.5 * SS(p->symbol_size), YV(p->ymax),
		      XV(p->x) + 0.5 * SS(p->symbol_size), YV(p->ymax));
	}

      pl_restorestate_r (multigrapher->plotter);
    }
}

/* An alternative means of ending a polyline in progress.  Rather than
   ending it by passing plot_point() a point with the `pendown' flag not
   set, one may call this function.  This yields faster response in
   real-time work; e.g. in reader.c it is called by read_and_plot_file()
   after all dataset(s) have been read from the file and plotted. */

void
end_polyline_and_flush (Multigrapher *multigrapher)
{
  pl_endpath_r (multigrapher->plotter);
  pl_flushpl_r (multigrapher->plotter);
  multigrapher->first_point_of_polyline = true;
}
