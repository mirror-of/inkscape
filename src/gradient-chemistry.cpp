#define __SP_GRADIENT_CHEMISTRY_C__

/*
 * Various utility methods for gradients
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <ctype.h>

#include "xml/repr-private.h"
#include "style.h"
#include "document-private.h"
#include "sp-root.h"
#include "gradient-chemistry.h"

// Terminology:
// "vector" is a gradient that has stops but not position coords. It can be referenced by one or more privates. Objects should not refer to it directly. It has no radial/linear distinction.
// "private" is a gradient that has no stops but has position coords (e.g. center, radius etc for a radial). It references a vector for the actual colors. Each private is only used by one object. It is either linear or radial.

static void sp_gradient_repr_set_link (SPRepr *repr, SPGradient *gr);
static void sp_item_repr_set_style_gradient (SPRepr *repr, const gchar *property, SPGradient *gr);
static gchar *sp_style_change_property (const gchar *sstr, const gchar *key, const gchar *value);

/* fixme: One more step is needed - normalization vector to 0-1 (not sure 100% still) */

SPGradient *
sp_gradient_ensure_vector_normalized (SPGradient *gr)
{
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);

	/* If we are already normalized vector, just return */
	if (gr->state == SP_GRADIENT_STATE_VECTOR) return gr;
	/* Fail, if we have wrong state set */
	if (gr->state != SP_GRADIENT_STATE_UNKNOWN) {
		g_warning ("file %s: line %d: Cannot normalize private gradient to vector (%s)", __FILE__, __LINE__, SP_OBJECT_ID (gr));
		return NULL;
	}

	//g_print ("GVECTORNORM: Requested vector normalization of gradient %s\n", SP_OBJECT_ID (gr));

	SPDocument *doc = SP_OBJECT_DOCUMENT (gr);
	SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS (doc);

	if (SP_OBJECT_PARENT (gr) != SP_OBJECT (defs)) {
		SPGradient *spnew;
		SPRepr *repr;
		/* Lonely gradient */
		/* Ensure vector, so we can know some our metadata */
		sp_gradient_ensure_vector (gr);
		g_assert (gr->vector);
		/* NOTICE */
		/* We are in some lonely place in tree, so clone EVERYTHING */
		/* And do not forget to flatten original */
		g_print ("GVECTORNORM: Gradient %s IS NOT in <defs>\n", SP_OBJECT_ID (gr));
		/* Step 1 - flatten original EXCEPT vector */
		SP_OBJECT(gr)->updateRepr(((SPObject *) gr)->repr, SP_OBJECT_WRITE_EXT | SP_OBJECT_WRITE_ALL);
		g_print ("GVECTORNORM: Gradient %s attributes flattened\n", SP_OBJECT_ID (gr));
		/* Step 2 - create new empty gradient and prepend it to <defs> */
		repr = sp_repr_new ("linearGradient");
		//sp_repr_set_attr(repr, "inkscape:collect", "always");
		sp_repr_add_child (SP_OBJECT_REPR (defs), repr, NULL);
		spnew = (SPGradient *) doc->getObjectByRepr(repr);
		g_assert (gr != NULL);
		g_assert (SP_IS_GRADIENT (gr));
		g_print ("GVECTORNORM: Created new vector gradient %s\n", SP_OBJECT_ID (spnew));
		/* Step 3 - set vector of new gradient */
		sp_gradient_repr_set_vector (spnew, SP_OBJECT_REPR (spnew), gr->vector);
		g_print ("GVECTORNORM: Added stops to %s\n", SP_OBJECT_ID (spnew));
		/* Step 4 - set state flag */
		spnew->state = SP_GRADIENT_STATE_VECTOR;
		g_print ("GVECTORNORM: Set of %s to vector normalized\n", SP_OBJECT_ID (spnew));
		/* Step 5 - set href of old vector */
		sp_gradient_repr_set_link (SP_OBJECT_REPR (gr), spnew);
		g_print ("GVECTORNORM: Set href of %s to %s\n", SP_OBJECT_ID (gr), SP_OBJECT_ID (spnew));
		/* Step 6 - clear stops of old gradient */
		sp_gradient_repr_set_vector (gr, SP_OBJECT_REPR (gr), NULL);
		g_print ("GVECTORNORM: Cleared all stops of %s\n", SP_OBJECT_ID (gr));
		/* Now we have successfully created new normalized vector, and cleared old stops */
		return spnew;
	} else {
		/* Normal situation: gradient is in <defs> */

		/* First make sure we have vector directly defined (i.e. gr has its own stops) */
		if (!gr->has_stops) {
			/* We do not have stops ourselves, so flatten stops as well */
			sp_gradient_ensure_vector (gr);
			g_assert (gr->vector);
			// this adds stops from gr->vector as children to gr
			sp_gradient_repr_set_vector (gr, SP_OBJECT_REPR (gr), gr->vector);
			//g_print ("GVECTORNORM: Added stops to %s\n", SP_OBJECT_ID (gr));
		}

		/* If gr hrefs some other gradient, remove the href */
		if (gr->ref->getObject()) {
			/* We are hrefing someone, so require flattening */
			SP_OBJECT(gr)->updateRepr(((SPObject *) gr)->repr, SP_OBJECT_WRITE_EXT | SP_OBJECT_WRITE_ALL);
			g_print ("GVECTORNORM: Gradient %s attributes flattened\n", SP_OBJECT_ID (gr));
			sp_gradient_repr_set_link (SP_OBJECT_REPR (gr), NULL);
		}

		/* Everything is OK, set state flag */
		gr->state = SP_GRADIENT_STATE_VECTOR;
		return gr;
	}
}

