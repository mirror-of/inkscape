#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <gnome.h>
#include <libgdome/gdome.h>
#include "libgnomeprint/gnome-canvas-bpath-util.h"
#include "svg-util.h"
#include "svg-path.h"

/* This module parses an SVG path element into a GnomeCanvasBpathDef.

   At present, there is no support for <marker> or any other contextual
   information from the SVG file. The API will need to change rather
   significantly to support these.

   Reference: SVG working draft 12 April 1999, section 11.
*/

typedef struct _SVGParsePathCtx SVGParsePathCtx;

struct _SVGParsePathCtx {
  GnomeCanvasBpathDef *bpath;
  double cpx, cpy;  /* current point */
  double rpx, rpy;  /* reflection point (for 's' command) */
  char cmd;         /* current command (lowercase) */
  int param;        /* parameter number */
  gboolean rel;     /* true if relative coords */
  double params[7]; /* parameters that have been parsed */
};

/* supply defaults for missing parameters, assuming relative coordinates
   are to be interpreted as x,y */
static void
svg_parse_path_default_xy (SVGParsePathCtx *ctx, int n_params)
{
  int i;

  if (ctx->rel)
    {
      for (i = ctx->param; i < n_params; i++)
	{
	  if (i > 2)
	    ctx->params[i] = ctx->params[i - 2];
	  else if (i == 1)
	    ctx->params[i] = ctx->cpy;
	  else if (i == 0)
	    /* we shouldn't get here (usually ctx->param > 0 as
               precondition) */
	    ctx->params[i] = ctx->cpx;
	}
    }
  else
    {
      for (i = ctx->param; i < n_params; i++)
	ctx->params[i] = 0.0;
    }
}

