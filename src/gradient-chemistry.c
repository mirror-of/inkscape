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

/*
 * fixme: This sucks a lot - basically we keep moving gradients around
 * all the time, causing LOT of unnecessary parsing
 *
 * This should be fixed by SPHeader fine grouping, but I am
 * currently too tired to write it
 *
 * Lauris
 */

static SPGradient *sp_gradient_get_private_normalized (SPDocument *document, SPGradient *vector);
static SPGradient *sp_gradient_get_radial_private_normalized (SPDocument *document, SPGradient *vector);

static void sp_gradient_repr_set_link (SPRepr *repr, SPGradient *gr);
static void sp_item_repr_set_style_gradient (SPRepr *repr, const guchar *property, SPGradient *gr);
static guchar *sp_style_change_property (const guchar *sstr, const guchar *key, const guchar *value);

/* fixme: One more step is needed - normalization vector to 0-1 (not sure 100% still) */

SPGradient *
sp_gradient_ensure_vector_normalized (SPGradient *gr)
{
	SPDocument *doc;
	SPDefs *defs;

	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);

	/* If we are already normalized vector, just return */
	if (gr->state == SP_GRADIENT_STATE_VECTOR) return gr;
	/* Fail, if we have wrong state set */
	if (gr->state != SP_GRADIENT_STATE_UNKNOWN) {
		g_warning ("file %s: line %d: Cannot normalize private gradient to vector (%s)", __FILE__, __LINE__, SP_OBJECT_ID (gr));
		return NULL;
	}

	g_print ("GVECTORNORM: Requested vector normalization of gradient %s\n", SP_OBJECT_ID (gr));

	doc = SP_OBJECT_DOCUMENT (gr);
	defs = (SPDefs *) SP_DOCUMENT_DEFS (doc);

	if (SP_OBJECT_PARENT (gr) != SP_OBJECT (defs)) {
		SPGradient *new;
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
		sp_object_invoke_write ((SPObject *) gr, ((SPObject *) gr)->repr,
					SP_OBJECT_WRITE_SODIPODI | SP_OBJECT_WRITE_ALL);
		g_print ("GVECTORNORM: Gradient %s attributes flattened\n", SP_OBJECT_ID (gr));
		/* Step 2 - create new empty gradient and prepend it to <defs> */
		repr = sp_repr_new ("linearGradient");
		sp_repr_add_child (SP_OBJECT_REPR (defs), repr, NULL);
		new = (SPGradient *) sp_document_lookup_id (doc, sp_repr_attr (repr, "id"));
		g_assert (gr != NULL);
		g_assert (SP_IS_GRADIENT (gr));
		g_print ("GVECTORNORM: Created new vector gradient %s\n", SP_OBJECT_ID (new));
		/* Step 3 - set vector of new gradient */
		sp_gradient_repr_set_vector (new, SP_OBJECT_REPR (new), gr->vector);
		g_print ("GVECTORNORM: Added stops to %s\n", SP_OBJECT_ID (new));
		/* Step 4 - set state flag */
		new->state = SP_GRADIENT_STATE_VECTOR;
		g_print ("GVECTORNORM: Set of %s to vector normalized\n", SP_OBJECT_ID (new));
		/* Step 5 - set href of old vector */
		sp_gradient_repr_set_link (SP_OBJECT_REPR (gr), new);
		g_print ("GVECTORNORM: Set href of %s to %s\n", SP_OBJECT_ID (gr), SP_OBJECT_ID (new));
		/* Step 6 - clear stops of old gradient */
		sp_gradient_repr_set_vector (gr, SP_OBJECT_REPR (gr), NULL);
		g_print ("GVECTORNORM: Cleared all stops of %s\n", SP_OBJECT_ID (gr));
		/* Now we have successfully created new normalized vector, and cleared old stops */
		return new;
	} else {
		SPObject *child;
		/* Gradient is in <defs> */
		g_print ("GVECTORNORM: Gradient %s IS in <defs>\n", SP_OBJECT_ID (gr));
		/* First make sure we have vector directly defined */
		if (!gr->has_stops) {
			/* We do not have stops ourselves, so flatten stops as well */
			sp_gradient_ensure_vector (gr);
			g_assert (gr->vector);
			sp_gradient_repr_set_vector (gr, SP_OBJECT_REPR (gr), gr->vector);
			g_print ("GVECTORNORM: Added stops to %s\n", SP_OBJECT_ID (gr));
		}
		/* Nof break free hrefing */
		if (gr->href) {
			/* We are hrefing someone, so require flattening */
			sp_object_invoke_write ((SPObject *) gr, ((SPObject *) gr)->repr,
						SP_OBJECT_WRITE_SODIPODI | SP_OBJECT_WRITE_ALL);
			g_print ("GVECTORNORM: Gradient %s attributes flattened\n", SP_OBJECT_ID (gr));
			sp_gradient_repr_set_link (SP_OBJECT_REPR (gr), NULL);
		}
		/* Now we can be sure we are in good condition */
		/* Still have to ensure we are in vector position */
		for (child = defs->children; child != NULL; child = child->next) {
			if (SP_IS_GRADIENT (child)) {
				SPGradient *gchild;
				gchild = SP_GRADIENT (child);
				if (gchild == gr) {
					/* Everything is OK, set state flag */
					/* fixme: I am not sure, whether one should clone, if hrefcount > 1 */
					gr->state = SP_GRADIENT_STATE_VECTOR;
					g_print ("GVECTORNORM: Found gradient %s in right position\n", SP_OBJECT_ID (gr));
					return gr;
				}
				/* If there is private gradient, we have to rearrange ourselves */
				if (gchild->state == SP_GRADIENT_STATE_PRIVATE) break;
			}
		}
		/* If we didn't find ourselves, <defs> is probably messed up */
		g_assert (child != NULL);
		/* Now we have to move ourselves to the beggining of <defs> */
		g_print ("GVECTORNORM: Moving %s to the beginning of <defs>\n", SP_OBJECT_ID (gr));
		if (!sp_repr_change_order (SP_OBJECT_REPR (defs), SP_OBJECT_REPR (gr), NULL)) {
			g_warning ("file %s: line %d: Cannot move gradient %s to vector position", __FILE__, __LINE__, SP_OBJECT_ID (gr));
			return NULL;
		}
		/* Everything is OK, set state flag */
		gr->state = SP_GRADIENT_STATE_VECTOR;
		g_print ("GVECTORNORM: Moved gradient %s to the right position\n", SP_OBJECT_ID (gr));
		return gr;
	}
}

