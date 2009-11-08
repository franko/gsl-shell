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

/* This file contains miscellaneous subroutines for GNU graph.  Currently,
   it contains only array_bounds(), which is called if the user fails to
   specify at least one of the bounds xmin,xmax,ymin,ymax.  

   array_bounds() returns the unspecified bounds via pointers.  I.e., it
   finishes the job of specifying a bounding box for the data points that
   will be plotted.  The box may later be expanded so that its bounds are
   multiples of the tick spacing (see plotter.c).

   array_bounds() is called in graph.c, just before a graph is begun. */

#include "sys-defines.h"
#include "extern.h"

/* bit fields for return value from Cohen-Sutherland clipper */
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

/* for internal clipper use */
enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };

/* forward references */
static int clip_line (double *x0_p, double *y0_p, double *x1_p, double *y1_p, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip, bool spec_min_x, bool spec_min_y, bool spec_max_x, bool spec_max_y);
static int compute_relevant_points (double xx, double yy, double oldxx, double oldyy, int clip_mode, double user_min_x, double user_min_y, double user_max_x, double user_max_y, bool spec_min_x, bool spec_min_y, bool spec_max_x, bool spec_max_y, double xxr[2], double yyr[2]);
static int compute_outcode (double x, double y, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip, bool spec_min_x, bool spec_min_y, bool spec_max_x, bool spec_max_y);

void
array_bounds (const Point *p, int length, 
	      bool transpose_axes, int clip_mode,
	      double *min_x, double *min_y, double *max_x, double *max_y,
	      bool spec_min_x, bool spec_min_y, 
	      bool spec_max_x, bool spec_max_y)
{
  /* keep compilers happy */
  double user_min_x = 0.0, user_min_y = 0.0;
  double user_max_x = 0.0, user_max_y = 0.0;
  double local_min_x = 0.0, local_min_y = 0.0; 
  double local_max_x = 0.0, local_max_y = 0.0;
  double xx, yy, oldxx, oldyy;
  bool point_seen = false;
  int i;

  if (length == 0)
    /* adopt a convention */
    {
      if (!spec_min_x)
	*min_x = 0.0;
      if (!spec_min_y)
	*min_y = 0.0;
      if (!spec_max_x)
	*max_x = *min_x;
      if (!spec_max_y)
	*max_y = *min_y;
      return;
    }

  if (spec_min_x)
    user_min_x = *min_x;
  else				/* won't use user_min_x */
    local_min_x = DBL_MAX;
  if (spec_max_x)
    user_max_x = *max_x;
  else				/* won't use user_max_x */
    local_max_x = -(DBL_MAX);
  
  /* special case: user specified both bounds, but min > max (reversed axis) */
  if (spec_min_x && spec_max_x && user_min_x > user_max_x)
    {
      double tmp;
      
      tmp = user_min_x;
      user_min_x = user_max_x;
      user_max_x = tmp;
    }

  if (spec_min_y)
    user_min_y = *min_y;
  else
    local_min_y = DBL_MAX;	/* won't use user_min_y */
  if (spec_max_y)
    user_max_y = *max_y;      
  else				/* won't use user_max_y */
    local_max_y = -(DBL_MAX);
    
  /* special case: user specified both bounds, but min > max (reversed axis) */
  if (spec_min_y && spec_max_y && user_min_y > user_max_y)
    {
      double tmp;
      
      tmp = user_min_y;
      user_min_y = user_max_y;
      user_max_y = tmp;
    }

  /* loop through points in array; examine each line segment */

  oldxx = oldyy = 0.0;		/* previous point */
  for (i = 0; i < length; i++)
    {
      double xxr[2], yyr[2];	/* storage for `relevant points' */
      int n, j;
      int effective_clip_mode;
      
      /* get new point */
      xx = (transpose_axes ? p[i].y : p[i].x);
      yy = (transpose_axes ? p[i].x : p[i].y);

      /* determine clipping mode (see compute_relevant_points() below) */
      if (i == 0 || p[i].pendown == false
	  || (p[i].linemode <= 0 && p[i].fill_fraction < 0.0))
	/* no polyline or filling, each point is isolated */
	effective_clip_mode = 0;
      else if (p[i].fill_fraction >= 0.0)
	effective_clip_mode = 2;
      else
	effective_clip_mode = clip_mode;

      n = compute_relevant_points (xx, yy, oldxx, oldyy,
				   effective_clip_mode,
				   user_min_x, user_min_y,
				   user_max_x, user_max_y,
				   spec_min_x, spec_min_y,
				   spec_max_x, spec_max_y,
				   xxr, yyr);
      /* loop through relevant points, updating bounding box */
      for (j = 0; j < n; j++)
	{
	  point_seen = true;
	  if (!spec_min_x)
	    local_min_x = DMIN(local_min_x, xxr[j]);
	  if (!spec_min_y)
	    local_min_y = DMIN(local_min_y, yyr[j]);
	  if (!spec_max_x)
	    local_max_x = DMAX(local_max_x, xxr[j]);
	  if (!spec_max_y)
	    local_max_y = DMAX(local_max_y, yyr[j]);
	}
      oldxx = xx;
      oldyy = yy;
    }
  
  if (!point_seen)
    /* a convention */
    local_min_x = local_min_y = local_max_x = local_max_y = 0.0;

  /* pass back bounds that user didn't specify */
  if (!spec_min_x)
    *min_x = local_min_x;
  if (!spec_min_y)
    *min_y = local_min_y;
  if (!spec_max_x)
    *max_x = local_max_x;
  if (!spec_max_y)
    *max_y = local_max_y;

  return;
}