static void
svg_parse_path_do_cmd (SVGParsePathCtx *ctx, gboolean final)
{
  double x1, y1, x2, y2, x3, y3;

#ifdef VERBOSE
  int i;

  g_print ("parse_path %c:", ctx->cmd);
  for (i = 0; i < ctx->param; i++)
    g_print (" %f", ctx->params[i]);
  g_print (final ? ".\n" : "\n");
#endif

  switch (ctx->cmd)
    {
    case 'm':
      /* moveto */
      if (ctx->param == 2 || final)
	{
	  svg_parse_path_default_xy (ctx, 2);
#ifdef VERBOSE
	  g_print ("'m' moveto %g,%g\n",
		   ctx->params[0], ctx->params[1]);
#endif
	  gnome_canvas_bpath_def_moveto (ctx->bpath,
					 ctx->params[0], ctx->params[1]);
	  ctx->cpx = ctx->rpx = ctx->params[0];
	  ctx->cpy = ctx->rpy = ctx->params[1];
	  ctx->param = 0;
	}
      break;
    case 'l':
      /* lineto */
      if (ctx->param == 2 || final)
	{
	  svg_parse_path_default_xy (ctx, 2);
#ifdef VERBOSE
	  g_print ("'l' lineto %g,%g\n",
		   ctx->params[0], ctx->params[1]);
#endif
	  gnome_canvas_bpath_def_lineto (ctx->bpath,
					 ctx->params[0], ctx->params[1]);
	  ctx->cpx = ctx->rpx = ctx->params[0];
	  ctx->cpy = ctx->rpy = ctx->params[1];
	  ctx->param = 0;
	}
      break;
    case 'c':
      /* curveto */
      if (ctx->param == 6 || final)
	{
	  svg_parse_path_default_xy (ctx, 6);
	  x1 = ctx->params[0];
	  y1 = ctx->params[1];
	  x2 = ctx->params[2];
	  y2 = ctx->params[3];
	  x3 = ctx->params[4];
	  y3 = ctx->params[5];
#ifdef VERBOSE
	  g_print ("'c' curveto %g,%g %g,%g, %g,%g\n",
		   x1, y1, x2, y2, x3, y3);
#endif
	  gnome_canvas_bpath_def_curveto (ctx->bpath,
					  x1, y1, x2, y2, x3, y3);
	  ctx->rpx = x2;
	  ctx->rpy = y2;
	  ctx->cpx = x3;
	  ctx->cpy = y3;
	  ctx->param = 0;
	}
      break;
    case 's':
      /* smooth curveto */
      if (ctx->param == 4 || final)
	{
	  svg_parse_path_default_xy (ctx, 4);
	  x1 = 2 * ctx->cpx - ctx->rpx;
	  y1 = 2 * ctx->cpy - ctx->rpy;
	  x2 = ctx->params[0];
	  y2 = ctx->params[1];
	  x3 = ctx->params[2];
	  y3 = ctx->params[3];
#ifdef VERBOSE
	  g_print ("'s' curveto %g,%g %g,%g, %g,%g\n",
		   x1, y1, x2, y2, x3, y3);
#endif
	  gnome_canvas_bpath_def_curveto (ctx->bpath,
					  x1, y1, x2, y2, x3, y3);
	  ctx->rpx = x2;
	  ctx->rpy = y2;
	  ctx->cpx = x3;
	  ctx->cpy = y3;
	  ctx->param = 0;
	}
      break;
    case 'h':
      /* horizontal lineto */
      if (ctx->param == 1)
	{
#ifdef VERBOSE
	  g_print ("'h' lineto %g,%g\n",
		   ctx->params[0], ctx->cpy);
#endif
	  gnome_canvas_bpath_def_lineto (ctx->bpath,
					 ctx->params[0], ctx->cpy);
	  ctx->cpx = ctx->rpx = ctx->params[0];
	  ctx->param = 0;
	}
      break;
    case 'v':
      /* vertical lineto */
      if (ctx->param == 1)
	{
#ifdef VERBOSE
	  g_print ("'v' lineto %g,%g\n",
		   ctx->cpx, ctx->params[0]);
#endif
	  gnome_canvas_bpath_def_lineto (ctx->bpath,
					 ctx->cpx, ctx->params[0]);
	  ctx->cpy = ctx->rpy = ctx->params[0];
	  ctx->param = 0;
	}
      break;
    case 'q':
      /* quadratic bezier curveto */

      /* non-normative reference:
	 http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/beziers.html
      */
      if (ctx->param == 4 || final)
	{
	  svg_parse_path_default_xy (ctx, 4);
	  /* raise quadratic bezier to cubic */
	  x1 = (ctx->cpx + 2 * ctx->params[0]) * (1.0 / 3.0);
	  y1 = (ctx->cpy + 2 * ctx->params[1]) * (1.0 / 3.0);
	  x3 = ctx->params[2];
	  y3 = ctx->params[3];
	  x2 = (x3 + 2 * ctx->params[0]) * (1.0 / 3.0);
	  y2 = (y3 + 2 * ctx->params[1]) * (1.0 / 3.0);
#ifdef VERBOSE
	  g_print ("'q' curveto %g,%g %g,%g, %g,%g\n",
		   x1, y1, x2, y2, x3, y3);
#endif
	  gnome_canvas_bpath_def_curveto (ctx->bpath,
					  x1, y1, x2, y2, x3, y3);
	  ctx->rpx = x2;
	  ctx->rpy = y2;
	  ctx->cpx = x3;
	  ctx->cpy = y3;
	  ctx->param = 0;
	}
      break;
    case 't':
      /* Truetype quadratic bezier curveto */
      if (ctx->param == 5)
	{
	  /* generate a quadratic bezier with control point = params[0,1]
	     and anchor point = 0.5 (params[0,1] + params[2,3]) */
	  x1 = (ctx->cpx + 2 * ctx->params[0]) * (1.0 / 3.0);
	  y1 = (ctx->cpy + 2 * ctx->params[1]) * (1.0 / 3.0);
	  x3 = 0.5 * (ctx->params[0] + ctx->params[2]);
	  y3 = 0.5 * (ctx->params[1] + ctx->params[3]);
	  x2 = (x3 + 2 * ctx->params[0]) * (1.0 / 3.0);
	  y2 = (y3 + 2 * ctx->params[1]) * (1.0 / 3.0);
#ifdef VERBOSE
	  g_print ("'t' curveto %g,%g %g,%g, %g,%g\n",
		   x1, y1, x2, y2, x3, y3);
#endif
	  gnome_canvas_bpath_def_curveto (ctx->bpath,
					  x1, y1, x2, y2, x3, y3);
	  ctx->cpx = x3;
	  ctx->cpy = y3;
	  /* shift two parameters */
	  ctx->params[0] = ctx->params[2];
	  ctx->params[1] = ctx->params[3];
	  ctx->params[2] = ctx->params[4];
	  ctx->param = 3;
	}
      else if (final)
	{
	  if (ctx->param > 2)
	    {
	      svg_parse_path_default_xy (ctx, 4);
	      /* raise quadratic bezier to cubic */
	      x1 = (ctx->cpx + 2 * ctx->params[0]) * (1.0 / 3.0);
	      y1 = (ctx->cpy + 2 * ctx->params[1]) * (1.0 / 3.0);
	      x3 = ctx->params[2];
	      y3 = ctx->params[3];
	      x2 = (x3 + 2 * ctx->params[0]) * (1.0 / 3.0);
	      y2 = (y3 + 2 * ctx->params[1]) * (1.0 / 3.0);
#ifdef VERBOSE
	      g_print ("'t' curveto %g,%g %g,%g, %g,%g\n",
		       x1, y1, x2, y2, x3, y3);
#endif
	      gnome_canvas_bpath_def_curveto (ctx->bpath,
					      x1, y1, x2, y2, x3, y3);
	      ctx->rpx = x2;
	      ctx->rpy = y2;
	      ctx->cpx = x3;
	      ctx->cpy = y3;
	    }
	  else
	    {
	      svg_parse_path_default_xy (ctx, 2);
#ifdef VERBOSE
	      g_print ("'t' lineto %g,%g\n",
		       ctx->params[0], ctx->params[1]);
#endif
	      gnome_canvas_bpath_def_lineto (ctx->bpath,
					     ctx->params[0], ctx->params[1]);
	      ctx->cpx = ctx->rpx = ctx->params[0];
	      ctx->cpy = ctx->rpy = ctx->params[1];
	    }
	  ctx->param = 0;
	}
      break;
      /* todo: d, e, f, g, j */
    default:
      ctx->param = 0;
    }
}