/**
 * Creates new private gradient for the given vector
 */

static SPGradient *
sp_gradient_get_private_normalized (SPDocument *document, SPGradient *vector, bool islinear)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (vector != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (vector), NULL);
	g_return_val_if_fail (vector->state == SP_GRADIENT_STATE_VECTOR, NULL);

	SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS (document);

	// create a new private gradient of the requested type
	SPRepr *repr = islinear? sp_repr_new ("linearGradient") : sp_repr_new ("radialGradient");
	sp_repr_set_attr(repr, "inkscape:collect", "always");
	sp_gradient_repr_set_link (repr, vector);
	/* Append the new private gradient to defs */
	sp_repr_append_child (SP_OBJECT_REPR (defs), repr);
	sp_repr_unref (repr);

	// get corresponding object
	SPGradient *gr = (SPGradient *) document->getObjectByRepr(repr);
	g_assert (gr != NULL);
	g_assert (SP_IS_GRADIENT (gr));
	// set state
	gr->state = SP_GRADIENT_STATE_PRIVATE;

	return gr;
}

/**
 * If gr has users already, create a new private; also checks if gr links to vector, relinks if not
 */

SPGradient *
sp_gradient_clone_private_if_necessary (SPGradient *gr, SPGradient *vector, bool islinear)
{
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (vector != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (vector), NULL);
	g_return_val_if_fail (vector->state == SP_GRADIENT_STATE_VECTOR, NULL);

	/* If we are already normalized private, change href and return */
	if ((gr->state == SP_GRADIENT_STATE_PRIVATE) && (SP_OBJECT_HREFCOUNT (gr) == 1)) {
		if ( gr->ref->getObject() != vector) {
			/* our href is not the vector; relink */
			sp_gradient_repr_set_link (SP_OBJECT_REPR (gr), vector);
		}
		return gr;
	}

	SPDocument *doc = SP_OBJECT_DOCUMENT (gr);
	SPObject *defs = SP_DOCUMENT_DEFS (doc);

	if ((gr->has_stops) ||
	    (gr->state != SP_GRADIENT_STATE_UNKNOWN) ||
	    (SP_OBJECT_PARENT (gr) != SP_OBJECT (defs)) ||
	    (SP_OBJECT_HREFCOUNT (gr) > 1)) {
       	// we have to clone a fresh new private gradient for the given vector
		return sp_gradient_get_private_normalized (doc, vector, islinear);
	} else {
		/* Set state */
		gr->state = SP_GRADIENT_STATE_PRIVATE;
		return gr;
	}
}

