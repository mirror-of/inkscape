#define __SP_PATH_C__

/*
 * SVG <path> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

#include <libnr/nr-path.h>
#include <libnr/nr-values.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>

#include "helper/sp-intl.h"
#include "svg/svg.h"
#include "attributes.h"
#include "style.h"
#include "version.h"

#include "sp-path.h"

#define noPATH_VERBOSE

static void sp_path_class_init (SPPathClass *klass);
static void sp_path_init (SPPath *path);

static void sp_path_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_path_set (SPObject *object, unsigned int key, const gchar *value);

static SPRepr *sp_path_write (SPObject *object, SPRepr *repr, guint flags);
static NR::Matrix sp_path_set_transform (SPItem *item, NR::Matrix const &xform);
static gchar * sp_path_description (SPItem * item);

static void sp_path_update (SPObject *object, SPCtx *ctx, guint flags);

static SPShapeClass *parent_class;

/**
 * Gets the GType object for SPPathClass
 */
GType
sp_path_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (SPPathClass),
			NULL, NULL,
			(GClassInitFunc) sp_path_class_init,
			NULL, NULL,
			sizeof (SPPath),
			16,
			(GInstanceInitFunc) sp_path_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_SHAPE, "SPPath", &info, (GTypeFlags)0);
	}
	return type;
}

/** 
 *  Does the object-oriented work of initializing the class structure
 *  including parent class, and registers function pointers for
 *  the functions build, set, write, and set_transform.
 */
static void
sp_path_class_init (SPPathClass * klass)
{
	SPObjectClass *sp_object_class;
	SPItemClass *item_class;

	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;

	parent_class = (SPShapeClass *)g_type_class_peek_parent (klass);

	sp_object_class->build = sp_path_build;
	sp_object_class->set = sp_path_set;
	sp_object_class->write = sp_path_write;
	sp_object_class->update = sp_path_update;

	item_class->description = sp_path_description;
	item_class->set_transform = sp_path_set_transform;
}


gint 
sp_nodes_in_path (SPPath *path)
{
	SPCurve * curve;
	gint r, i; 
	curve = SP_SHAPE (path) -> curve;
	r = curve->end; 
	i = curve->length - 1;
	if (i > r) i = r; // sometimes after switching from node editor length is wrong, e.g. f6 - draw - f2 - tab - f1, this fixes it
	for (; i >= 0; i --) 
		if ((curve->bpath + i) -> code == NR_MOVETO)
			r --;
	return r;
}

static gchar *
sp_path_description (SPItem * item)
{
	return g_strdup_printf (_("Path (%i nodes)"), sp_nodes_in_path (SP_PATH (item)));
}

/**
 * Initializes an SPPath.  Currently does nothing.
 */
static void
sp_path_init (SPPath *path)
{
	/* Nothing here */
}

/**
 *  Given a repr, this sets the data items in the path object such as
 *  fill & style attributes, markers, and CSS properties.
 */
