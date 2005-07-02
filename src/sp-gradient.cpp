#define __SP_GRADIENT_C__

/** \file
 * SVG \<stop\> \<linearGradient\> and \<radialGradient\> implementation.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2004 David Turner
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#define noSP_GRADIENT_VERBOSE

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <libnr/nr-gradient.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-div.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-scale-ops.h>
#include <libnr/nr-matrix-translate-ops.h>
#include "libnr/nr-scale-translate-ops.h"

#include <gtk/gtksignal.h>

#include "display/nr-gradient-gpl.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "attributes.h"
#include "document-private.h"
#include "sp-object-repr.h"
#include "sp-gradient.h"
#include "gradient-chemistry.h"
#include "sp-gradient-fns.h"
#include "sp-gradient-reference.h"
#include "sp-gradient-vector.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-stop.h"
#include "uri.h"
#include "uri-references.h"
#include "xml/repr.h"

#define SP_MACROS_SILENT
#include "macros.h"

/* Has to be power of 2 */
#define NCOLORS NR_GRADIENT_VECTOR_LENGTH

static void sp_stop_class_init(SPStopClass *klass);
static void sp_stop_init(SPStop *stop);

static void sp_stop_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_stop_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_stop_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *stop_parent_class;

GType
sp_stop_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPStopClass),
            NULL, NULL,
            (GClassInitFunc) sp_stop_class_init,
            NULL, NULL,
            sizeof(SPStop),
            16,
            (GInstanceInitFunc) sp_stop_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPStop", &info, (GTypeFlags)0);
    }
    return type;
}

static void sp_stop_class_init(SPStopClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    stop_parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = sp_stop_build;
    sp_object_class->set = sp_stop_set;
    sp_object_class->write = sp_stop_write;
}

static void
sp_stop_init(SPStop *stop)
{
    stop->offset = 0.0;
    sp_color_set_rgb_rgba32(&stop->color, 0x000000ff);
}

static void sp_stop_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) stop_parent_class)->build)
        (* ((SPObjectClass *) stop_parent_class)->build)(object, document, repr);

    sp_object_read_attr(object, "offset");
    sp_object_read_attr(object, "stop-color");
    sp_object_read_attr(object, "stop-opacity");
    sp_object_read_attr(object, "style");
}

