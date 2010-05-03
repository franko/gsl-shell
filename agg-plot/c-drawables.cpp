
#include "agg_color_rgba.h"
#include "agg_math_stroke.h"

#include "units-plot.h"
#include "plot.h"
#include "trans.h"
#include "vertex-source.h"
#include "lua-plot-priv.h"
#include "c-drawables.h"

struct property_reg line_cap_properties[] = {
  {(int) agg::butt_cap,   "butt"  },
  {(int) agg::square_cap, "square"},
  {(int) agg::round_cap,  "round" },
  {0, NULL}
};

struct property_reg line_join_properties[] = {
  {(int) agg::miter_join,        "miter"      },
  {(int) agg::miter_join_revert, "miter.rev"  },
  {(int) agg::round_join,        "round"      },
  {(int) agg::bevel_join,        "bevel"      },
  {(int) agg::miter_join_round,  "miter.round"},
  {0, NULL}
};

typedef my::path path_type;

CPLOT *
plot_new(int with_units)
{
  typedef units_plot<vertex_source, ref_manager> units_plot_type;
  plot_type *p;

  if (with_units)
    p = new units_plot_type();
  else
    p = new plot_type();

#ifdef DEBUG_PLOT
  fprintf(stderr, "Creating plot: %p\n", p);
#endif

  return (CPLOT *) p; 
}

void plot_free (CPLOT* _d)
{
  plot_type* p = (plot_type*) _d;
  delete p;
}

static vertex_source*
build_pipeline (vertex_source* in, struct trans_spec *base)
{
  struct trans_spec *spec;
  vertex_source* curr;
  size_t n;

  for (spec = base, n = 0; spec->tag != trans_end; spec++, n++) {}
  
  for (spec = base + (n-1); spec >= base; spec--)
    {
      switch (spec->tag)
	{
	  double *len;
	  trans::dash* ds;
	  trans::stroke* stroke;
	  trans::affine* trans;
	case trans_stroke:
	  stroke = new trans::stroke(in, spec->content.stroke.width);
	  stroke->line_cap((agg::line_cap_e) spec->content.stroke.line_cap);
	  stroke->line_join((agg::line_join_e) spec->content.stroke.line_join);
	  curr = stroke;
	  break;
	case trans_marker:
	  curr = new trans::marker(in, spec->content.marker.size);
	  break;
	case trans_curve:
	  curr = new trans::curve(in);
	  break;
	case trans_dash:
	  ds = new trans::dash(in);
	  len = spec->content.dash.len;
	  ds->add_dash(len[0], len[1]);
	  curr = ds;
	  break;
	case trans_rotate:
	  trans = new trans::affine(in);
	  trans->rotate(spec->content.rotate.angle);
	  curr = trans;
	  break;
	case trans_translate:
	  trans = new trans::affine(in);
	  trans->translate(spec->content.translate.x, spec->content.translate.y);
	  curr = trans;
	  break;
	case trans_end:
	  ;
	}

      in = curr;
    }
  
  return in;
}

static agg::rgba8 new_color(struct color *c)
{
  return agg::rgba8(c->r, c->g, c->b, c->a);
}

void plot_add(CPLOT *_p, CVERTSRC *_vs, struct color *color,
	      struct trans_spec *post, struct trans_spec *pre,
	      int outline)
{
  plot_type* p = (plot_type*) _p;
  vertex_source* vs = (vertex_source*) _vs;
  vertex_source* curr;

  curr = build_pipeline (vs, pre);

  if (curr->need_resize())
    curr = new trans::resize(curr);

  curr = build_pipeline (curr, post);

  p->add(curr, new_color(color), (bool) outline);
}

CPATH* path_new()
{
  path_type* p = new path_type();
  return (CPATH *) p;
}

void vertex_source_ref (CVERTSRC *_p)
{
  vertex_source* p = (vertex_source*) _p;
  p->ref();
}

void vertex_source_unref (CVERTSRC* _p)
{
  vertex_source* p = (vertex_source*) _p;
  unsigned rc = p->unref();
  if (rc == 0)
    delete p;
}

void
path_cmd (CPATH *_p, int _cmd, struct cmd_call_stack *s)
{
  path_type* p = (path_type*) _p;
  agg::path_storage& ps = p->get_path();
  path_cmd_e cmd = (path_cmd_e) _cmd;

  switch (cmd)
    {
    case CMD_MOVE_TO:
      ps.move_to (s->f[0], s->f[1]);
      break;
    case CMD_LINE_TO:
      ps.line_to (s->f[0], s->f[1]);
      break;
    case CMD_CLOSE:
      ps.close_polygon ();
      break;
    case CMD_ARC_TO:
      ps.arc_to (s->f[0], s->f[1], s->f[2], s->b[0], s->b[1], s->f[3], s->f[4]);
      break;
    case CMD_CURVE3:
      ps.curve3 (s->f[0], s->f[1], s->f[2], s->f[3]);
      break;
    case CMD_CURVE4:
      ps.curve4 (s->f[0], s->f[1], s->f[2], s->f[3], s->f[4], s->f[5]);
      break;
    case CMD_ERROR:
      ;
    }
}

CTEXT* text_new (double size, double width)
{
  my::text *t = new my::text(size, width);
  return (CTEXT *) t;
}

void text_set_text (CTEXT *_t, const char *text)
{
  my::text *t = (my::text *) _t;
  t->set_text(text);
}

void text_set_point (CTEXT *_t, double x, double y)
{
  my::text *t = (my::text *) _t;
  t->start_point (x, y);
}

void text_rotate (CTEXT *_t, double angle)
{
  my::text *t = (my::text *) _t;
  t->rotate(angle);
}