static void
svg_parse_path_data (SVGParsePathCtx *ctx, const char *data)
{
  int i;
  double val = 0;
  char c;
  gboolean in_num, in_frac;
  int sign;
  double frac;

  in_num = FALSE;
  for (i = 0; ; i++)
    {
      c = data[i];
      if (c >= '0' && c <= '9')
	{
	  /* digit */
	  if (in_num)
	    {
	      if (in_frac)
		val += (frac *= 0.1) * (c - '0');
	      else
		val = (val * 10) + c - '0';
	    }
	  else
	    {
	      in_num = TRUE;
	      in_frac = FALSE;
	      val = c - '0';
	      sign = 1;
	    }
	}
      else if (c == '.')
	{
	  if (!in_num)
	    {
	      in_num = TRUE;
	      val = 0;
	    }
	  in_frac = TRUE;
	  frac = 1;
	}
      else if (in_num)
	{
	  /* end of number */

	  val *= sign;
	  if (ctx->rel)
	    {
	      /* Handle relative coordinates. This switch statement attempts
		 to determine _what_ the coords are relative to. This is
		 underspecified in the 12 Apr working draft. */
	      switch (ctx->cmd)
		{
		case 'l':
		case 'm':
		case 'c':
		case 's':
		case 'q':
		case 't':
#ifndef SVGV_RELATIVE
		  /* rule: even-numbered params are x-relative, odd-numbered
		     are y-relative */
		  if (ctx->param == 0)
		    val += ctx->cpx;
		  else if (ctx->param == 1)
		    val += ctx->cpy;
		  else
		    val += ctx->params[ctx->param - 2];
		  break;
#else
		  /* rule: even-numbered params are x-relative, odd-numbered
		     are y-relative */
		  if (ctx->param == 0 || (ctx->param % 2 ==0))
		    val += ctx->cpx;
		  else 
		    val += ctx->cpy;
		  break;
#endif
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'j':
		  /* rule: first two params are x and y, rest are not
		     relative */
		  if (ctx->param == 0)
		    val += ctx->cpx;
		  else if (ctx->param == 1)
		    val += ctx->cpy;
		  break;
		case 'h':
		  /* rule: x-relative */
		  val += ctx->cpx;
		  break;
		case 'v':
		  /* rule: y-relative */
		  val += ctx->cpy;
		  break;
		}
	    }
	  ctx->params[ctx->param++] = val;
	  svg_parse_path_do_cmd (ctx, FALSE);
	  in_num = FALSE;
	}

      if (c == '\0')
	break;
      else if (c == '-')
	{
	  sign = -1;
	  val = 0;
	  in_num = TRUE;
	  in_frac = FALSE;
	}
      else if (c == 'A')
	ctx->rel = FALSE;
      else if (c == 'r')
	ctx->rel = TRUE;
      else if (c == 'z')
	{
	  if (ctx->param)
	    svg_parse_path_do_cmd (ctx, TRUE);
#ifdef VERBOSE
	  g_print ("'z' closepath\n");
#endif
	  gnome_canvas_bpath_def_closepath (ctx->bpath);
	}
      else if (c >= 'A' && c <= 'Z')
	{
	  if (ctx->param)
	    svg_parse_path_do_cmd (ctx, TRUE);
	  ctx->cmd = c + 'a' - 'A';
	  ctx->rel = FALSE;
	}
      else if (c >= 'a' && c <= 'z')
	{
	  if (ctx->param)
	    svg_parse_path_do_cmd (ctx, TRUE);
	  ctx->cmd = c;
	  ctx->rel = TRUE;
	}
      /* else c _should_ be whitespace or , */
    }
}