static void
sp_stop_set(SPObject *object, unsigned key, gchar const *value)
{
    SPStop *stop = SP_STOP(object);

    switch (key) {
        case SP_ATTR_STYLE: {
            /* fixme: We are reading simple values 3 times during
             *        build (Lauris) */
            /* fixme: We need presentation attributes etc. */

           // FIXME: remove the hackish "style reading" from here, instead add stop-color and
           // stop-opacity to SPStyle and use them from there. After that remove the ugly
           // sp_object_get_style_property, it's used only here. With the current code e.g. a
           // currentColor specification does not work on a stop.

            {
                gchar const *p = sp_object_get_style_property(object, "stop-color", "black");
                guint32 color = sp_svg_read_color(p, sp_color_get_rgba32_ualpha(&stop->color, 0x00));
                sp_color_set_rgb_rgba32(&stop->color, color);
            }
            {
                gchar const *p = sp_object_get_style_property(object, "stop-opacity", "1");
                gdouble opacity = sp_svg_read_percentage(p, stop->opacity);
                stop->opacity = opacity;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_PROP_STOP_COLOR: {
            {
                gchar const *p = sp_object_get_style_property(object, "stop-color", "black");
                guint32 color = sp_svg_read_color(p, sp_color_get_rgba32_ualpha(&stop->color, 0x00));
                sp_color_set_rgb_rgba32(&stop->color, color);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_PROP_STOP_OPACITY: {
            {
                gchar const *p = sp_object_get_style_property(object, "stop-opacity", "1");
                gdouble opacity = sp_svg_read_percentage(p, stop->opacity);
                stop->opacity = opacity;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_OFFSET: {
            stop->offset = sp_svg_read_percentage(value, 0.0);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        default: {
            if (((SPObjectClass *) stop_parent_class)->set)
                (* ((SPObjectClass *) stop_parent_class)->set)(object, key, value);
            break;
        }
    }
}

static Inkscape::XML::Node *
sp_stop_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPStop *stop = SP_STOP(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new("svg:stop");
    }

    gchar c[64];
    sp_svg_write_color(c, 64, sp_color_get_rgba32_ualpha(&stop->color, 255));

    Inkscape::SVGOStringStream os;
    os << "stop-color:" << c << ";stop-opacity:" << stop->opacity << ";";
    sp_repr_set_attr(repr, "style", os.str().c_str());
    sp_repr_set_attr(repr, "stop-color", NULL);
    sp_repr_set_attr(repr, "stop-opacity", NULL);
    sp_repr_set_double(repr, "offset", stop->offset);

    if (((SPObjectClass *) stop_parent_class)->write)
        (* ((SPObjectClass *) stop_parent_class)->write)(object, repr, flags);

    return repr;
}

/*
 * Gradient
 */

static void sp_gradient_class_init(SPGradientClass *klass);
static void sp_gradient_init(SPGradient *gr);

static void sp_gradient_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_gradient_release(SPObject *object);
static void sp_gradient_set(SPObject *object, unsigned key, gchar const *value);
static void sp_gradient_child_added(SPObject *object,
                                    Inkscape::XML::Node *child,
                                    Inkscape::XML::Node *ref);
static void sp_gradient_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_gradient_modified(SPObject *object, guint flags);
static Inkscape::XML::Node *sp_gradient_write(SPObject *object, Inkscape::XML::Node *repr,
                                              guint flags);

static void gradient_ref_modified(SPObject *href, guint flags, SPGradient *gradient);

static bool sp_gradient_invalidate_vector(SPGradient *gr);
static void sp_gradient_rebuild_vector(SPGradient *gr);

static void gradient_ref_changed(SPObject *old_ref, SPObject *ref, SPGradient *gradient);

static SPPaintServerClass *gradient_parent_class;

GType
sp_gradient_get_type()
{
    static GType gradient_type = 0;
    if (!gradient_type) {
        GTypeInfo gradient_info = {
            sizeof(SPGradientClass),
            NULL, NULL,
            (GClassInitFunc) sp_gradient_class_init,
            NULL, NULL,
            sizeof(SPGradient),
            16,
            (GInstanceInitFunc) sp_gradient_init,
            NULL,   /* value_table */
        };
        gradient_type = g_type_register_static(SP_TYPE_PAINT_SERVER, "SPGradient",
                                               &gradient_info, (GTypeFlags)0);
    }
    return gradient_type;
}

static void
sp_gradient_class_init(SPGradientClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    gradient_parent_class = (SPPaintServerClass *)g_type_class_ref(SP_TYPE_PAINT_SERVER);

    sp_object_class->build = sp_gradient_build;
    sp_object_class->release = sp_gradient_release;
    sp_object_class->set = sp_gradient_set;
    sp_object_class->child_added = sp_gradient_child_added;
    sp_object_class->remove_child = sp_gradient_remove_child;
    sp_object_class->modified = sp_gradient_modified;
    sp_object_class->write = sp_gradient_write;
}

static void
sp_gradient_init(SPGradient *gr)
{
    gr->ref = new SPGradientReference(SP_OBJECT(gr));
    gr->ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(gradient_ref_changed), gr));

    /* Fixme: reprs being rearranged (e.g. via the XML editor)
     *        may require us to clear the state */
    gr->state = SP_GRADIENT_STATE_UNKNOWN;

    gr->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
    gr->units_set = FALSE;

    gr->gradientTransform = NR::identity();
    gr->gradientTransform_set = FALSE;

    gr->spread = SP_GRADIENT_SPREAD_PAD;
    gr->spread_set = FALSE;

    gr->has_stops = FALSE;

    gr->vector.built = false;
    gr->vector.stops.clear();

    gr->color = NULL;
}

static void
sp_gradient_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    SPGradient *gradient = SP_GRADIENT(object);

    if (((SPObjectClass *) gradient_parent_class)->build)
        (* ((SPObjectClass *) gradient_parent_class)->build)(object, document, repr);

    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (SP_IS_STOP(ochild)) {
            gradient->has_stops = TRUE;
            break;
        }
    }

    sp_object_read_attr(object, "gradientUnits");
    sp_object_read_attr(object, "gradientTransform");
    sp_object_read_attr(object, "spreadMethod");
    sp_object_read_attr(object, "xlink:href");

    /* Register ourselves */
    sp_document_add_resource(document, "gradient", object);
}

static void
sp_gradient_release(SPObject *object)
{
    SPGradient *gradient = (SPGradient *) object;

#ifdef SP_GRADIENT_VERBOSE
    g_print("Releasing gradient %s\n", SP_OBJECT_ID(object));
#endif

    if (SP_OBJECT_DOCUMENT(object)) {
        /* Unregister ourselves */
        sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "gradient", SP_OBJECT(object));
    }

    if (gradient->ref) {
        if (gradient->ref->getObject()) {
            sp_signal_disconnect_by_data(gradient->ref->getObject(), gradient);
        }
        gradient->ref->detach();
        delete gradient->ref;
        gradient->ref = NULL;
    }

    if (gradient->color) {
        g_free(gradient->color);
        gradient->color = NULL;
    }

    if (((SPObjectClass *) gradient_parent_class)->release)
        ((SPObjectClass *) gradient_parent_class)->release(object);
}

static void
sp_gradient_set(SPObject *object, unsigned key, gchar const *value)
{
    SPGradient *gr = SP_GRADIENT(object);

    switch (key) {
        case SP_ATTR_GRADIENTUNITS:
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    gr->units = SP_GRADIENT_UNITS_USERSPACEONUSE;
                } else {
                    gr->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
                }
                gr->units_set = TRUE;
            } else {
                gr->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
                gr->units_set = FALSE;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_GRADIENTTRANSFORM: {
            NR::Matrix t;
            if (value && sp_svg_transform_read(value, &t)) {
                gr->gradientTransform = t;
                gr->gradientTransform_set = TRUE;
            } else {
                gr->gradientTransform = NR::identity();
                gr->gradientTransform_set = FALSE;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_SPREADMETHOD:
            if (value) {
                if (!strcmp(value, "reflect")) {
                    gr->spread = SP_GRADIENT_SPREAD_REFLECT;
                } else if (!strcmp(value, "repeat")) {
                    gr->spread = SP_GRADIENT_SPREAD_REPEAT;
                } else {
                    gr->spread = SP_GRADIENT_SPREAD_PAD;
                }
                gr->spread_set = TRUE;
            } else {
                gr->spread_set = FALSE;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_XLINK_HREF:
            if (value) {
                try {
                    gr->ref->attach(Inkscape::URI(value));
                } catch (Inkscape::BadURIException &e) {
                    g_warning("%s", e.what());
                    gr->ref->detach();
                }
            } else {
                gr->ref->detach();
            }
            break;
        default:
            if (((SPObjectClass *) gradient_parent_class)->set)
                ((SPObjectClass *) gradient_parent_class)->set(object, key, value);
            break;
    }
}

/**
Gets called when the gradient is (re)attached to another gradient
*/
static void
gradient_ref_changed(SPObject *old_ref, SPObject *ref, SPGradient *gr)
{
    if (old_ref) {
        sp_signal_disconnect_by_data(old_ref, gr);
    }
    if ( SP_IS_GRADIENT(ref)
         && ref != gr )
    {
        g_signal_connect(G_OBJECT(ref), "modified", G_CALLBACK(gradient_ref_modified), gr);
    }
    /* Fixme: what should the flags (second) argument be? */
    gradient_ref_modified(ref, 0, gr);
}

static void
sp_gradient_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPGradient *gr = SP_GRADIENT(object);

    sp_gradient_invalidate_vector(gr);

    if (((SPObjectClass *) gradient_parent_class)->child_added)
        (* ((SPObjectClass *) gradient_parent_class)->child_added)(object, child, ref);

    SPObject *ochild = sp_object_get_child_by_repr(object, child);
    if ( ochild && SP_IS_STOP(ochild) ) {
        gr->has_stops = TRUE;
    }

    /* Fixme: should we schedule "modified" here? */
    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_gradient_remove_child(SPObject *object, Inkscape::XML::Node *child)
{
    SPGradient *gr = SP_GRADIENT(object);

    sp_gradient_invalidate_vector(gr);

    if (((SPObjectClass *) gradient_parent_class)->remove_child)
        (* ((SPObjectClass *) gradient_parent_class)->remove_child)(object, child);

    gr->has_stops = FALSE;
    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (SP_IS_STOP(ochild)) {
            gr->has_stops = TRUE;
            break;
        }
    }

    /* Fixme: should we schedule "modified" here? */
    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_gradient_modified(SPObject *object, guint flags)
{
    SPGradient *gr = SP_GRADIENT(object);

    if (flags & SP_OBJECT_CHILD_MODIFIED_FLAG) {
        sp_gradient_invalidate_vector(gr);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    // FIXME: climb up the ladder of hrefs
    GSList *l = NULL;
    for (SPObject *child = sp_object_first_child(object) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
        g_object_ref(G_OBJECT(child));
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref(G_OBJECT(child));
    }
}

static Inkscape::XML::Node *
sp_gradient_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPGradient *gr = SP_GRADIENT(object);

    if (((SPObjectClass *) gradient_parent_class)->write)
        (* ((SPObjectClass *) gradient_parent_class)->write)(object, repr, flags);

    if (flags & SP_OBJECT_WRITE_BUILD) {
        GSList *l = NULL;
        for (SPObject *child = sp_object_first_child(object); child; child = SP_OBJECT_NEXT(child)) {
            Inkscape::XML::Node *crepr;
            crepr = child->updateRepr(NULL, flags);
            if (crepr) l = g_slist_prepend(l, crepr);
        }
        while (l) {
            sp_repr_add_child(repr, (Inkscape::XML::Node *) l->data, NULL);
            sp_repr_unref((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    }

    if (gr->ref->getURI()) {
        gchar *uri_string = gr->ref->getURI()->toString();
        sp_repr_set_attr(repr, "xlink:href", uri_string);
        g_free(uri_string);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || gr->units_set) {
        switch (gr->units) {
            case SP_GRADIENT_UNITS_USERSPACEONUSE:
                sp_repr_set_attr(repr, "gradientUnits", "userSpaceOnUse");
                break;
            default:
                sp_repr_set_attr(repr, "gradientUnits", "objectBoundingBox");
                break;
        }
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || gr->gradientTransform_set) {
        gchar c[256];
        if (sp_svg_transform_write(c, 256, gr->gradientTransform)) {
            sp_repr_set_attr(repr, "gradientTransform", c);
        } else {
            sp_repr_set_attr(repr, "gradientTransform", NULL);
        }
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || gr->spread_set) {
        /* FIXME: Ensure that gr->spread is the inherited value
         * if !gr->spread_set.  Not currently happening: see sp_gradient_modified.
         */
        switch (gr->spread) {
            case SP_GRADIENT_SPREAD_REFLECT:
                sp_repr_set_attr(repr, "spreadMethod", "reflect");
                break;
            case SP_GRADIENT_SPREAD_REPEAT:
                sp_repr_set_attr(repr, "spreadMethod", "repeat");
                break;
            default:
                sp_repr_set_attr(repr, "spreadMethod", "pad");
                break;
        }
    }

    return repr;
}

/**
 * Forces the vector to be built, if not present (i.e.\ changed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
void
sp_gradient_ensure_vector(SPGradient *gradient)
{
    g_return_if_fail(gradient != NULL);
    g_return_if_fail(SP_IS_GRADIENT(gradient));

    if (!gradient->vector.built) {
        sp_gradient_rebuild_vector(gradient);
    }
}

void
sp_gradient_set_units(SPGradient *gr, SPGradientUnits units)
{
    if (units != gr->units) {
        gr->units = units;
        gr->units_set = TRUE;
        SP_OBJECT(gr)->requestModified(SP_OBJECT_MODIFIED_FLAG);
    }
}

void
sp_gradient_set_spread(SPGradient *gr, SPGradientSpread spread)
{
    if (spread != gr->spread) {
        gr->spread = spread;
        gr->spread_set = TRUE;
        SP_OBJECT(gr)->requestModified(SP_OBJECT_MODIFIED_FLAG);
    }
}

/**
 * Returns the first of {src, src-\>ref-\>getObject(),
 * src-\>ref-\>getObject()-\>ref-\>getObject(), ...}
 * for which \a match is true, or NULL if none found.
 *
 * The raison d'être of this routine is that it correctly handles cycles in the href chain (e.g. if
 * a gradient gives itself as its href, or if each of two gradients gives the other as its href).
 *
 * \pre SP_IS_GRADIENT(src).
 */
static SPGradient *
chase_hrefs(SPGradient *const src, bool (*match)(SPGradient const *))
{
    g_return_val_if_fail(SP_IS_GRADIENT(src), NULL);

    /* Use a pair of pointers for detecting loops: p1 advances half as fast as p2.  If there is a
       loop, then once p1 has entered the loop, we'll detect it the next time the distance between
       p1 and p2 is a multiple of the loop size. */
    SPGradient *p1 = src, *p2 = src;
    bool do1 = false;
    for (;;) {
        if (match(p2)) {
            return p2;
        }

        p2 = p2->ref->getObject();
        if (!p2) {
            return p2;
        }
        if (do1) {
            p1 = p1->ref->getObject();
        }
        do1 = !do1;

        if ( p2 == p1 ) {
            /* We've been here before, so return NULL to indicate that no matching gradient found
             * in the chain. */
            return NULL;
        }
    }
}

static bool
has_stops(SPGradient const *gr)
{
    return SP_GRADIENT_HAS_STOPS(gr);
}

static bool
has_spread_set(SPGradient const *gr)
{
    return gr->spread_set;
}


/**
 * Returns private vector of given gradient (the gradient at the end of the href chain which has
 * stops), optionally normalizing it.
 *
 * \pre SP_IS_GRADIENT(gradient).
 * \pre There exists a gradient in the chain that has stops.
 */
SPGradient *
sp_gradient_get_vector(SPGradient *gradient, gboolean force_vector)
{
    g_return_val_if_fail(gradient != NULL, NULL);
    g_return_val_if_fail(SP_IS_GRADIENT(gradient), NULL);

    SPGradient *const src = chase_hrefs(gradient, has_stops);
    return ( force_vector
             ? sp_gradient_ensure_vector_normalized(src)
             : src );
}

/**
 * Returns the effective spread of given gradient (climbing up the refs chain if needed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
SPGradientSpread
sp_gradient_get_spread(SPGradient *gradient)
{
    g_return_val_if_fail(SP_IS_GRADIENT(gradient), SP_GRADIENT_SPREAD_PAD);

    SPGradient const *src = chase_hrefs(gradient, has_spread_set);
    return ( src
             ? src->spread
             : SP_GRADIENT_SPREAD_PAD ); // pad is the default
}

/**
Clears the gradient's svg:stop children from its repr
 */
void
sp_gradient_repr_clear_vector(SPGradient *gr)
{
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(gr);

    /* Collect stops from original repr */
    GSList *sl = NULL;
    for (Inkscape::XML::Node *child = repr->firstChild() ; child != NULL; child = child->next() ) {
        if (!strcmp(child->name(), "svg:stop")) {
            sl = g_slist_prepend(sl, child);
        }
    }
    /* Remove all stops */
    while (sl) {
        /* fixme: This should work, unless we make gradient
         *        into generic group */
        sp_repr_unparent((Inkscape::XML::Node *)sl->data);
        sl = g_slist_remove(sl, sl->data);
    }
}

/**
Writes the gradient's internal vector (whether from its own stops, or inherited from
refs) into the gradient repr as svg:stop elements
 */
void
sp_gradient_repr_write_vector(SPGradient *gr)
{
    g_return_if_fail(gr != NULL);
    g_return_if_fail(SP_IS_GRADIENT(gr));

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(gr);

    /* We have to be careful, as vector may be our own, so construct repr list at first */
    GSList *cl = NULL;

    for (guint i = 0; i < gr->vector.stops.size(); i++) {
        gchar c[64];
        Inkscape::SVGOStringStream os;
        Inkscape::XML::Node *child = sp_repr_new("svg:stop");
        sp_repr_set_double(child, "offset", gr->vector.stops[i].offset);
        sp_svg_write_color(c, 64, sp_color_get_rgba32_ualpha(&gr->vector.stops[i].color, 0x00));
        os << "stop-color:" << c << ";stop-opacity:" << gr->vector.stops[i].opacity << ";";
        sp_repr_set_attr(child, "style", os.str().c_str());
        /* Order will be reversed here */
        cl = g_slist_prepend(cl, child);
    }

    sp_gradient_repr_clear_vector(gr);

    /* And insert new children from list */
    while (cl) {
        Inkscape::XML::Node *child = static_cast<Inkscape::XML::Node *>(cl->data);
        sp_repr_add_child(repr, child, NULL);
        sp_repr_unref(child);
        cl = g_slist_remove(cl, child);
    }
}

static void
gradient_ref_modified(SPObject *href, guint flags, SPGradient *gradient)
{
    if (sp_gradient_invalidate_vector(gradient)) {
        SP_OBJECT(gradient)->requestModified(SP_OBJECT_MODIFIED_FLAG);
        /* Conditional to avoid causing infinite loop if there's a cycle in the href chain. */
    }
}

/** Return true iff change made. */
static bool
sp_gradient_invalidate_vector(SPGradient *gr)
{
    bool ret = false;

    if (gr->color != NULL) {
        void *tmp = gr->color;
        gr->color = NULL;
        g_free(tmp);
        ret = true;
    }

    if (gr->vector.built) {
        gr->vector.built = false;
        gr->vector.stops.clear();
        ret = true;
    }

    return ret;
}

/** Creates normalized color vector */
static void
sp_gradient_rebuild_vector(SPGradient *gr)
{
    gint len = 0;
    for ( SPObject *child = sp_object_first_child(SP_OBJECT(gr)) ;
          child != NULL ;
          child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_STOP(child)) {
            len ++;
        }
    }

    gr->has_stops = (len != 0);

    gr->vector.stops.clear();

    SPGradient *ref = gr->ref->getObject();
    if ( !gr->has_stops && ref ) {
        /* Copy vector from referenced gradient */
        gr->vector.built = true;   // Prevent infinite recursion.
        sp_gradient_ensure_vector(ref);
        if (!ref->vector.stops.empty()) {
            gr->vector.built = ref->vector.built;
            gr->vector.stops.assign(ref->vector.stops.begin(), ref->vector.stops.end());
            return;
        }
    }

    for (SPObject *child = sp_object_first_child(SP_OBJECT(gr)) ;
         child != NULL;
         child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_STOP(child)) {
            SPStop *stop = SP_STOP(child);

            SPGradientStop gstop;
            if (gr->vector.stops.size() > 0) {
                // "Each gradient offset value is required to be equal to or greater than the
                // previous gradient stop's offset value. If a given gradient stop's offset
                // value is not equal to or greater than all previous offset values, then the
                // offset value is adjusted to be equal to the largest of all previous offset
                // values."
                gstop.offset = MAX(stop->offset, gr->vector.stops.back().offset);
            } else {
                gstop.offset = stop->offset;
            }

            // "Gradient offset values less than 0 (or less than 0%) are rounded up to
            // 0%. Gradient offset values greater than 1 (or greater than 100%) are rounded
            // down to 100%."
            gstop.offset = CLAMP(gstop.offset, 0, 1);

            sp_color_copy(&gstop.color, &stop->color);
            gstop.opacity = stop->opacity;

            gr->vector.stops.push_back(gstop);
        }
    }

    // Normalize per section 13.2.4 of SVG 1.1.
    if (gr->vector.stops.size() == 0) {
        /* "If no stops are defined, then painting shall occur as if 'none' were specified as the
         * paint style."
         */
        {
            SPGradientStop gstop;
            gstop.offset = 0.0;
            sp_color_set_rgb_rgba32(&gstop.color, 0x00000000);
            gstop.opacity = 0.0;
            gr->vector.stops.push_back(gstop);
        }
        {
            SPGradientStop gstop;
            gstop.offset = 1.0;
            sp_color_set_rgb_rgba32(&gstop.color, 0x00000000);
            gstop.opacity = 0.0;
            gr->vector.stops.push_back(gstop);
        }
    } else {
        /* "If one stop is defined, then paint with the solid color fill using the color defined
         * for that gradient stop."
         */
        if (gr->vector.stops.front().offset > 0.0) {
            // If the first one is not at 0, then insert a copy of the first at 0.
            SPGradientStop gstop;
            gstop.offset = 0.0;
            sp_color_copy(&gstop.color, &gr->vector.stops.front().color);
            gstop.opacity = gr->vector.stops.front().opacity;
            gr->vector.stops.insert(gr->vector.stops.begin(), gstop);
        }
        if (gr->vector.stops.back().offset < 1.0) {
            // If the last one is not at 1, then insert a copy of the last at 1.
            SPGradientStop gstop;
            gstop.offset = 1.0;
            sp_color_copy(&gstop.color, &gr->vector.stops.back().color);
            gstop.opacity = gr->vector.stops.back().opacity;
            gr->vector.stops.push_back(gstop);
        }
    }

    gr->vector.built = true;
}

void
sp_gradient_ensure_colors(SPGradient *gr)
{
    if (!gr->vector.built) {
        sp_gradient_rebuild_vector(gr);
    }
    g_return_if_fail(!gr->vector.stops.empty());

    if (!gr->color) {
        gr->color = g_new(guchar, 4 * NCOLORS);
    }

    for (guint i = 0; i < gr->vector.stops.size() - 1; i++) {
        guint32 color = sp_color_get_rgba32_falpha(&gr->vector.stops[i].color,
                                                   gr->vector.stops[i].opacity);
        gint r0 = (color >> 24) & 0xff;
        gint g0 = (color >> 16) & 0xff;
        gint b0 = (color >> 8) & 0xff;
        gint a0 = color & 0xff;
        color = sp_color_get_rgba32_falpha(&gr->vector.stops[i + 1].color,
                                           gr->vector.stops[i + 1].opacity);
        gint r1 = (color >> 24) & 0xff;
        gint g1 = (color >> 16) & 0xff;
        gint b1 = (color >> 8) & 0xff;
        gint a1 = color & 0xff;
        gint o0 = (gint) floor(gr->vector.stops[i].offset * (NCOLORS - 0.001));
        gint o1 = (gint) floor(gr->vector.stops[i + 1].offset * (NCOLORS - 0.001));
        if (o1 > o0) {
            gint dr = ((r1 - r0) << 16) / (o1 - o0);
            gint dg = ((g1 - g0) << 16) / (o1 - o0);
            gint db = ((b1 - b0) << 16) / (o1 - o0);
            gint da = ((a1 - a0) << 16) / (o1 - o0);
            gint r = r0 << 16;
            gint g = g0 << 16;
            gint b = b0 << 16;
            gint a = a0 << 16;
            for (int j = o0; j < o1 + 1; j++) {
                gr->color[4 * j] = r >> 16;
                gr->color[4 * j + 1] = g >> 16;
                gr->color[4 * j + 2] = b >> 16;
                gr->color[4 * j + 3] = a >> 16;
                r += dr;
                g += dg;
                b += db;
                a += da;
            }
        }
    }
}

/**
 * Renders gradient vector to buffer.
 * RGB buffer background should be set up beforehand.
 *
 * @param len,width,height,rowstride Buffer parameters (1 or 2 dimensional).
 * @param span Full integer width of requested gradient.
 * @param pos Buffer starting position in span.
 */
static void
sp_gradient_render_vector_line_rgba(SPGradient *const gradient, guchar *buf,
                                    gint const len, gint const pos, gint const span)
{
    g_return_if_fail(gradient != NULL);
    g_return_if_fail(SP_IS_GRADIENT(gradient));
    g_return_if_fail(buf != NULL);
    g_return_if_fail(len > 0);
    g_return_if_fail(pos >= 0);
    g_return_if_fail(pos + len <= span);
    g_return_if_fail(span > 0);

    if (!gradient->color) {
        sp_gradient_ensure_colors(gradient);
    }

    gint idx = (pos * 1024 << 8) / span;
    gint didx = (1024 << 8) / span;

    for (gint x = 0; x < len; x++) {
        // Can this be done with 4 byte copies?
        *buf++ = gradient->color[4 * (idx >> 8)];
        *buf++ = gradient->color[4 * (idx >> 8) + 1];
        *buf++ = gradient->color[4 * (idx >> 8) + 2];
        *buf++ = gradient->color[4 * (idx >> 8) + 3];
        idx += didx;
    }
}

void
sp_gradient_render_vector_block_rgba(SPGradient *const gradient, guchar *buf,
                                     gint const width, gint const height, gint const rowstride,
                                     gint const pos, gint const span, gboolean const horizontal)
{
    g_return_if_fail(gradient != NULL);
    g_return_if_fail(SP_IS_GRADIENT(gradient));
    g_return_if_fail(buf != NULL);
    g_return_if_fail(width > 0);
    g_return_if_fail(height > 0);
    g_return_if_fail(pos >= 0);
    g_return_if_fail((horizontal && (pos + width <= span)) || (!horizontal && (pos + height <= span)));
    g_return_if_fail(span > 0);

    if (horizontal) {
        sp_gradient_render_vector_line_rgba(gradient, buf, width, pos, span);
        for (gint y = 1; y < height; y++) {
            memcpy(buf + y * rowstride, buf, 4 * width);
        }
    } else {
        guchar *tmp = (guchar *)alloca(4 * height);
        sp_gradient_render_vector_line_rgba(gradient, tmp, height, pos, span);
        for (gint y = 0; y < height; y++) {
            guchar *b = buf + y * rowstride;
            for (gint x = 0; x < width; x++) {
                *b++ = tmp[0];
                *b++ = tmp[1];
                *b++ = tmp[2];
                *b++ = tmp[3];
            }
            tmp += 4;
        }
    }
}

void
sp_gradient_render_vector_block_rgb(SPGradient *gradient, guchar *buf,
                                    gint const width, gint const height, gint const rowstride,
                                    gint const pos, gint const span, gboolean const horizontal)
{
    g_return_if_fail(gradient != NULL);
    g_return_if_fail(SP_IS_GRADIENT(gradient));
    g_return_if_fail(buf != NULL);
    g_return_if_fail(width > 0);
    g_return_if_fail(height > 0);
    g_return_if_fail(pos >= 0);
    g_return_if_fail((horizontal && (pos + width <= span)) || (!horizontal && (pos + height <= span)));
    g_return_if_fail(span > 0);

    if (horizontal) {
        guchar *tmp = (guchar*)alloca(4 * width);
        sp_gradient_render_vector_line_rgba(gradient, tmp, width, pos, span);
        for (gint y = 0; y < height; y++) {
            guchar *t = tmp;
            for (gint x = 0; x < width; x++) {
                gint a = t[3];
                gint fc = (t[0] - buf[0]) * a;
                buf[0] = buf[0] + ((fc + (fc >> 8) + 0x80) >> 8);
                fc = (t[1] - buf[1]) * a;
                buf[1] = buf[1] + ((fc + (fc >> 8) + 0x80) >> 8);
                fc = (t[2] - buf[2]) * a;
                buf[2] = buf[2] + ((fc + (fc >> 8) + 0x80) >> 8);
                buf += 3;
                t += 4;
            }
        }
    } else {
        guchar *tmp = (guchar*)alloca(4 * height);
        sp_gradient_render_vector_line_rgba(gradient, tmp, height, pos, span);
        for (gint y = 0; y < height; y++) {
            guchar *t = tmp + 4 * y;
            for (gint x = 0; x < width; x++) {
                gint a = t[3];
                gint fc = (t[0] - buf[0]) * a;
                buf[0] = buf[0] + ((fc + (fc >> 8) + 0x80) >> 8);
                fc = (t[1] - buf[1]) * a;
                buf[1] = buf[1] + ((fc + (fc >> 8) + 0x80) >> 8);
                fc = (t[2] - buf[2]) * a;
                buf[2] = buf[2] + ((fc + (fc >> 8) + 0x80) >> 8);
            }
        }
    }
}

NR::Matrix
sp_gradient_get_g2d_matrix(SPGradient const *gr, NR::Matrix const &ctm, NR::Rect const &bbox)
{
    if (gr->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        return ( NR::scale(bbox.dimensions())
                 * NR::translate(bbox.min())
                 * ctm );
    } else {
        return ctm;
    }
}

NR::Matrix
sp_gradient_get_gs2d_matrix(SPGradient const *gr, NR::Matrix const &ctm, NR::Rect const &bbox)
{
    if (gr->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        return ( gr->gradientTransform
                 * NR::scale(bbox.dimensions())
                 * NR::translate(bbox.min())
                 * ctm );
    } else {
        return gr->gradientTransform * ctm;
    }
}

void
sp_gradient_set_gs2d_matrix(SPGradient *gr, NR::Matrix const &ctm,
                            NR::Rect const &bbox, NR::Matrix const &gs2d)
{
    gr->gradientTransform = gs2d / ctm;
    if ( gr->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX ) {
        gr->gradientTransform = ( gr->gradientTransform
                                  / NR::translate(bbox.min())
                                  / NR::scale(bbox.dimensions()) );
    }
    gr->gradientTransform_set = TRUE;

    SP_OBJECT(gr)->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/*
 * Linear Gradient
 */

class SPLGPainter;

struct SPLGPainter {
    SPPainter painter;
    SPLinearGradient *lg;

    NRLGradientRenderer lgr;
};

static void sp_lineargradient_class_init(SPLinearGradientClass *klass);
static void sp_lineargradient_init(SPLinearGradient *lg);

static void sp_lineargradient_build(SPObject *object,
                                    SPDocument *document,
                                    Inkscape::XML::Node *repr);
static void sp_lineargradient_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_lineargradient_write(SPObject *object, Inkscape::XML::Node *repr,
                                                    guint flags);

static SPPainter *sp_lineargradient_painter_new(SPPaintServer *ps,
                                                NR::Matrix const &full_transform,
                                                NR::Matrix const &parent_transform,
                                                NRRect const *bbox);
static void sp_lineargradient_painter_free(SPPaintServer *ps, SPPainter *painter);

static void sp_lg_fill(SPPainter *painter, NRPixBlock *pb);

static SPGradientClass *lg_parent_class;

GType
sp_lineargradient_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPLinearGradientClass),
            NULL, NULL,
            (GClassInitFunc) sp_lineargradient_class_init,
            NULL, NULL,
            sizeof(SPLinearGradient),
            16,
            (GInstanceInitFunc) sp_lineargradient_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GRADIENT, "SPLinearGradient", &info, (GTypeFlags)0);
    }
    return type;
}

static void sp_lineargradient_class_init(SPLinearGradientClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPPaintServerClass *ps_class = (SPPaintServerClass *) klass;

    lg_parent_class = (SPGradientClass*)g_type_class_ref(SP_TYPE_GRADIENT);

    sp_object_class->build = sp_lineargradient_build;
    sp_object_class->set = sp_lineargradient_set;
    sp_object_class->write = sp_lineargradient_write;

    ps_class->painter_new = sp_lineargradient_painter_new;
    ps_class->painter_free = sp_lineargradient_painter_free;
}

static void sp_lineargradient_init(SPLinearGradient *lg)
{
    sp_svg_length_unset(&lg->x1, SP_SVG_UNIT_PERCENT, 0.0, 0.0);
    sp_svg_length_unset(&lg->y1, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
    sp_svg_length_unset(&lg->x2, SP_SVG_UNIT_PERCENT, 1.0, 1.0);
    sp_svg_length_unset(&lg->y2, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
}

static void sp_lineargradient_build(SPObject *object,
                                    SPDocument *document,
                                    Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) lg_parent_class)->build)
        (* ((SPObjectClass *) lg_parent_class)->build)(object, document, repr);

    sp_object_read_attr(object, "x1");
    sp_object_read_attr(object, "y1");
    sp_object_read_attr(object, "x2");
    sp_object_read_attr(object, "y2");
}

