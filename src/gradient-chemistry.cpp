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
#include "desktop-style.h"
#include "sp-gradient.h"
#include "sp-gradient-reference.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-root.h"
#include "sp-tspan.h"
#include "gradient-chemistry.h"
#include "libnr/nr-point-ops.h"
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include "svg/svg.h"


// Terminology:
// "vector" is a gradient that has stops but not position coords. It can be referenced by one or more privates. Objects should not refer to it directly. It has no radial/linear distinction.
// "private" is a gradient that has no stops but has position coords (e.g. center, radius etc for a radial). It references a vector for the actual colors. Each private is only used by one object. It is either linear or radial.

static void sp_gradient_repr_set_link (SPRepr *repr, SPGradient *gr);
static void sp_item_repr_set_style_gradient (SPRepr *repr, const gchar *property, SPGradient *gr, bool recursive);

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

		/* Step 1 - flatten original EXCEPT vector */
		SP_OBJECT(gr)->updateRepr(((SPObject *) gr)->repr, SP_OBJECT_WRITE_EXT | SP_OBJECT_WRITE_ALL);

		/* Step 2 - create new empty gradient and prepend it to <defs> */
		repr = sp_repr_new ("linearGradient");
		//sp_repr_set_attr(repr, "inkscape:collect", "always");
		sp_repr_add_child (SP_OBJECT_REPR (defs), repr, NULL);
		spnew = (SPGradient *) doc->getObjectByRepr(repr);
		g_assert (gr != NULL);
		g_assert (SP_IS_GRADIENT (gr));

		/* Step 3 - set vector of new gradient */
		sp_gradient_repr_set_vector (spnew, SP_OBJECT_REPR (spnew), gr->vector);

		/* Step 4 - set state flag */
		spnew->state = SP_GRADIENT_STATE_VECTOR;

		/* Step 5 - set href of old vector */
		sp_gradient_repr_set_link (SP_OBJECT_REPR (gr), spnew);

		/* Step 6 - clear stops of old gradient */
		sp_gradient_repr_set_vector (gr, SP_OBJECT_REPR (gr), NULL);

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
sp_gradient_get_private_normalized (SPDocument *document, SPGradient *vector, SPGradientType type)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (vector != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (vector), NULL);
	g_return_val_if_fail (SP_GRADIENT_HAS_STOPS(vector), NULL);

	SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS (document);

	// create a new private gradient of the requested type
	SPRepr *repr;
	if (type == SP_GRADIENT_TYPE_LINEAR) {
		repr = sp_repr_new ("linearGradient");
	} else {
		repr = sp_repr_new ("radialGradient");
	}

	// privates are garbage-collectable
	sp_repr_set_attr(repr, "inkscape:collect", "always");

	// link to vector
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
Count how many times gr is used by the styles of o and its descendants
*/
guint
count_gradient_hrefs (SPObject *o, SPGradient *gr)
{
	if (!o)
		return 1;

	guint i = 0;

	SPStyle *style = SP_OBJECT_STYLE (o);
	if (style && 
			style->fill.type == SP_PAINT_TYPE_PAINTSERVER &&
			SP_IS_GRADIENT (SP_STYLE_FILL_SERVER (style)) &&
			SP_GRADIENT (SP_STYLE_FILL_SERVER (style)) == gr) {
		i ++;
	}
	if (style && 
			style->stroke.type == SP_PAINT_TYPE_PAINTSERVER &&
			SP_IS_GRADIENT (SP_STYLE_STROKE_SERVER (style)) &&
			SP_GRADIENT (SP_STYLE_STROKE_SERVER (style)) == gr) {
		i ++;
	}

	for ( SPObject *child = sp_object_first_child(o) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		i += count_gradient_hrefs (child, gr);
	}

	return i;
}


/**
 * If gr has other users, create a new private; also check if gr links to vector, relink if not
 */