svg_parse_path_child (SVGParsePathCtx *ctx, GdomeNode *node)
{
  GdomeDOMString *tagName;
  GdomeDOMString *data;
  GdomeException exc = 0;

  tagName = gdome_el_tagName ((GdomeElement *)node, &exc);

#ifdef VERBOSE
  printf ("tagName = %s\n", tagName->str);
#endif

  if (!strcasecmp (tagName->str, "data"))
    {
      data = gdome_el_getAttribute (node, svg_mk_const_gdome_str ("d"),
				    &exc);
      
      if (data)
	{
	  svg_parse_path_data (ctx, data->str);
	  gdome_str_unref (data);
	} 
    }
}

GnomeCanvasBpathDef *
svg_parse_path (GdomeNode *node)
{
  SVGParsePathCtx ctx;
  GdomeDOMString *data;
  GdomeException exc = 0;
  GdomeNode *child, *next;

  ctx.bpath = gnome_canvas_bpath_def_new ();
  ctx.cpx = 0.0;
  ctx.cpy = 0.0;
  ctx.cmd = 0;
  ctx.param = 0;

  data = gdome_el_getAttribute (node, svg_mk_const_gdome_str ("d"),
				&exc);

  svg_parse_path_data (&ctx, data->str);

#if 0 /* <data> children are gone from the spec */
  /* finalize last command */
  for (child = gdome_n_firstChild (node, &exc); child != NULL; child = next) {
	  svg_parse_path_child (&ctx, child);
	  next = gdome_n_nextSibling (child, &exc);
	  gdome_n_removeChild (node, (GdomeDOMNode *)child, &exc);
	  gdome_n_unref (child, &exc);
  }
#endif

  if (ctx.param)
    svg_parse_path_do_cmd (&ctx, TRUE);

  gdome_str_unref (data);
  /* todo: parse the path data of child <data> elements */

  return ctx.bpath;
}