/*
 * Either normalizes given gradient to private, or returns fresh normalized
 * private - gradient is flattened in any case, and vector set.
 * Vector has to be normalized beforehand.
 */

SPGradient *
sp_gradient_ensure_private_normalized (SPGradient *gr, SPGradient *vector)
{
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);

	gr = sp_gradient_clone_private_if_necessary (gr, vector, true);

	// add converting to userspaceonuse here

	return gr;
}

SPGradient *
sp_gradient_ensure_radial_private_normalized (SPGradient *gr, SPGradient *vector)
{
	g_return_val_if_fail (SP_IS_RADIALGRADIENT (gr), NULL);

	gr = sp_gradient_clone_private_if_necessary (gr, vector, false);

	// add converting to userspaceonuse here

	return gr;
}


/*
 * Sets item fill to lineargradient with given vector, creating
 * new private gradient, if needed
 * gr has to be normalized vector
 */

SPGradient *
sp_item_force_fill_lineargradient_vector (SPItem *item, SPGradient *gr)
{
	SPGradient *pg;

	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

	SPStyle *style = SP_OBJECT_STYLE (item);

	if ((style->fill.type != SP_PAINT_TYPE_PAINTSERVER) || !SP_IS_LINEARGRADIENT (SP_STYLE_FILL_SERVER (style))) {
		/* Current fill style is not lineargradient, so construct everything */
		pg = sp_gradient_get_private_normalized (SP_OBJECT_DOCUMENT (item), gr, true);
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "fill", pg);
		return pg;
	} else {
		/* Current fill style is lineargradient */
		SPGradient *ig = SP_GRADIENT (SP_STYLE_FILL_SERVER (style));
		if ((ig->state != SP_GRADIENT_STATE_PRIVATE) || (SP_OBJECT_HREFCOUNT (ig) != 1)) {
			/* Check, whether we have to normalize private gradient */
			pg = sp_gradient_ensure_private_normalized (ig, gr);
			g_return_val_if_fail (pg != NULL, NULL);
			g_return_val_if_fail (SP_IS_LINEARGRADIENT (pg), NULL);
			if (pg != ig) {
				/* We have to change object style here */
				g_print ("Changing object %s fill to gradient %s requested\n", SP_OBJECT_ID (item), SP_OBJECT_ID (pg));
				sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "fill", pg);
			}
			return pg;
		} else {
			/* ig is private gradient, so change href to vector */
			g_assert (ig->state == SP_GRADIENT_STATE_PRIVATE);
			if ( ig->ref->getObject() != gr ) {
				/* href is not vector */
				sp_gradient_repr_set_link (SP_OBJECT_REPR (ig), gr);
			}
			return ig;
		}
	}
}

SPGradient *
sp_item_force_stroke_lineargradient_vector (SPItem *item, SPGradient *gr)
{
	SPGradient *pg;

	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

	SPStyle *style = SP_OBJECT_STYLE (item);
	g_assert (style != NULL);

	if ((style->stroke.type != SP_PAINT_TYPE_PAINTSERVER) || !SP_IS_LINEARGRADIENT (SP_STYLE_STROKE_SERVER (style))) {
		/* Current fill style is not lineargradient, so construct everything */
		pg = sp_gradient_get_private_normalized (SP_OBJECT_DOCUMENT (item), gr, true);
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "stroke", pg);
		return pg;
	} else {
		/* Current fill style is lineargradient */
		SPGradient *ig = SP_GRADIENT (SP_STYLE_STROKE_SERVER (style));
		if ((ig->state != SP_GRADIENT_STATE_PRIVATE) || (SP_OBJECT_HREFCOUNT (ig) != 1)) {
			/* Check, whether we have to normalize private gradient */
			pg = sp_gradient_ensure_private_normalized (ig, gr);
			g_return_val_if_fail (pg != NULL, NULL);
			g_return_val_if_fail (SP_IS_LINEARGRADIENT (pg), NULL);
			if (pg != ig) {
				/* We have to change object style here */
				g_print ("Changing object %s stroke to gradient %s requested\n", SP_OBJECT_ID (item), SP_OBJECT_ID (pg));
				sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "stroke", pg);
			}
			return pg;
		}
		/* ig is private gradient, so change href to vector */
		g_assert (ig->state == SP_GRADIENT_STATE_PRIVATE);
		if ( ig->ref->getObject() != gr ) {
			/* href is not vector */
			sp_gradient_repr_set_link (SP_OBJECT_REPR (ig), gr);
		}
		return ig;
	}
}