SPGradient *
sp_gradient_clone_private_if_necessary (SPGradient *gr, SPGradient *vector, SPGradientType type, SPObject *o)
{
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (vector != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (vector), NULL);
	g_return_val_if_fail (SP_GRADIENT_HAS_STOPS(vector), NULL);

	// user is the object that uses this gradient; normally it's item but for tspans, we
	// check its ancestor text so that tspans don't get different gradients from their
	// texts.
	SPObject *user = o;
	while (SP_IS_TSPAN(user)) {
		user = SP_OBJECT_PARENT (user);
	}

	// Check the number of uses of the gradient within this object;
	// if we are private and there are no other users,
	if ((gr->state == SP_GRADIENT_STATE_PRIVATE) && (SP_OBJECT_HREFCOUNT (gr) <= count_gradient_hrefs(user, gr))) {
		// check vector
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

		// create an empty one
		SPGradient *gr_new = sp_gradient_get_private_normalized (doc, vector, type);

		// copy all the attributes to it
		SPRepr *repr_new = SP_OBJECT_REPR (gr_new);
		SPRepr *repr = SP_OBJECT_REPR (gr);
		sp_repr_set_attr (repr_new, "gradientUnits", sp_repr_attr (repr, "gradientUnits"));
		sp_repr_set_attr (repr_new, "gradientTransform", sp_repr_attr (repr, "gradientTransform"));
		sp_repr_set_attr (repr_new, "spreadMethod", sp_repr_attr (repr, "spreadMethod"));
		if (SP_IS_RADIALGRADIENT (gr)) {
			sp_repr_set_attr (repr_new, "cx", sp_repr_attr (repr, "cx"));
			sp_repr_set_attr (repr_new, "cy", sp_repr_attr (repr, "cy"));
			sp_repr_set_attr (repr_new, "fx", sp_repr_attr (repr, "fx"));
			sp_repr_set_attr (repr_new, "fy", sp_repr_attr (repr, "fy"));
			sp_repr_set_attr (repr_new, "r", sp_repr_attr (repr, "r"));
		} else {
			sp_repr_set_attr (repr_new, "x1", sp_repr_attr (repr, "x1"));
			sp_repr_set_attr (repr_new, "y1", sp_repr_attr (repr, "y1"));
			sp_repr_set_attr (repr_new, "x2", sp_repr_attr (repr, "x2"));
			sp_repr_set_attr (repr_new, "y2", sp_repr_attr (repr, "y2"));
		}

		return gr_new;
	} else {
		/* Set state */
		gr->state = SP_GRADIENT_STATE_PRIVATE;
		return gr;
	}
}

SPGradient *
sp_gradient_convert_to_userspace (SPGradient *gr, SPItem *item, const gchar *property)
{
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);

	// First, clone it if it is shared
	gr = sp_gradient_clone_private_if_necessary (gr, sp_gradient_get_vector (gr, FALSE), 
					 SP_IS_RADIALGRADIENT (gr) ? SP_GRADIENT_TYPE_RADIAL : SP_GRADIENT_TYPE_LINEAR, SP_OBJECT (item));

	if (gr->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {

		SPRepr *repr = SP_OBJECT_REPR (gr);

		// calculate the bbox of the item
		NRRect bbox;
		sp_document_ensure_up_to_date (SP_OBJECT_DOCUMENT(item));
		sp_item_invoke_bbox(item, &bbox, NR::identity(), TRUE); // we need "true" bbox without item_i2d_affine
		NR::Matrix bbox2user (bbox.x1 - bbox.x0, 0, 0, bbox.y1 - bbox.y0, bbox.x0, bbox.y0);

		// skew is the additional transform, defined by the proportions of the item, 
		// that we need to apply to the gradient in order to work around this weird bit from SVG 1.1:

			// When gradientUnits="objectBoundingBox" and gradientTransform is the identity
			// matrix, the stripes of the linear gradient are perpendicular to the gradient
			// vector in object bounding box space (i.e., the abstract coordinate system where
			// (0,0) is at the top/left of the object bounding box and (1,1) is at the
			// bottom/right of the object bounding box). When the object's bounding box is not
			// square, the stripes that are conceptually perpendicular to the gradient vector
			// within object bounding box space will render non-perpendicular relative to the
			// gradient vector in user space due to application of the non-uniform scaling
			// transformation from bounding box space to user space.

		NR::Matrix skew = bbox2user;
		double exp = skew.expansion();
		skew[0] /= exp;
		skew[1] /= exp;
		skew[2] /= exp;
		skew[3] /= exp;
		skew[4] = 0;
		skew[5] = 0;

		// apply skew to the gradient
		gr->gradientTransform = skew;
		{
			gchar c[256];
			if (sp_svg_transform_write(c, 256, gr->gradientTransform)) {
				sp_repr_set_attr(SP_OBJECT_REPR(gr), "gradientTransform", c);
			} else {
				sp_repr_set_attr(SP_OBJECT_REPR(gr), "gradientTransform", NULL);
			}
		}

		// Matrix to convert points to userspace coords; postmultiply by inverse of skew so
		// as to cancel it out when it's applied to the gradient during rendering
		NR::Matrix point_convert = bbox2user * skew.inverse();

		if (SP_IS_RADIALGRADIENT (gr)) {
			SPRadialGradient *rg = SP_RADIALGRADIENT (gr);

			// original points in the bbox coords
			NR::Point c_b = NR::Point (rg->cx.computed, rg->cy.computed);
			NR::Point f_b = NR::Point (rg->fx.computed, rg->fy.computed);
			double r_b = rg->r.computed;

			// converted points in userspace coords
			NR::Point c_u = c_b * point_convert; 
			NR::Point f_u = f_b * point_convert; 
			double r_u = r_b * point_convert.expansion();

			sp_repr_set_double (repr, "cx", c_u[NR::X]);
			sp_repr_set_double (repr, "cy", c_u[NR::Y]);
			sp_repr_set_double (repr, "fx", f_u[NR::X]);
			sp_repr_set_double (repr, "fy", f_u[NR::Y]);
			sp_repr_set_double (repr, "r", r_u);

		} else {
			SPLinearGradient *lg = SP_LINEARGRADIENT (gr);

			NR::Point p1_b = NR::Point (lg->x1.computed, lg->y1.computed);
			NR::Point p2_b = NR::Point (lg->x2.computed, lg->y2.computed);

			NR::Point p1_u = p1_b * point_convert; 
			NR::Point p2_u = p2_b * point_convert;

			sp_repr_set_double (repr, "x1", p1_u[NR::X]);
			sp_repr_set_double (repr, "y1", p1_u[NR::Y]);
			sp_repr_set_double (repr, "x2", p2_u[NR::X]);
			sp_repr_set_double (repr, "y2", p2_u[NR::Y]);
		}

		// set the gradientUnits
		sp_repr_set_attr (repr, "gradientUnits", "userSpaceOnUse");
	}

	// apply the gradient to the item (may be necessary if we cloned it); not recursive
	// generally because grouped items will be taken care of later (we're being called
	// from sp_item_adjust_paint_recursive); however text and all its children should all
	// refer to one gradient, hence the recursive call for text (because we can't/don't
	// want to access tspans and set gradients on them separately)
	if (SP_IS_TEXT(item))
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), property, gr, true);
	else
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), property, gr, false);

	return gr;
}