/*
 * Either normalizes given gradient to private, or returns fresh normalized
 * private - gradient is flattened in any case, and vector set
 * Vector has to be normalized beforehand
 */

SPGradient *
sp_gradient_ensure_private_normalized (SPGradient *gr, SPGradient *vector)
{
	SPDocument *doc;
	SPObject *defs;

	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (vector != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (vector), NULL);
	g_return_val_if_fail (vector->state == SP_GRADIENT_STATE_VECTOR, NULL);

	/* If we are already normalized private, change href and return */
	if ((gr->state == SP_GRADIENT_STATE_PRIVATE) && (SP_OBJECT_HREFCOUNT (gr) == 1)) {
		if (gr->href != vector) {
			/* href is not vector */
			sp_gradient_repr_set_link (SP_OBJECT_REPR (gr), vector);
		}
		return gr;
	}

	g_print ("Private normalization of gradient %s requested\n", SP_OBJECT_ID (gr));

	doc = SP_OBJECT_DOCUMENT (gr);
	defs = SP_DOCUMENT_DEFS (doc);

	/* Determine, whether we have to clone fresh new gradient */
	if ((gr->has_stops) ||
	    (gr->state != SP_GRADIENT_STATE_UNKNOWN) ||
	    (SP_OBJECT_PARENT (gr) != SP_OBJECT (defs)) ||
	    (SP_OBJECT_HREFCOUNT (gr) > 1)) {
#if 0
		SPRepr *repr;
		/* We are either in some lonely place, or have multiple refs, or vector or whatever, so clone */
		/* fixme: no 'linear' please */
		repr = sp_lineargradient_build_repr (SP_LINEARGRADIENT (gr), FALSE);
		sp_gradient_repr_set_link (repr, vector);
		/* Append cloned private gradient to defs */
		sp_repr_append_child (SP_OBJECT_REPR (defs), repr);
		sp_repr_unref (repr);
		/* fixme: This does not look like nice */
		gr = (SPGradient *) sp_document_lookup_id (doc, sp_repr_attr (repr, "id"));
		g_assert (gr != NULL);
		g_assert (SP_IS_GRADIENT (gr));
		/* fixme: Maybe add extra sanity check here */
		gr->state = SP_GRADIENT_STATE_PRIVATE;
		return gr;
#else
		return sp_gradient_get_private_normalized (SP_OBJECT_DOCUMENT (gr), vector);
#endif
	} else {
		SPObject *sibling;
		/* We still have to determine, whether we have to change our position */
		for (sibling = SP_OBJECT_NEXT (gr); sibling != NULL; sibling = SP_OBJECT_NEXT (sibling)) {
			if (SP_IS_GRADIENT (sibling) && SP_GRADIENT (sibling)->state == SP_GRADIENT_STATE_VECTOR) {
				/* Found vector after us, so move */
				while (SP_OBJECT_NEXT (sibling)) sibling = SP_OBJECT_NEXT (sibling);
				if (!sp_repr_change_order (SP_OBJECT_REPR (defs), SP_OBJECT_REPR (gr), SP_OBJECT_REPR (sibling))) {
					g_warning ("Cannot move gradient %s to private position", SP_OBJECT_ID (gr));
					return NULL;
				}
				break;
			}
		}
		/* Set state */
		gr->state = SP_GRADIENT_STATE_PRIVATE;
		return gr;
	}
}