static void
sp_path_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	SPPath *path;
	SPVersion version;

	path = SP_PATH (object);

	version = sp_object_get_sodipodi_version (object);

	/* Fixes old Sodipodi nodetype to namespaced parameter */
	if (sp_version_inside_range (version, 0, 0, 0, 25)) {
		const gchar *str;
		str = sp_repr_attr (repr, "SODIPODI-PATH-NODE-TYPES");
		sp_repr_set_attr (repr, "sodipodi:nodetypes", str);
		sp_repr_set_attr (repr, "SODIPODI-PATH-NODE-TYPES", NULL);
	}

	sp_object_read_attr (object, "d");

	/* Are these calls actually necessary? */
	sp_object_read_attr (object, "marker");
	sp_object_read_attr (object, "marker-start");
	sp_object_read_attr (object, "marker-mid");
	sp_object_read_attr (object, "marker-end");

	if (sp_version_inside_range (version, 0, 0, 0, 25)) {
		SPShape *shape;
		SPCSSAttr *css;
		const gchar *val;
		gboolean changed;
		gboolean open;
		shape = (SPShape *) path;
		/* Remove fill from open paths for compatibility with inkscape < 0.25 */
		/* And set fill-rule of closed paths to evenodd */
		/* We force style rewrite at moment (Lauris) */
		changed = TRUE;
		open = FALSE;
		if (shape->curve && shape->curve->bpath) {
			NArtBpath *bp;
			for (bp = shape->curve->bpath; bp->code != NR_END; bp++) {
				if (bp->code == NR_MOVETO_OPEN) {
					open = TRUE;
					break;
				}
			}
		}
		css = sp_repr_css_attr (repr, "style");
		if (open) {
			val = sp_repr_css_property (css, "fill", NULL);
			if (val && strcmp (val, "none")) {
				sp_repr_css_set_property (css, "fill", "none");
				changed = TRUE;
			}
		} else {
			val = sp_repr_css_property (css, "fill-rule", NULL);
			if (!val) {
				sp_repr_css_set_property (css, "fill-rule", "evenodd");
				changed = TRUE;
			}
		}
		if (changed) {
			sp_repr_css_set (repr, css, "style");
		}
		sp_repr_css_attr_unref (css);
	}

	if (((SPObjectClass *) parent_class)->build) {
		((SPObjectClass *) parent_class)->build (object, document, repr);
	}
}

/**
 *  Sets a value in the path object given by 'key', to 'value'.  This is used
 *  for setting attributes and markers on a path object.
 */
static void
sp_path_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPPath *path;

	path = (SPPath *) object;

	switch (key) {
	case SP_ATTR_D:
		if (value) {
			NArtBpath *bpath;
			SPCurve *curve;
			bpath = sp_svg_read_path (value);
			curve = sp_curve_new_from_bpath (bpath);
			if (curve) {
			  sp_shape_set_curve ((SPShape *) path, curve, TRUE);
			  sp_curve_unref (curve);
			}
		} else {
			sp_shape_set_curve ((SPShape *) path, NULL, TRUE);
		}
		break;
	case SP_PROP_MARKER:
	case SP_PROP_MARKER_START:
	case SP_PROP_MARKER_MID:
	case SP_PROP_MARKER_END:
	      sp_shape_set_marker (object, key,  value);
	    break;
 	default:
		if (((SPObjectClass *) parent_class)->set) {
			((SPObjectClass *) parent_class)->set (object, key, value);
		}
		break;
	}
}

/**
 *
 * Writes the path object into a SPRepr
 */
static SPRepr *
sp_path_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPShape *shape;
	NArtBpath *abp;
	gchar *str;

	shape = (SPShape *) object;

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("path");
	}

	abp = sp_curve_first_bpath (shape->curve);
	str = sp_svg_write_path (abp);
	sp_repr_set_attr (repr, "d", str);
	g_free (str);

	if (((SPObjectClass *) (parent_class))->write) {
		((SPObjectClass *) (parent_class))->write (object, repr, flags);
	}

	return repr;
}

static void
sp_path_update (SPObject *object, SPCtx *ctx, guint flags)
{
	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // since we change the description, it's not a "just translation" anymore
	}

	if (((SPObjectClass *) parent_class)->update)
		((SPObjectClass *) parent_class)->update (object, ctx, flags);
}


/**
 * Writes the given transform into the repr for the given item.
 */
static NR::Matrix
sp_path_set_transform (SPItem *item, NR::Matrix const &xform)
{
	SPPath *path;
	SPShape *shape;
	SPCurve *curve;
	NRBPath dpath, spath;

	path = (SPPath *) item;
	shape = (SPShape *) item;

	/* Transform the path */
	spath.path = shape->curve->bpath;
	nr_path_duplicate_transform (&dpath, &spath, xform);
	curve = sp_curve_new_from_bpath (dpath.path);
	if (curve) {
		sp_shape_set_curve (shape, curve, TRUE);
		sp_curve_unref (curve);
	}

	// Adjust stroke
	sp_shape_adjust_stroke (item, NR::expansion(xform));

	// Adjust pattern fill
	sp_shape_adjust_pattern (item, xform);

	sp_object_request_update(SP_OBJECT(item), SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

	// nothing remains - we've written all of the transform, so return identity
	return NR::identity();
}