SPGradient *
sp_item_force_fill_radialgradient_vector (SPItem *item, SPGradient *gr)
{
	SPGradient *pg;

	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

	SPStyle *style = SP_OBJECT_STYLE (item);

	if ((style->fill.type != SP_PAINT_TYPE_PAINTSERVER) || !SP_IS_RADIALGRADIENT (SP_STYLE_FILL_SERVER (style))) {
		/* Current fill style is not radialgradient, so construct everything */
		pg = sp_gradient_get_private_normalized (SP_OBJECT_DOCUMENT (item), gr, false);
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "fill", pg);
		return pg;
	} else {
		/* Current fill style is radialgradient */
		SPGradient *ig = SP_GRADIENT (SP_STYLE_FILL_SERVER (style));
		if ((ig->state != SP_GRADIENT_STATE_PRIVATE) || (SP_OBJECT_HREFCOUNT (ig) != 1)) {
			/* Check, whether we have to normalize private gradient */
			pg = sp_gradient_ensure_radial_private_normalized (ig, gr);
			g_return_val_if_fail (pg != NULL, NULL);
			g_return_val_if_fail (SP_IS_RADIALGRADIENT (pg), NULL);
			if (pg != ig) {
				/* We have to change object style here */
				g_print ("Changing object %s fill to gradient %s requested\n", SP_OBJECT_ID (item), SP_OBJECT_ID (pg));
				sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "fill", pg);
			}
			return pg;
		} else {
			/* ig is private gradient, so change href to vector */
			g_assert (ig->state == SP_GRADIENT_STATE_PRIVATE);
			if ( ig->ref->getObject() != gr ) {
				/* href is not vector */
				sp_gradient_repr_set_link (SP_OBJECT_REPR (ig), gr);
			}
			return ig;
		}
	}
}

SPGradient *
sp_item_force_stroke_radialgradient_vector (SPItem *item, SPGradient *gr)
{
	SPGradient *pg;

	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

	SPStyle *style = SP_OBJECT_STYLE (item);
	g_assert (style != NULL);

	if ((style->stroke.type != SP_PAINT_TYPE_PAINTSERVER) || !SP_IS_RADIALGRADIENT (SP_STYLE_STROKE_SERVER (style))) {
		/* Current fill style is not radialgradient, so construct everything */
		pg = sp_gradient_get_private_normalized (SP_OBJECT_DOCUMENT (item), gr, false);
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "stroke", pg);
		return pg;
	} else {
		/* Current fill style is radialgradient */
		SPGradient *ig = SP_GRADIENT (SP_STYLE_STROKE_SERVER (style));
		if ((ig->state != SP_GRADIENT_STATE_PRIVATE) || (SP_OBJECT_HREFCOUNT (ig) != 1)) {
			/* Check, whether we have to normalize private gradient */
			pg = sp_gradient_ensure_radial_private_normalized (ig, gr);
			g_return_val_if_fail (pg != NULL, NULL);
			g_return_val_if_fail (SP_IS_RADIALGRADIENT (pg), NULL);
			if (pg != ig) {
				/* We have to change object style here */
				g_print ("Changing object %s stroke to gradient %s requested\n", SP_OBJECT_ID (item), SP_OBJECT_ID (pg));
				sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "stroke", pg);
			}
			return pg;
		}
		/* ig is private gradient, so change href to vector */
		g_assert (ig->state == SP_GRADIENT_STATE_PRIVATE);
		if ( ig->ref->getObject() != gr ) {
			/* href is not vector */
			sp_gradient_repr_set_link (SP_OBJECT_REPR (ig), gr);
		}
		return ig;
	}
}