/* For a new data point (xx,yy), compute the `relevant points', i.e. the
   ones that should be used in updating the bounding box.  There may be 0,
   1, or 2 of them.  The number of relevant points is returned, and the
   relevant points themselves are returned via pointers.

   The relevant points are computed from the line segment extending from
   (oldxx,oldyy) to (xx,yy), via an algorithm parametrized by a
   gnuplot-style clip mode (0, 1, or 2).

   If clip mode=0 then the simplest algorithm is used: (xx,yy) is a
   relevant point iff it satisfies the user-specified bound(s), and there
   are no other relevant points, i.e., (oldxx,oldyy) is ignored.  

   If clip mode=1 then if the line segment from (oldxx, oldyy) from (xx,yy)
   has at least one endpoint that satisfies the user-specified bounds, it
   generates two relevant points: the endpoints of the line segment,
   clipped to the bounds.  If on the other hand neither endpoint of the
   line segment from (oldxx,oldyy) to (xx,yy) satisfies the user-specified
   bounds, no relevant points are generated even if the line segment
   contains points that satisfy the bounds.

   If clip mode=2 then the line segment, if it intersects the bounding box,
   is clipped on both ends, and both resulting endpoints are relevant. */

static int
compute_relevant_points (double xx, double yy,
			 double oldxx, double oldyy,
			 int clip_mode,
			 double user_min_x, double user_min_y, 
			 double user_max_x, double user_max_y,
			 bool spec_min_x, bool spec_min_y, 
			 bool spec_max_x, bool spec_max_y,
			 double xxr[2], double yyr[2])
{
  int clipval;

  switch (clip_mode)
    {
    case 0:
      if ((!spec_min_x || xx >= user_min_x)
	  && (!spec_max_x || xx <= user_max_x)
	  && (!spec_min_y || yy >= user_min_y)
	  && (!spec_max_y || yy <= user_max_y))
	{
	  xxr[0] = xx;
	  yyr[0] = yy;
	  return 1;
	}
      else
	return 0;
      break;
    case 1:
    default:
      clipval = clip_line (&oldxx, &oldyy, &xx, &yy, user_min_x, user_max_x, user_min_y, user_max_y, spec_min_x, spec_min_y, spec_max_x, spec_max_y);
      if ((clipval & ACCEPTED) 
	  && !((clipval & CLIPPED_FIRST) && (clipval & CLIPPED_SECOND)))
	{
	  xxr[0] = oldxx;
	  yyr[0] = oldyy;
	  xxr[1] = xx;
	  yyr[1] = yy;
	  return 2;
	}
      else
	return 0;
      break;
    case 2:
      clipval = clip_line (&oldxx, &oldyy, &xx, &yy, user_min_x, user_max_x, user_min_y, user_max_y, spec_min_x, spec_min_y, spec_max_x, spec_max_y);
      if (clipval & ACCEPTED)
	{
	  xxr[0] = oldxx;
	  yyr[0] = oldyy;
	  xxr[1] = xx;
	  yyr[1] = yy;
	  return 2;
	}
      else
	return 0;
      break;
    }
}