SPGradient *
sp_gradient_ensure_radial_private_normalized (SPGradient *gr, SPGradient *vector)
{
	SPDocument *doc;
	SPObject *defs;

	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_RADIALGRADIENT (gr), NULL);
	g_return_val_if_fail (vector != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (vector), NULL);
	g_return_val_if_fail (vector->state == SP_GRADIENT_STATE_VECTOR, NULL);

	/* If we are already normalized private, change href and return */
	if ((gr->state == SP_GRADIENT_STATE_PRIVATE) && (SP_OBJECT_HREFCOUNT (gr) == 1)) {
		if (gr->href != vector) {
			/* href is not vector */
			sp_gradient_repr_set_link (SP_OBJECT_REPR (gr), vector);
		}
		return gr;
	}

	g_print ("Private normalization of radial gradient %s requested\n", SP_OBJECT_ID (gr));

	doc = SP_OBJECT_DOCUMENT (gr);
	defs = SP_DOCUMENT_DEFS (doc);

	/* Determine, whether we have to clone fresh new gradient */
	if ((gr->has_stops) ||
	    (gr->state != SP_GRADIENT_STATE_UNKNOWN) ||
	    (SP_OBJECT_PARENT (gr) != SP_OBJECT (defs)) ||
	    (SP_OBJECT_HREFCOUNT (gr) > 1)) {
#if 0
		SPRepr *repr;
		/* We are either in some lonely place, or have multiple refs, or vector or whatever, so clone */
		/* fixme: no 'linear' please */
		repr = sp_radialgradient_build_repr (SP_RADIALGRADIENT (gr), FALSE);
		sp_gradient_repr_set_link (repr, vector);
		/* Append cloned private gradient to defs */
		sp_repr_append_child (SP_OBJECT_REPR (defs), repr);
		sp_repr_unref (repr);
		/* fixme: This does not look like nice */
		gr = (SPGradient *) sp_document_lookup_id (doc, sp_repr_attr (repr, "id"));
		g_assert (gr != NULL);
		g_assert (SP_IS_GRADIENT (gr));
		/* fixme: Maybe add extra sanity check here */
		gr->state = SP_GRADIENT_STATE_PRIVATE;
		return gr;
#else
		return sp_gradient_get_radial_private_normalized (SP_OBJECT_DOCUMENT (gr), vector);
#endif
	} else {
		SPObject *sibling;
		/* We still have to determine, whether we have to change our position */
		for (sibling = SP_OBJECT_NEXT (gr); sibling != NULL; sibling = SP_OBJECT_NEXT (sibling)) {
			if (SP_IS_GRADIENT (sibling) && SP_GRADIENT (sibling)->state == SP_GRADIENT_STATE_VECTOR) {
				/* Found vector after us, so move */
				while (SP_OBJECT_NEXT (sibling)) sibling = SP_OBJECT_NEXT (sibling);
				if (!sp_repr_change_order (SP_OBJECT_REPR (defs), SP_OBJECT_REPR (gr), SP_OBJECT_REPR (sibling))) {
					g_warning ("Cannot move gradient %s to private position", SP_OBJECT_ID (gr));
					return NULL;
				}
				break;
			}
		}
		/* Set state */
		gr->state = SP_GRADIENT_STATE_PRIVATE;
		return gr;
	}
}

/*
 * Releases all stale gradient references to given gradient vector,
 * preparing it for deletion (if no referenced by real objects)
 */