static void
sp_gradient_repr_set_link (SPRepr *repr, SPGradient *link)
{
	const gchar *id;
	gchar *ref;
	gint len;

	g_return_if_fail (repr != NULL);
	g_return_if_fail (link != NULL);
	g_return_if_fail (SP_IS_GRADIENT (link));

	if (link) {
		id = SP_OBJECT_ID (link);
		len = strlen (id);
		ref = (gchar*) alloca (len + 2);
		*ref = '#';
		memcpy (ref + 1, id, len + 1);
	} else {
		ref = NULL;
	}

	sp_repr_set_attr (repr, "xlink:href", ref);
}

static void
sp_item_repr_set_style_gradient (SPRepr *repr, const gchar *property, SPGradient *gr)
{
	SPCSSAttr *css;
	gchar *val;

	g_return_if_fail (repr != NULL);
	g_return_if_fail (gr != NULL);
	g_return_if_fail (SP_IS_GRADIENT (gr));

	val = g_strdup_printf ("url(#%s)", SP_OBJECT_ID (gr));
	css = sp_repr_css_attr_new ();
	sp_repr_css_set_property (css, property, val);
	g_free (val);
	sp_repr_css_change_recursive (repr, css, "style");
	sp_repr_css_attr_unref (css);
}

/*
 * Get default normalized gradient vector of document, create if there is none
 */

SPGradient *
sp_document_default_gradient_vector (SPDocument *document)
{
	SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS (document);

	for (SPObject *child = sp_object_first_child(SP_OBJECT(defs)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
		if (SP_IS_GRADIENT (child)) {
			SPGradient *gr;
			gr = SP_GRADIENT (child);
			if (gr->state == SP_GRADIENT_STATE_VECTOR) return gr;
			if (gr->state == SP_GRADIENT_STATE_PRIVATE) break;
			sp_gradient_ensure_vector (gr);
			if (gr->has_stops) {
				/* We have everything, but push it through normalization testing to be sure */
				return sp_gradient_ensure_vector_normalized (gr);
			}
		}
	}

	/* There were no suitable vector gradients - create one */
	SPRepr *repr, *stop;
	repr = sp_repr_new ("linearGradient");
	//sp_repr_set_attr(repr, "inkscape:collect", "always");
	stop = sp_repr_new ("stop");
	sp_repr_set_attr (stop, "style", "stop-color:#000;stop-opacity:1;");
	sp_repr_set_attr (stop, "offset", "0");
	sp_repr_append_child (repr, stop);
	sp_repr_unref (stop);
	stop = sp_repr_new ("stop");
	sp_repr_set_attr (stop, "style", "stop-color:#fff;stop-opacity:1;");
	sp_repr_set_attr (stop, "offset", "1");
	sp_repr_append_child (repr, stop);
	sp_repr_unref (stop);

	sp_repr_add_child (SP_OBJECT_REPR (defs), repr, NULL);
	sp_repr_unref (repr);

	/* fixme: This does not look like nice */
	SPGradient *gr;
	gr = (SPGradient *) document->getObjectByRepr(repr);
	g_assert (gr != NULL);
	g_assert (SP_IS_GRADIENT (gr));
	/* fixme: Maybe add extra sanity check here */
	gr->state = SP_GRADIENT_STATE_VECTOR;

	return gr;
}

/*
 * Get private vector of given gradient
 */

SPGradient *
sp_gradient_get_vector (SPGradient *gradient, gboolean force_private)
{
	SPGradient *ref;
	g_return_val_if_fail (gradient != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gradient), NULL);

	/* follow the chain of references to find the first gradient
	 * with gradient stops */
	ref = gradient;
	while ( !SP_GRADIENT_HAS_STOPS(gradient) && ref ) {
		gradient = ref;
		ref = gradient->ref->getObject();
	}

	return (force_private) ? sp_gradient_ensure_vector_normalized (gradient) : gradient;
}

