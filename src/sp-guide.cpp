#define __SP_GUIDE_C__

/*
 * Inkscape guideline implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *
 * Copyright (C) 2000-2002 authors
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include "helper/guideline.h"
#include "svg/svg.h"
#include "attributes.h"
#include "sp-guide.h"
#include <libnr/nr-values.h>
#include "helper/sp-intl.h"

enum {
	PROP_0,
	PROP_COLOR,
	PROP_HICOLOR
};

static void sp_guide_class_init (SPGuideClass * klass);
static void sp_guide_init (SPGuide * guide);
/*  static void sp_guide_finalize (GObject * object); */
static void sp_guide_set_property (GObject * object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void sp_guide_get_property (GObject * object, guint prop_id, GValue *value, GParamSpec *pspec);

static void sp_guide_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_guide_release (SPObject *object);
static void sp_guide_set (SPObject *object, unsigned int key, const gchar *value);

static SPObjectClass * parent_class;

GType
sp_guide_get_type (void)
{
	static GType guide_type = 0;
	if (!guide_type) {
		GTypeInfo guide_info = {
			sizeof (SPGuideClass),
			NULL, NULL,
			(GClassInitFunc) sp_guide_class_init,
			NULL, NULL,
			sizeof (SPGuide),
			16,
			(GInstanceInitFunc) sp_guide_init,
			NULL,	/* value_table */
		};
		guide_type = g_type_register_static (SP_TYPE_OBJECT, "SPGuide", &guide_info, (GTypeFlags)0);
	}
	return guide_type;
}

static void
sp_guide_class_init (SPGuideClass * klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	parent_class = (SPObjectClass*)g_type_class_ref (SP_TYPE_OBJECT);

	gobject_class->set_property = sp_guide_set_property;
	gobject_class->get_property = sp_guide_get_property;

	sp_object_class->build = sp_guide_build;
	sp_object_class->release = sp_guide_release;
	sp_object_class->set = sp_guide_set;

	g_object_class_install_property (gobject_class,
					 PROP_COLOR,
					 g_param_spec_uint ("color", "Color", "Color",
							    0,
							    0xffffffff,
							    0xff000000,
							    (GParamFlags)G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
					 PROP_HICOLOR,
					 g_param_spec_uint ("hicolor", "HiColor", "HiColor",
							    0,
							    0xffffffff,
							    0xff000000,
							    (GParamFlags)G_PARAM_READWRITE));
}

static void
sp_guide_init (SPGuide * guide)
{
	guide->normal = component_vectors[NR::Y];
	/* constrain y coordinate; horizontal line.  I doubt it ever matters what we initialize
	   this to. */
	guide->position = 0.0;
	guide->color = 0x0000ff7f;
	guide->hicolor = 0xff00007f;
}

static void
sp_guide_set_property (GObject * object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	SPGuide * guide;
	GSList * l;

	guide = SP_GUIDE (object);

	switch (prop_id) {
	case PROP_COLOR:
		guide->color = g_value_get_uint (value);
		for (l = guide->views; l != NULL; l = l->next) {
			sp_guideline_set_color (SP_GUIDELINE (l->data), guide->color);
		}
		break;
	case PROP_HICOLOR:
		guide->hicolor = g_value_get_uint (value);
		break;
	}
}

static void
sp_guide_get_property (GObject * object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	SPGuide * guide;

	guide = SP_GUIDE (object);

	switch (prop_id) {
	case PROP_COLOR:
		g_value_set_uint(value, guide->color);
		break;
	case PROP_HICOLOR:
		g_value_set_uint(value, guide->hicolor);
		break;
	}
}

static void
sp_guide_build (SPObject * object, SPDocument * document, SPRepr * repr)
{
	if (((SPObjectClass *) (parent_class))->build)
		(* ((SPObjectClass *) (parent_class))->build) (object, document, repr);

	sp_object_read_attr (object, "orientation");
	sp_object_read_attr (object, "position");

}

static void
sp_guide_release (SPObject *object)
{
	SPGuide *guide;

	guide = (SPGuide *) object;

	while (guide->views) {
		gtk_object_destroy (GTK_OBJECT (guide->views->data));
		guide->views = g_slist_remove (guide->views, guide->views->data);
	}

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

static void
sp_guide_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPGuide *guide;

	guide = SP_GUIDE (object);

	switch (key) {
	case SP_ATTR_ORIENTATION:
		if (value && !strcmp (value, "horizontal")) {
			/* Visual representation of a horizontal line, constrain vertically (y coordinate). */
			guide->normal = component_vectors[NR::Y];
		} else {
			guide->normal = component_vectors[NR::X];
		}
		break;
	case SP_ATTR_POSITION:
		sp_svg_number_read_d (value, &guide->position);
		break;
	default:
		if (((SPObjectClass *) (parent_class))->set)
			((SPObjectClass *) (parent_class))->set (object, key, value);
		break;
	}
}

void
sp_guide_show (SPGuide * guide, SPCanvasGroup * group, GCallback handler)
{
	SPCanvasItem *item;

	bool const vertical_line_p = ( guide->normal == component_vectors[NR::X] );
	g_assert( ( guide->normal == component_vectors[NR::X] )  ||
		  ( guide->normal == component_vectors[NR::Y] ) );
	item = sp_guideline_new (group, guide->position, vertical_line_p);
	sp_guideline_set_color (SP_GUIDELINE (item), guide->color);

	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (handler), guide);

	guide->views = g_slist_prepend (guide->views, item);
}

void
sp_guide_hide (SPGuide * guide, SPCanvas * canvas)
{
	GSList * l;

	g_assert (guide != NULL);
	g_assert (SP_IS_GUIDE (guide));
	g_assert (canvas != NULL);
	g_assert (SP_IS_CANVAS (canvas));

	for (l = guide->views; l != NULL; l = l->next) {
		if (canvas == SP_CANVAS_ITEM (l->data)->canvas) {
			gtk_object_destroy (GTK_OBJECT (l->data));
			guide->views = g_slist_remove (guide->views, l->data);
			return;
		}
	}
	g_assert_not_reached ();
}

void
sp_guide_sensitize (SPGuide * guide, SPCanvas * canvas, gboolean sensitive)
{
	GSList * l;

	g_assert (guide != NULL);
	g_assert (SP_IS_GUIDE (guide));
	g_assert (canvas != NULL);
	g_assert (SP_IS_CANVAS (canvas));

	for (l = guide->views; l != NULL; l = l->next) {
		if (canvas == SP_CANVAS_ITEM (l->data)->canvas) {
			sp_guideline_set_sensitive (SP_GUIDELINE (l->data), sensitive);
			return;
		}
	}
	g_assert_not_reached ();
}

double
sp_guide_position_from_pt (SPGuide const *guide, NR::Point const &pt)
{
	return dot(guide->normal, pt);
}

/** Temporary moveto in response to motion event while dragging (desktop-events.cpp). */
void sp_guide_moveto(SPGuide const *guide, gdouble position)
{
	g_assert (guide != NULL);
	g_assert (SP_IS_GUIDE (guide));

	for (GSList *l = guide->views; l != NULL; l = l->next) {
		sp_guideline_set_position(SP_GUIDELINE(l->data),
					  position);
	}
}

/** Permanent (committing) version of sp_guide_moveto, in response to button release event after
    dragging a guideline, or clicking OK in guide editing dialog (both in desktop-events.cpp). */
void sp_guide_position_set(SPGuide *guide, double position)
{
	g_assert (SP_IS_GUIDE (guide));

	sp_guide_moveto(guide, position);
	sp_repr_set_double(SP_OBJECT(guide)->repr,
				     "position", position);
}

/**
 * Returns a human-readable description of the guideline for use in dialog boxes and status bar.
 *
 * The caller is responsible for freeing the string.
 */
char *
sp_guide_description(SPGuide const *guide)
{
	using NR::X;
	using NR::Y;

	if ( guide->normal == component_vectors[X] ) {
		return g_strdup(_("vertical guideline"));
	} else if ( guide->normal == component_vectors[Y] ) {
		return g_strdup(_("horizontal guideline"));
	} else {
		double const radians = atan2(guide->normal[X],
					     guide->normal[Y]);
		/* flip y axis and rotate 90 degrees to convert to line angle */
		double const degrees = ( radians / M_PI ) * 180.0;
		int const degrees_int = (int) floor( degrees + .5 );
		return g_strdup_printf("%d degree guideline", degrees_int);
		/* Alternative suggestion: "angled guideline". */
	}
}

void
sp_guide_remove (SPGuide * guide)
{
	g_assert (SP_IS_GUIDE (guide));

	sp_repr_unparent (SP_OBJECT (guide)->repr);
}