void
sp_gradient_vector_release_references (SPGradient *gradient)
{
	g_return_if_fail (gradient != NULL);
	g_return_if_fail (SP_IS_GRADIENT (gradient));

	if (SP_OBJECT_HREFCOUNT (gradient) > 0) {
		const GSList *glist, *l;
		glist = sp_document_get_resource_list (SP_OBJECT_DOCUMENT (gradient), "gradient");
		for (l = glist; l != NULL; l = l->next) {
			SPGradient *gr;
			gr = SP_GRADIENT (l->data);
			if (SP_OBJECT_HREFCOUNT (gr) < 1) {
				if (gr->href == gradient) {
					sp_repr_set_attr (SP_OBJECT_REPR (gr), "xlink:href", NULL);
				}
			}
		}
	}
}

/*
 * Finds and normalizes first free gradient
 */

static SPGradient *
sp_gradient_get_private_normalized (SPDocument *document, SPGradient *vector)
{
	SPDefs *defs;
	SPObject *child;
	SPRepr *repr;
	SPGradient *gr;

	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (vector != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (vector), NULL);
	g_return_val_if_fail (vector->state == SP_GRADIENT_STATE_VECTOR, NULL);

	defs = (SPDefs *) SP_DOCUMENT_DEFS (document);

	for (child = defs->children; child != NULL; child = child->next) {
		if (SP_IS_LINEARGRADIENT (child)) {
			SPGradient *gr;
			gr = SP_GRADIENT (child);
			if (SP_OBJECT_HREFCOUNT (gr) == 0) {
				if (gr->state == SP_GRADIENT_STATE_PRIVATE) {
					SPRepr *repr;
					repr = SP_OBJECT_REPR (gr);
					sp_repr_set_attr (repr, "gradientUnits", NULL);
					sp_repr_set_attr (repr, "gradientTransform", NULL);
					sp_repr_set_attr (repr, "spreadMethod", NULL);
					sp_gradient_repr_set_link (repr, vector);
					sp_repr_set_attr (repr, "x1", NULL);
					sp_repr_set_attr (repr, "y1", NULL);
					sp_repr_set_attr (repr, "x2", NULL);
					sp_repr_set_attr (repr, "y2", NULL);
					return gr;
				} else if (gr->state == SP_GRADIENT_STATE_UNKNOWN) {
					/* fixme: This is plain wrong - what if out gradient is not at private position? */
					sp_gradient_ensure_vector (gr);
					if (!gr->has_stops) {
						SPRepr *repr;
						gr = sp_gradient_ensure_private_normalized (gr, vector);
						repr = SP_OBJECT_REPR (gr);
						sp_repr_set_attr (repr, "gradientUnits", NULL);
						sp_repr_set_attr (repr, "gradientTransform", NULL);
						sp_repr_set_attr (repr, "spreadMethod", NULL);
						sp_repr_set_attr (repr, "x1", NULL);
						sp_repr_set_attr (repr, "y1", NULL);
						sp_repr_set_attr (repr, "x2", NULL);
						sp_repr_set_attr (repr, "y2", NULL);
						return gr;
					}
				}
			}
		}
	}

	/* Have to create our own */
	repr = sp_repr_new ("linearGradient");
	sp_gradient_repr_set_link (repr, vector);
	/* Append cloned private gradient to defs */
	sp_repr_append_child (SP_OBJECT_REPR (defs), repr);
	sp_repr_unref (repr);
	/* fixme: This does not look like nice */
	gr = (SPGradient *) sp_document_lookup_id (document, sp_repr_attr (repr, "id"));
	g_assert (gr != NULL);
	g_assert (SP_IS_GRADIENT (gr));
	/* fixme: Maybe add extra sanity check here */
	gr->state = SP_GRADIENT_STATE_PRIVATE;

	return gr;
}