/*
 * We take dumb approach currently
 *
 * Just ensure gradient href == 1, and flatten it
 *
 */

void
sp_object_ensure_fill_gradient_normalized (SPObject *object)
{
	SPLinearGradient *lg;

	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_OBJECT (object));
	/* Need style */
	g_return_if_fail (object->style != NULL);
	/* Fill has to be set */
	g_return_if_fail (object->style->fill.set);
	/* Fill has to be paintserver */
	g_return_if_fail (object->style->fill.type == SP_PAINT_TYPE_PAINTSERVER);
	/* Has to be linear gradient */
	g_return_if_fail (SP_IS_LINEARGRADIENT (SP_OBJECT_STYLE_FILL_SERVER (object)));

	lg = SP_LINEARGRADIENT (SP_OBJECT_STYLE_FILL_SERVER (object));

	if (SP_OBJECT_HREFCOUNT (lg) > 1) {
		const gchar *sstr;
		SPRepr *repr;
		gchar *val, *newval;
		/* We have to clone gradient */
		sp_gradient_ensure_vector (SP_GRADIENT (lg));
		repr = sp_lineargradient_build_repr (lg, TRUE);
		lg = (SPLinearGradient *)SP_DOCUMENT_DEFS(SP_OBJECT_DOCUMENT(object))->appendChildRepr(repr);
		sp_repr_unref (repr);
		val = g_strdup_printf ("url(#%s)", SP_OBJECT_ID (lg));
		sstr = sp_object_getAttribute (object, "style", NULL);
		newval = sp_style_change_property (sstr, "fill", val);
		g_free (newval);
		sp_object_setAttribute (object, "style", newval, NULL);
		g_free (newval);
	}
}

void sp_object_ensure_stroke_gradient_normalized(SPObject *)
{
	g_warning ("file %s: line %d: Normalization of stroke gradient not implemented", __FILE__, __LINE__);
}

/* Fixme: This belongs to SVG/style eventually */

static gchar *
sp_style_change_property (const gchar *sstr, const gchar *key, const gchar *value)
{
	const gchar *s;
	gchar *buf, *d;
	gint len;

	g_print ("%s <- %s:%s\n", sstr, key, value);

	if (!sstr) {
		if (!value) return NULL;
		return g_strdup_printf ("%s:%s;", key, value);
	}

	s = sstr;
	len = strlen (key);
	buf = (gchar*)alloca (strlen (sstr) + strlen (key) + ((value) ? strlen (value) : 0) + 2);
	d = buf;

	while (*s) {
		while (*s && isspace (*s)) s += 1;
		if (*s) {
			const gchar *q;
			q = strchr (s, ':');
			if (q) {
				const gchar *e;
				e = strchr (q, ';');
				if (e) {
					if (((q - s) == len) && !strncmp (s, key, len)) {
						/* Found it */
						g_print ("found %s:%s\n", key, value);
						if (value) {
							memcpy (d, key, len);
							d += len;
							*d = ':';
							d += 1;
							memcpy (d, value, strlen (value));
							d += strlen (value);
							*d = ';';
							d += 1;
						}
					} else {
						/* Copy key:value; pair */
						memcpy (d, s, e + 1 - s);
						d += e + 1 - s;
					}
					s = e + 1;
				} else {
					gint slen;
					/* Copy up to end */
					slen = strlen (s);
					memcpy (d, s, slen);
					d += slen;
					s += slen;
				}
			} else {
				gint slen;
				/* Copy up to end */
				slen = strlen (s);
				memcpy (d, s, slen);
				d += slen;
				s += slen;
			}
			*d = '\0';
			g_print ("%s\n", buf);
		}
	}

	*d = '\0';

	return (*buf) ? g_strdup (buf) : NULL;
}