/* clip_line() takes two points, the endpoints of a line segment in the
 * device frame (expressed in terms of floating-point device coordinates),
 * and destructively passes back two points: the endpoints of the line
 * segment clipped by Cohen-Sutherland to the rectangular clipping area.
 * The return value contains bitfields ACCEPTED, CLIPPED_FIRST, and
 * CLIPPED_SECOND.
 *
 * This is a modified C-S clipper: the flags spec_{min,max}_{x,y} indicate
 * whether or not clipping is to be performed on each edge. 
 */

static int
clip_line (double *x0_p, double *y0_p, double *x1_p, double *y1_p, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip, bool spec_min_x, bool spec_min_y, bool spec_max_x, bool spec_max_y)
{
  double x0 = *x0_p;
  double y0 = *y0_p;
  double x1 = *x1_p;
  double y1 = *y1_p;
  int outcode0, outcode1;
  bool accepted;
  int clipval = 0;
  
  outcode0 = compute_outcode (x0, y0, x_min_clip, x_max_clip, y_min_clip, y_max_clip, spec_min_x, spec_min_y, spec_max_x, spec_max_y);
  outcode1 = compute_outcode (x1, y1, x_min_clip, x_max_clip, y_min_clip, y_max_clip, spec_min_x, spec_min_y, spec_max_x, spec_max_y);  

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
	  int outcode_out = (outcode0 ? outcode0 : outcode1);
	  double x, y;		/* intersection with clip edge */
	  
	  if (outcode_out & RIGHT)
	    {
	      x = x_max_clip;
	      y = y0 + (y1 - y0) * (x_max_clip - x0) / (x1 - x0);
	    }
	  else if (outcode_out & LEFT)
	    {
	      x = x_min_clip;
	      y = y0 + (y1 - y0) * (x_min_clip - x0) / (x1 - x0);
	    }
	  else if (outcode_out & TOP)
	    {
	      x = x0 + (x1 - x0) * (y_max_clip - y0) / (y1 - y0);
	      y = y_max_clip;
	    }
	  else			/* BOTTOM bit must be set */
	    {
	      x = x0 + (x1 - x0) * (y_min_clip - y0) / (y1 - y0);
	      y = y_min_clip;
	    }
	  
	  if (outcode_out == outcode0)
	    {
	      x0 = x;
	      y0 = y;
	      outcode0 = compute_outcode (x0, y0, x_min_clip, x_max_clip, y_min_clip, y_max_clip, spec_min_x, spec_min_y, spec_max_x, spec_max_y);
	    }
	  else
	    {
	      x1 = x; 
	      y1 = y;
	      outcode1 = compute_outcode (x1, y1, x_min_clip, x_max_clip, y_min_clip, y_max_clip, spec_min_x, spec_min_y, spec_max_x, spec_max_y);
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

static int
compute_outcode (double x, double y, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip, bool spec_min_x, bool spec_min_y, bool spec_max_x, bool spec_max_y)
{
  int code = 0;

  if (spec_max_x && x > x_max_clip)
    code |= RIGHT;
  else if (spec_min_x && x < x_min_clip)
    code |= LEFT;
  if (spec_max_y && y > y_max_clip)
    code |= TOP;
  else if (spec_min_y && y < y_min_clip)
    code |= BOTTOM;
  
  return code;
}