static void
sp_lineargradient_set(SPObject *object, unsigned key, gchar const *value)
{
    SPLinearGradient *lg = SP_LINEARGRADIENT(object);

    switch (key) {
        case SP_ATTR_X1:
            if (!sp_svg_length_read(value, &lg->x1)) {
                sp_svg_length_unset(&lg->x1, SP_SVG_UNIT_PERCENT, 0.0, 0.0);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y1:
            if (!sp_svg_length_read(value, &lg->y1)) {
                sp_svg_length_unset(&lg->y1, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_X2:
            if (!sp_svg_length_read(value, &lg->x2)) {
                sp_svg_length_unset(&lg->x2, SP_SVG_UNIT_PERCENT, 1.0, 1.0);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y2:
            if (!sp_svg_length_read(value, &lg->y2)) {
                sp_svg_length_unset(&lg->y2, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) lg_parent_class)->set)
                (* ((SPObjectClass *) lg_parent_class)->set)(object, key, value);
            break;
    }
}

static Inkscape::XML::Node *
sp_lineargradient_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPLinearGradient *lg = SP_LINEARGRADIENT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new("svg:linearGradient");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || lg->x1.set)
        sp_repr_set_double(repr, "x1", lg->x1.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || lg->y1.set)
        sp_repr_set_double(repr, "y1", lg->y1.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || lg->x2.set)
        sp_repr_set_double(repr, "x2", lg->x2.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || lg->y2.set)
        sp_repr_set_double(repr, "y2", lg->y2.computed);

    if (((SPObjectClass *) lg_parent_class)->write)
        (* ((SPObjectClass *) lg_parent_class)->write)(object, repr, flags);

    return repr;
}

/*
 * Basically we have to deal with transformations
 *
 * 1) color2norm - maps point in (0,NCOLORS) vector to (0,1) vector
 *    fixme: I do not know how to deal with start > 0 and end < 1
 * 2) norm2pos - maps (0,1) vector to x1,y1 - x2,y2
 * 2) gradientTransform
 * 3) bbox2user
 * 4) ctm == userspace to pixel grid

   See also (*) in sp-pattern about why we may need parent_transform.
 */

static SPPainter *
sp_lineargradient_painter_new(SPPaintServer *ps,
                              NR::Matrix const &full_transform,
                              NR::Matrix const &parent_transform,
                              NRRect const *bbox)
{
    SPLinearGradient *lg = SP_LINEARGRADIENT(ps);
    SPGradient *gr = SP_GRADIENT(ps);

    if (!gr->color) sp_gradient_ensure_colors(gr);

    SPLGPainter *lgp = g_new(SPLGPainter, 1);

    lgp->painter.type = SP_PAINTER_IND;
    lgp->painter.fill = sp_lg_fill;

    lgp->lg = lg;

    /* fixme: Technically speaking, we map NCOLORS on line
     *        [start,end] onto line [0,1] (Lauris) */
    /* fixme: I almost think we should fill color array start and
     *        end in that case (Lauris) */
    /* fixme: The alternative would be to leave these just empty
     *        garbage or something similar (Lauris) */
    /* fixme: Originally I had 1023.9999 here - not sure whether
     *        we have really to cut out ceil int (Lauris) */
    NR::Matrix color2norm(NR::identity());
    NR::Matrix color2px;
    if (gr->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        NR::Matrix norm2pos(NR::identity());

        /* BBox to user coordinate system */
        NR::Matrix bbox2user(bbox->x1 - bbox->x0, 0, 0, bbox->y1 - bbox->y0, bbox->x0, bbox->y0);

        NR::Matrix color2pos = color2norm * norm2pos;
        NR::Matrix color2tpos = color2pos * gr->gradientTransform;
        NR::Matrix color2user = color2tpos * bbox2user;
        color2px = color2user * full_transform;

    } else {
        /* Problem: What to do, if we have mixed lengths and percentages? */
        /* Currently we do ignore percentages at all, but that is not good (lauris) */

        NR::Matrix norm2pos(NR::identity());
        NR::Matrix color2pos = color2norm * norm2pos;
        NR::Matrix color2tpos = color2pos * gr->gradientTransform;
        color2px = color2tpos * full_transform;

    }

    NRMatrix v2px;
    color2px.copyto(&v2px);

    nr_lgradient_renderer_setup(&lgp->lgr, gr->color, sp_gradient_get_spread(gr), &v2px,
                                lg->x1.computed, lg->y1.computed,
                                lg->x2.computed, lg->y2.computed);

    return (SPPainter *) lgp;
}

static void
sp_lineargradient_painter_free(SPPaintServer *ps, SPPainter *painter)
{
    g_free(painter);
}

void
sp_lineargradient_set_position(SPLinearGradient *lg,
                               gdouble x1, gdouble y1,
                               gdouble x2, gdouble y2)
{
    g_return_if_fail(lg != NULL);
    g_return_if_fail(SP_IS_LINEARGRADIENT(lg));

    /* fixme: units? (Lauris)  */
    sp_svg_length_set(&lg->x1, SP_SVG_UNIT_NONE, x1, x1);
    sp_svg_length_set(&lg->y1, SP_SVG_UNIT_NONE, y1, y1);
    sp_svg_length_set(&lg->x2, SP_SVG_UNIT_NONE, x2, x2);
    sp_svg_length_set(&lg->y2, SP_SVG_UNIT_NONE, y2, y2);

    SP_OBJECT(lg)->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_lg_fill(SPPainter *painter, NRPixBlock *pb)
{
    SPLGPainter *lgp = (SPLGPainter *) painter;

    if (lgp->lg->color == NULL) {
        sp_gradient_ensure_colors (lgp->lg);
        lgp->lgr.vector = lgp->lg->color;
    }

    nr_render((NRRenderer *) &lgp->lgr, pb, NULL);
}

/*
 * Radial Gradient
 */

class SPRGPainter;

struct SPRGPainter {
    SPPainter painter;
    SPRadialGradient *rg;
    NRRGradientRenderer rgr;
};

static void sp_radialgradient_class_init(SPRadialGradientClass *klass);
static void sp_radialgradient_init(SPRadialGradient *rg);

static void sp_radialgradient_build(SPObject *object,
                                    SPDocument *document,
                                    Inkscape::XML::Node *repr);
static void sp_radialgradient_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_radialgradient_write(SPObject *object, Inkscape::XML::Node *repr,
                                                    guint flags);

static SPPainter *sp_radialgradient_painter_new(SPPaintServer *ps,
                                                NR::Matrix const &full_transform,
                                                NR::Matrix const &parent_transform,
                                                NRRect const *bbox);
static void sp_radialgradient_painter_free(SPPaintServer *ps, SPPainter *painter);

static void sp_rg_fill(SPPainter *painter, NRPixBlock *pb);

static SPGradientClass *rg_parent_class;

GType
sp_radialgradient_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPRadialGradientClass),
            NULL, NULL,
            (GClassInitFunc) sp_radialgradient_class_init,
            NULL, NULL,
            sizeof(SPRadialGradient),
            16,
            (GInstanceInitFunc) sp_radialgradient_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GRADIENT, "SPRadialGradient", &info, (GTypeFlags)0);
    }
    return type;
}