void
sp_gradient_transform_multiply (SPGradient *gradient, NR::Matrix postmul, bool set)
{
	if (set) {
		gradient->gradientTransform = postmul;
	} else {
		gradient->gradientTransform *= postmul; // fixme: get gradient transform by climbing to hrefs?
	}
	gradient->gradientTransform_set = TRUE;

	gchar c[256];
	if (sp_svg_transform_write(c, 256, gradient->gradientTransform)) {
		sp_repr_set_attr(SP_OBJECT_REPR(gradient), "gradientTransform", c);
	} else {
		sp_repr_set_attr(SP_OBJECT_REPR(gradient), "gradientTransform", NULL);
	}
}



/*
 * Sets item fill or stroke to the gradient of the specified type with given vector, creating
 * new private gradient, if needed.
 * gr has to be normalized vector
 */

SPGradient *
sp_item_set_gradient (SPItem *item, SPGradient *gr, SPGradientType type, bool is_fill)
{
	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM (item), NULL);
	g_return_val_if_fail (gr != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

	SPStyle *style = SP_OBJECT_STYLE (item);
	g_assert (style != NULL);

	guint style_type = is_fill? style->fill.type : style->stroke.type;
	SPPaintServer *ps = NULL;
	if (style_type == SP_PAINT_TYPE_PAINTSERVER)
		ps = is_fill? SP_STYLE_FILL_SERVER (style) : SP_STYLE_STROKE_SERVER (style);

	if (ps && 
			(
			(type == SP_GRADIENT_TYPE_LINEAR && SP_IS_LINEARGRADIENT (ps)) ||
			(type == SP_GRADIENT_TYPE_RADIAL && SP_IS_RADIALGRADIENT (ps)) 
			) ) {

		/* Current fill style is the gradient of the required type */
		SPGradient *current = SP_GRADIENT (ps);

		//g_print ("hrefcount %d   count %d\n", SP_OBJECT_HREFCOUNT (ig), count_gradient_hrefs(SP_OBJECT (item), ig));

		if (current->state == SP_GRADIENT_STATE_PRIVATE && 
					(SP_OBJECT_HREFCOUNT (current) == 1 || SP_OBJECT_HREFCOUNT (current) == count_gradient_hrefs(SP_OBJECT (item), current))) {
			// current is private and it's either used once, or all its uses are by children of item; 
                   // so just change its href to vector

			g_assert (current->state == SP_GRADIENT_STATE_PRIVATE);

			if ( current->ref->getObject() != gr ) {
				/* href is not the vector */
				sp_gradient_repr_set_link (SP_OBJECT_REPR (current), gr);
			}
			SP_OBJECT (item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
			return current;

		} else {
			// the gradient is not private, or it is shared with someone else;
			// normalize it (this includes creating new private if necessary)
			SPGradient *normalized = sp_gradient_clone_private_if_necessary (current, gr, type, item);

			g_return_val_if_fail (normalized != NULL, NULL);

			if (normalized != current) {
				/* We have to change object style here; recursive because this is used from fill&stroke and must work for groups etc. */
				sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), is_fill? "fill" : "stroke", normalized, true);
			}
			SP_OBJECT (item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
			return normalized;
		}

	} else {
		/* Current fill style is not a gradient or wrong type, so construct everything */
		SPGradient *constructed = sp_gradient_get_private_normalized (SP_OBJECT_DOCUMENT (item), gr, type);
		constructed = sp_gradient_convert_to_userspace (constructed, item, is_fill? "fill" : "stroke");
		sp_item_repr_set_style_gradient (SP_OBJECT_REPR (item), is_fill? "fill" : "stroke", constructed, true);
		SP_OBJECT (item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
		return constructed;
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
sp_item_repr_set_style_gradient (SPRepr *repr, const gchar *property, SPGradient *gr, bool recursive)
{
	g_return_if_fail (repr != NULL);
	g_return_if_fail (gr != NULL);
	g_return_if_fail (SP_IS_GRADIENT (gr));

	gchar *val = g_strdup_printf ("url(#%s)", SP_OBJECT_ID (gr));

	SPCSSAttr *css = sp_repr_css_attr_new ();
	sp_repr_css_set_property (css, property, val);
	g_free (val);
	if (recursive) {
		sp_repr_css_change_recursive (repr, css, "style");
	} else {
		sp_repr_css_change (repr, css, "style");
	}
	sp_repr_css_attr_unref (css);
}

/*
 * Get default normalized gradient vector of document, create if there is none
 */

SPGradient *
sp_document_default_gradient_vector (SPDocument *document, guint32 color)
{
	SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS (document);

	SPRepr *repr = sp_repr_new ("linearGradient");

	sp_repr_set_attr(repr, "inkscape:collect", "always"); 
      // set here, but removed when it's edited in the gradient editor
	// to further reduce clutter, we could 
	// (1) here, search gradients by color and return what is found without duplication
	// (2) in fill & stroke, show only one copy of each gradient in list

	SPRepr *stop = sp_repr_new ("stop");

	gchar b[64];
	sp_svg_write_color (b, 64, color);

	{
	gchar *t = g_strdup_printf ("stop-color:%s;stop-opacity:1;", b);
	sp_repr_set_attr (stop, "style", t);
	g_free (t);
	}

	sp_repr_set_attr (stop, "offset", "0");

	sp_repr_append_child (repr, stop);
	sp_repr_unref (stop);

	stop = sp_repr_new ("stop");

	{
	gchar *t = g_strdup_printf ("stop-color:%s;stop-opacity:0;", b);
	sp_repr_set_attr (stop, "style", t);
	g_free (t);
	}

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

/**
Return the preferred vector for an object, made from its current color or from desktop style if none
*/
SPGradient *
sp_gradient_vector_for_object (SPDocument *doc, SPDesktop *desktop, SPObject *o, bool is_fill)
{
    SPGradient *vector;

    // take the color of the object
    guint type = is_fill ? SP_OBJECT_STYLE (o)->fill.type : SP_OBJECT_STYLE (o)->stroke.type;
    if (type == SP_PAINT_TYPE_COLOR) {
        gfloat d[4];
        sp_color_get_rgb_floatv (is_fill ? &(SP_OBJECT_STYLE (o)->fill.value.color) : &(SP_OBJECT_STYLE (o)->stroke.value.color), d);
        guint32 rgba = SP_RGBA32_F_COMPOSE(d[0], d[1], d[2], 1.0);
        vector = sp_document_default_gradient_vector (doc, rgba);
    } else { // if none, take current color of the desktop
        vector = sp_document_default_gradient_vector (doc, sp_desktop_get_color (desktop, is_fill));
    }

    return vector;
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