static SPGradient *
sp_gradient_get_radial_private_normalized (SPDocument *document, SPGradient *vector)
{
	SPDefs *defs;
	SPObject *child;
	SPRepr *repr;
	SPGradient *gr;

	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (vector != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (vector), NULL);
	g_return_val_if_fail (vector->state == SP_GRADIENT_STATE_VECTOR, NULL);

	defs = (SPDefs *) SP_DOCUMENT_DEFS (document);

	for (child = defs->children; child != NULL; child = child->next) {
		if (SP_IS_RADIALGRADIENT (child)) {
			SPGradient *gr;
			gr = SP_GRADIENT (child);
			if (SP_OBJECT_HREFCOUNT (gr) == 0) {
				if (gr->state == SP_GRADIENT_STATE_PRIVATE) {
					SPRepr *repr;
					repr = SP_OBJECT_REPR (gr);
					sp_repr_set_attr (repr, "gradientUnits", NULL);
					sp_repr_set_attr (repr, "gradientTransform", NULL);
					sp_repr_set_attr (repr, "spreadMethod", NULL);
					sp_gradient_repr_set_link (repr, vector);
					sp_repr_set_attr (repr, "cx", NULL);
					sp_repr_set_attr (repr, "cy", NULL);
					sp_repr_set_attr (repr, "fx", NULL);
					sp_repr_set_attr (repr, "fy", NULL);
					sp_repr_set_attr (repr, "r", NULL);
					return gr;
				} else if (gr->state == SP_GRADIENT_STATE_UNKNOWN) {
					/* fixme: This is plain wrong - what if out gradient is not at private position? */
					sp_gradient_ensure_vector (gr);
					if (!gr->has_stops) {
						SPRepr *repr;
						gr = sp_gradient_ensure_radial_private_normalized (gr, vector);
						repr = SP_OBJECT_REPR (gr);
						sp_repr_set_attr (repr, "gradientUnits", NULL);
						sp_repr_set_attr (repr, "gradientTransform", NULL);
						sp_repr_set_attr (repr, "spreadMethod", NULL);
						sp_gradient_repr_set_link (repr, vector);
						sp_repr_set_attr (repr, "cx", NULL);
						sp_repr_set_attr (repr, "cy", NULL);
						sp_repr_set_attr (repr, "fx", NULL);
						sp_repr_set_attr (repr, "fy", NULL);
						sp_repr_set_attr (repr, "r", NULL);
						return gr;
					}
				}
			}
		}
	}

	/* Have to create our own */
	repr = sp_repr_new ("radialGradient");
	sp_gradient_repr_set_link (repr, vector);
	/* Append cloned private gradient to defs */
	sp_repr_append_child (SP_OBJECT_REPR (defs), repr);
	sp_repr_unref (repr);
	/* fixme: This does not look like nice */
	gr = (SPGradient *) sp_document_lookup_id (document, sp_repr_attr (repr, "id"));
	g_assert (gr != NULL);
	g_assert (SP_IS_GRADIENT (gr));
	/* fixme: Maybe add extra sanity check here */
	gr->state = SP_GRADIENT_STATE_PRIVATE;

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
	SPStyle *style;

	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

#if 0
	g_print ("Changing item %s gradient vector to %s requested\n", SP_OBJECT_ID (item), SP_OBJECT_ID (gr));
#endif

	style = SP_OBJECT_STYLE (item);

	if ((style->fill.type != SP_PAINT_TYPE_PAINTSERVER) || !SP_IS_LINEARGRADIENT (SP_STYLE_FILL_SERVER (style))) {
		/* Current fill style is not lineargradient, so construct everything */
		pg = sp_gradient_get_private_normalized (SP_OBJECT_DOCUMENT (item), gr);
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "fill", pg);
		return pg;
	} else {
		SPGradient *ig;
		/* Current fill style is lineargradient */
		ig = SP_GRADIENT (SP_STYLE_FILL_SERVER (style));
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
			if (ig->href != gr) {
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
	SPStyle *style;

	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

#if 0
	g_print ("Changing item %s gradient vector to %s requested\n", SP_OBJECT_ID (item), SP_OBJECT_ID (gr));
#endif

	style = SP_OBJECT_STYLE (item);
	g_assert (style != NULL);

	if ((style->stroke.type != SP_PAINT_TYPE_PAINTSERVER) || !SP_IS_LINEARGRADIENT (SP_STYLE_STROKE_SERVER (style))) {
		/* Current fill style is not lineargradient, so construct everything */
		pg = sp_gradient_get_private_normalized (SP_OBJECT_DOCUMENT (item), gr);
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "stroke", pg);
		return pg;
	} else {
		SPGradient *ig;
		/* Current fill style is lineargradient */
		ig = SP_GRADIENT (SP_STYLE_STROKE_SERVER (style));
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
		if (ig->href != gr) {
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
	SPStyle *style;

	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

#if 0
	g_print ("Changing item %s gradient vector to %s requested\n", SP_OBJECT_ID (item), SP_OBJECT_ID (gr));
#endif

	style = SP_OBJECT_STYLE (item);

	if ((style->fill.type != SP_PAINT_TYPE_PAINTSERVER) || !SP_IS_RADIALGRADIENT (SP_STYLE_FILL_SERVER (style))) {
		/* Current fill style is not radialgradient, so construct everything */
		pg = sp_gradient_get_radial_private_normalized (SP_OBJECT_DOCUMENT (item), gr);
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "fill", pg);
		return pg;
	} else {
		SPGradient *ig;
		/* Current fill style is radialgradient */
		ig = SP_GRADIENT (SP_STYLE_FILL_SERVER (style));
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
			if (ig->href != gr) {
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
	SPStyle *style;

	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

#if 0
	g_print ("Changing item %s gradient vector to %s requested\n", SP_OBJECT_ID (item), SP_OBJECT_ID (gr));
#endif

	style = SP_OBJECT_STYLE (item);
	g_assert (style != NULL);

	if ((style->stroke.type != SP_PAINT_TYPE_PAINTSERVER) || !SP_IS_RADIALGRADIENT (SP_STYLE_STROKE_SERVER (style))) {
		/* Current fill style is not radialgradient, so construct everything */
		pg = sp_gradient_get_radial_private_normalized (SP_OBJECT_DOCUMENT (item), gr);
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), "stroke", pg);
		return pg;
	} else {
		SPGradient *ig;
		/* Current fill style is radialgradient */
		ig = SP_GRADIENT (SP_STYLE_STROKE_SERVER (style));
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
		if (ig->href != gr) {
			/* href is not vector */
			sp_gradient_repr_set_link (SP_OBJECT_REPR (ig), gr);
		}
		return ig;
	}
}

static void
sp_gradient_repr_set_link (SPRepr *repr, SPGradient *link)
{
	const guchar *id;
	gchar *ref;
	gint len;

	g_return_if_fail (repr != NULL);
	g_return_if_fail (link != NULL);
	g_return_if_fail (SP_IS_GRADIENT (link));

	if (link) {
		id = SP_OBJECT_ID (link);
		len = strlen (id);
		ref = alloca (len + 2);
		*ref = '#';
		memcpy (ref + 1, id, len + 1);
	} else {
		ref = NULL;
	}

	sp_repr_set_attr (repr, "xlink:href", ref);
}

static void
sp_item_repr_set_style_gradient (SPRepr *repr, const guchar *property, SPGradient *gr)
{
	SPCSSAttr *css;
	guchar *val;

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
	SPDefs *defs;
	SPObject *child;
	SPRepr *repr, *stop;
	SPGradient *gr;

	defs = (SPDefs *) SP_DOCUMENT_DEFS (document);

	for (child = defs->children; child != NULL; child = child->next) {
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
	repr = sp_repr_new ("linearGradient");
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
	gr = (SPGradient *) sp_document_lookup_id (document, sp_repr_attr (repr, "id"));
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
	g_return_val_if_fail (gradient != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gradient), NULL);

	while (!SP_GRADIENT_HAS_STOPS (gradient) && gradient->href) gradient = gradient->href;

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
		const guchar *sstr;
		SPRepr *repr;
		guchar *val, *newval;
		/* We have to clone gradient */
		sp_gradient_ensure_vector (SP_GRADIENT (lg));
		repr = sp_lineargradient_build_repr (lg, TRUE);
		lg = (SPLinearGradient *) sp_document_add_repr (SP_OBJECT_DOCUMENT (object), repr);
		sp_repr_unref (repr);
		val = g_strdup_printf ("url(#%s)", SP_OBJECT_ID (lg));
		sstr = sp_object_getAttribute (object, "style", NULL);
		newval = sp_style_change_property (sstr, "fill", val);
		g_free (newval);
		sp_object_setAttribute (object, "style", newval, NULL);
		g_free (newval);
	}
}

void
sp_object_ensure_stroke_gradient_normalized (SPObject *object)
{
	g_warning ("file %s: line %d: Normalization of stroke gradient not implemented", __FILE__, __LINE__);
}

/* Fixme: This belongs to SVG/style eventually */

static guchar *
sp_style_change_property (const guchar *sstr, const guchar *key, const guchar *value)
{
	const guchar *s;
	guchar *buf, *d;
	gint len;

	g_print ("%s <- %s:%s\n", sstr, key, value);

	if (!sstr) {
		if (!value) return NULL;
		return g_strdup_printf ("%s:%s;", key, value);
	}

	s = sstr;
	len = strlen (key);
	buf = alloca (strlen (sstr) + strlen (key) + ((value) ? strlen (value) : 0) + 2);
	d = buf;

	while (*s) {
		while (*s && isspace (*s)) s += 1;
		if (*s) {
			const guchar *q;
			q = strchr (s, ':');
			if (q) {
				const guchar *e;
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