static void sp_radialgradient_class_init(SPRadialGradientClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPPaintServerClass *ps_class = (SPPaintServerClass *) klass;

    rg_parent_class = (SPGradientClass*)g_type_class_ref(SP_TYPE_GRADIENT);

    sp_object_class->build = sp_radialgradient_build;
    sp_object_class->set = sp_radialgradient_set;
    sp_object_class->write = sp_radialgradient_write;

    ps_class->painter_new = sp_radialgradient_painter_new;
    ps_class->painter_free = sp_radialgradient_painter_free;
}

static void
sp_radialgradient_init(SPRadialGradient *rg)
{
    sp_svg_length_unset(&rg->cx, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
    sp_svg_length_unset(&rg->cy, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
    sp_svg_length_unset(&rg->r, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
    sp_svg_length_unset(&rg->fx, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
    sp_svg_length_unset(&rg->fy, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
}

static void
sp_radialgradient_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) rg_parent_class)->build)
        (* ((SPObjectClass *) rg_parent_class)->build)(object, document, repr);

    sp_object_read_attr(object, "cx");
    sp_object_read_attr(object, "cy");
    sp_object_read_attr(object, "r");
    sp_object_read_attr(object, "fx");
    sp_object_read_attr(object, "fy");
}

static void
sp_radialgradient_set(SPObject *object, unsigned key, gchar const *value)
{
    SPRadialGradient *rg = SP_RADIALGRADIENT(object);

    switch (key) {
        case SP_ATTR_CX:
            if (!sp_svg_length_read(value, &rg->cx)) {
                sp_svg_length_unset(&rg->cx, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
            }
            if (!rg->fx.set) {
                rg->fx.value = rg->cx.value;
                rg->fx.computed = rg->cx.computed;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_CY:
            if (!sp_svg_length_read(value, &rg->cy)) {
                sp_svg_length_unset(&rg->cy, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
            }
            if (!rg->fy.set) {
                rg->fy.value = rg->cy.value;
                rg->fy.computed = rg->cy.computed;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_R:
            if (!sp_svg_length_read(value, &rg->r)) {
                sp_svg_length_unset(&rg->r, SP_SVG_UNIT_PERCENT, 0.5, 0.5);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_FX:
            if (!sp_svg_length_read(value, &rg->fx)) {
                sp_svg_length_unset(&rg->fx, rg->cx.unit, rg->cx.value, rg->cx.computed);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_FY:
            if (!sp_svg_length_read(value, &rg->fy)) {
                sp_svg_length_unset(&rg->fy, rg->cy.unit, rg->cy.value, rg->cy.computed);
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) rg_parent_class)->set)
                ((SPObjectClass *) rg_parent_class)->set(object, key, value);
            break;
    }
}

static Inkscape::XML::Node *
sp_radialgradient_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPRadialGradient *rg = SP_RADIALGRADIENT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new("svg:radialGradient");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || rg->cx.set) sp_repr_set_double(repr, "cx", rg->cx.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || rg->cy.set) sp_repr_set_double(repr, "cy", rg->cy.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || rg->r.set) sp_repr_set_double(repr, "r", rg->r.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || rg->fx.set) sp_repr_set_double(repr, "fx", rg->fx.computed);
    if ((flags & SP_OBJECT_WRITE_ALL) || rg->fy.set) sp_repr_set_double(repr, "fy", rg->fy.computed);

    if (((SPObjectClass *) rg_parent_class)->write)
        (* ((SPObjectClass *) rg_parent_class)->write)(object, repr, flags);

    return repr;
}

static SPPainter *
sp_radialgradient_painter_new(SPPaintServer *ps,
                              NR::Matrix const &full_transform,
                              NR::Matrix const &parent_transform,
                              NRRect const *bbox)
{
    SPRadialGradient *rg = SP_RADIALGRADIENT(ps);
    SPGradient *gr = SP_GRADIENT(ps);

    if (!gr->color) sp_gradient_ensure_colors(gr);

    SPRGPainter *rgp = g_new(SPRGPainter, 1);

    rgp->painter.type = SP_PAINTER_IND;
    rgp->painter.fill = sp_rg_fill;

    rgp->rg = rg;

    NR::Matrix gs2px;

    if (gr->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        /* fixme: We may try to normalize here too, look at linearGradient (Lauris) */

        /* BBox to user coordinate system */
        NR::Matrix bbox2user(bbox->x1 - bbox->x0, 0, 0, bbox->y1 - bbox->y0, bbox->x0, bbox->y0);

        NR::Matrix gs2user = gr->gradientTransform * bbox2user;

        gs2px = gs2user * full_transform;
    } else {
        /* Problem: What to do, if we have mixed lengths and percentages? */
        /* Currently we do ignore percentages at all, but that is not good (lauris) */

        gs2px = gr->gradientTransform * full_transform;
    }

    NRMatrix gs2px_nr;
    gs2px.copyto(&gs2px_nr);

    nr_rgradient_renderer_setup(&rgp->rgr, gr->color, sp_gradient_get_spread(gr),
                                &gs2px_nr,
                                rg->cx.computed, rg->cy.computed,
                                rg->fx.computed, rg->fy.computed,
                                rg->r.computed);

    return (SPPainter *) rgp;
}

static void
sp_radialgradient_painter_free(SPPaintServer *ps, SPPainter *painter)
{
    g_free(painter);
}

void
sp_radialgradient_set_position(SPRadialGradient *rg,
                               gdouble cx, gdouble cy, gdouble fx, gdouble fy, gdouble r)
{
    g_return_if_fail(rg != NULL);
    g_return_if_fail(SP_IS_RADIALGRADIENT(rg));

    /* fixme: units? (Lauris)  */
    sp_svg_length_set(&rg->cx, SP_SVG_UNIT_NONE, cx, cx);
    sp_svg_length_set(&rg->cy, SP_SVG_UNIT_NONE, cy, cy);
    sp_svg_length_set(&rg->fx, SP_SVG_UNIT_NONE, fx, fx);
    sp_svg_length_set(&rg->fy, SP_SVG_UNIT_NONE, fy, fy);
    sp_svg_length_set(&rg->r, SP_SVG_UNIT_NONE, r, r);

    SP_OBJECT(rg)->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_rg_fill(SPPainter *painter, NRPixBlock *pb)
{
    SPRGPainter *rgp = (SPRGPainter *) painter;

    if (rgp->rg->color == NULL) {
        sp_gradient_ensure_colors (rgp->rg);
        rgp->rgr.vector = rgp->rg->color;
    }

    nr_render((NRRenderer *) &rgp->rgr, pb, NULL);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
