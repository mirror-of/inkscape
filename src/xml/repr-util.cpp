#define __SP_REPR_UTIL_C__

/** \file
 * Miscellaneous helpers for reprs.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 1999-2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * g++ port Copyright (C) 2003 Nathan Hurst
 *
 * Licensed under GNU GPL
 */

#include "config.h"

#include <math.h>

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>

#include <glib.h>

#include <svg/stringstream.h>
#include <xml/repr-private.h>
#include <xml/sp-repr-attr.h>

/*#####################
# DEFINITIONS
#####################*/

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#ifndef MAX
#define MAX(a,b) (((a) < (b)) ? (b) : (a))
#endif

/*#####################
# FORWARD DECLARATIONS
#####################*/

static void sp_xml_ns_register_defaults ();
static char *sp_xml_ns_auto_prefix (const char *uri);

// The following are in splivarot.cpp but probably should be moved
// here...
SPRepr *LCA (SPRepr * a, SPRepr * b);
bool   Ancetre (SPRepr * a, SPRepr * who);
SPRepr *AncetreFils (SPRepr * a, SPRepr * d);

/*#####################
# UTILITY
#####################*/

/**
 * Locale-independent double to string conversion
 */
unsigned int
sp_xml_dtoa (gchar *buf, double val, unsigned int tprec, unsigned int fprec, unsigned int padf)
{
    double dival, fval, epsilon;
    int idigits, ival, i;
    i = 0;
    if (val < 0.0) {
        buf[i++] = '-';
        val = -val;
    }
    /* Determine number of integral digits */
    if (val >= 1.0) {
        idigits = (int) floor (log10 (val));
    } else {
        idigits = 0;
    }
    /* Determine the actual number of fractional digits */
    fprec = MAX (fprec, tprec - idigits);
    /* Find epsilon */
    epsilon = 0.5 * pow (10.0, - (double) fprec);
    /* Round value */
    val += epsilon;
    /* Extract integral and fractional parts */
    dival = floor (val);
    ival = (int) dival;
    fval = val - dival;
    /* Write integra */
    if (ival > 0) {
        char c[32];
        int j;
        j = 0;
        while (ival > 0) {
            c[32 - (++j)] = '0' + (ival % 10);
            ival /= 10;
        }
        memcpy (buf + i, &c[32 - j], j);
        i += j;
        tprec -= j;
    } else {
        buf[i++] = '0';
        tprec -= 1;
    }
    if ((fprec > 0) && (padf || (fval > epsilon))) {
        buf[i++] = '.';
        while ((fprec > 0) && (padf || (fval > epsilon))) {
            fval *= 10.0;
            dival = floor (fval);
            fval -= dival;
            buf[i++] = '0' + (int) dival;
            fprec -= 1;
        }

    }
    buf[i] = 0;
    return i;
}





/*#####################
# MAIN
#####################*/

/**
 * SPXMLNs
 */

static SPXMLNs *namespaces=NULL;

/*
 * There are the prefixes to use for the XML namespaces defined
 * in repr.h
 */
static void
sp_xml_ns_register_defaults ()
{
    static SPXMLNs defaults[7];

    defaults[0].uri = g_quark_from_static_string (SP_SODIPODI_NS_URI);
    defaults[0].prefix = g_quark_from_static_string ("sodipodi");
    defaults[0].next = &defaults[1];

    defaults[1].uri = g_quark_from_static_string (SP_XLINK_NS_URI);
    defaults[1].prefix = g_quark_from_static_string ("xlink");
    defaults[1].next = &defaults[2];

    defaults[2].uri = g_quark_from_static_string (SP_SVG_NS_URI);
    defaults[2].prefix = g_quark_from_static_string ("svg");
    defaults[2].next = &defaults[3];

    defaults[3].uri = g_quark_from_static_string (SP_INKSCAPE_NS_URI);
    defaults[3].prefix = g_quark_from_static_string ("inkscape");
    defaults[3].next = &defaults[4];

    defaults[4].uri = g_quark_from_static_string (SP_RDF_NS_URI);
    defaults[4].prefix = g_quark_from_static_string ("rdf");
    defaults[4].next = &defaults[5];

    defaults[5].uri = g_quark_from_static_string (SP_CC_NS_URI);
    defaults[5].prefix = g_quark_from_static_string ("cc");
    defaults[5].next = &defaults[6];

    defaults[6].uri = g_quark_from_static_string (SP_DC_NS_URI);
    defaults[6].prefix = g_quark_from_static_string ("dc");
    defaults[6].next = NULL;

    namespaces = &defaults[0];
}

char *
sp_xml_ns_auto_prefix (const char *uri)
{
    const char *start, *end;
    char *new_prefix;
    start = uri;
    while ((end = strpbrk (start, ":/"))) {
        start = end + 1;
    }
    end = start + strspn (start, "abcdefghijklmnopqrstuvwxyz");
    if (end == start) {
        start = "ns";
        end = start + 2;
    }
    new_prefix = g_strndup (start, end - start);
    if (sp_xml_ns_prefix_uri (new_prefix)) {
        char *temp;
        int counter=0;
        do {
            temp = g_strdup_printf ("%s%d", new_prefix, counter++);
        } while (sp_xml_ns_prefix_uri (temp));
        g_free (new_prefix);
        new_prefix = temp;
    }
    return new_prefix;
}

const gchar *
sp_xml_ns_uri_prefix (const gchar *uri, const gchar *suggested)
{
    SPXMLNs *iter;
    const char *prefix;

    if (!uri) return NULL;

    if (!namespaces) {
        sp_xml_ns_register_defaults ();
    }

    GQuark const key = g_quark_from_string (uri);
    prefix = NULL;
    for ( iter = namespaces ; iter ; iter = iter->next ) {
        if ( iter->uri == key ) {
            prefix = g_quark_to_string (iter->prefix);
            break;
        }
    }
    if (!prefix) {
        const char *new_prefix;
        SPXMLNs *ns;
        if (suggested) {
            new_prefix = suggested;
        } else {
            new_prefix = sp_xml_ns_auto_prefix (uri);
        }
        ns = g_new (SPXMLNs, 1);
        if (ns) {
            ns->uri = g_quark_from_string (uri);
            ns->prefix = g_quark_from_string (new_prefix);
            ns->next = namespaces;
            namespaces = ns;
            prefix = g_quark_to_string (ns->prefix);
        }
        if (!suggested) {
            g_free ((char *)new_prefix);
        }
    }
    return prefix;
}

const gchar *
sp_xml_ns_prefix_uri (const gchar *prefix)
{
    SPXMLNs *iter;
    const char *uri;

    if (!prefix) return NULL;

    if (!namespaces) {
        sp_xml_ns_register_defaults ();
    }

    GQuark const key = g_quark_from_string(prefix);
    uri = NULL;
    for ( iter = namespaces ; iter ; iter = iter->next ) {
        if ( iter->prefix == key ) {
            uri = g_quark_to_string (iter->uri);
            break;
        }
    }
    return uri;
}

/* SPXMLDocument */

SPXMLText *
sp_xml_document_createTextNode (SPXMLDocument *doc, const gchar *data)
{
    return new SPReprText(Inkscape::Util::SharedCStringPtr::copy(data));
}

/** Returns the first child of \a repr, or NULL if \a repr has no children (or if repr is itself
 *  NULL).
 *
 * \see sp_repr_next
 */
SPRepr *
sp_repr_children (SPRepr *repr)
{
    //This is not worth a warning
    // child of null is null
    //g_return_val_if_fail (repr != NULL, NULL);

    if (!repr)
        return NULL;
    return repr->firstChild();
}

/** Returns the next sibling of \a repr, or NULL if \a repr is the last sibling (or if repr is
 *  NULL).
 *
 *  \see sp_repr_prev
 *  \see sp_repr_parent
 *  \see sp_repr_children
 */
SPRepr *
sp_repr_next (SPRepr *repr)
{
    //This is not worth a warning
    // next of null is null
    //g_return_val_if_fail (repr != NULL, NULL);

    if (!repr)
        return NULL;
    return repr->next();
}

int sp_repr_attr_is_set (SPRepr * repr, const char * key)
{
    char * result;

    result = (char *) sp_repr_attr (repr, key);

    return (result != NULL);
}

double sp_repr_get_double_attribute (SPRepr * repr, const char * key, double def)
{
    char * result;

    g_return_val_if_fail (repr != NULL, def);
    g_return_val_if_fail (key != NULL, def);

    result = (char *) sp_repr_attr (repr, key);

    if (result == NULL) return def;

    return g_ascii_strtod (result, NULL);
}

int sp_repr_get_int_attribute (SPRepr * repr, const char * key, int def)
{
    char * result;

    g_return_val_if_fail (repr != NULL, def);
    g_return_val_if_fail (key != NULL, def);

    result = (char *) sp_repr_attr (repr, key);

    if (result == NULL) return def;

    return atoi (result);
}

/** 
 *  Works for different-parent objects, so long as they have a common ancestor. Return value:
 *    0    positions are equivalent
 *    1    first object's position is greater than the second
 *   -1    first object's position is less than the second
 */
int
sp_repr_compare_position(SPRepr *first, SPRepr *second)
{
    int p1, p2;
    if (sp_repr_parent(first) == sp_repr_parent(second)) {
        /* Basic case - first and second have same parent */
        p1 = sp_repr_position(first);
        p2 = sp_repr_position(second);
    } else {
        /* Special case - the two objects have different parents.  They
           could be in different groups or on different layers for
           instance. */

        // Find the lowest common ancestor (LCA)
        SPRepr *ancestor = LCA(first, second);
        g_assert(ancestor != NULL);

        if (ancestor == first) {
            return 1;
        } else if (ancestor == second) {
            return -1;
        } else {
            SPRepr const *to_first = AncetreFils(first, ancestor);
            SPRepr const *to_second = AncetreFils(second, ancestor);
            g_assert(sp_repr_parent(to_second) == sp_repr_parent(to_first));
            p1 = sp_repr_position(to_first);
            p2 = sp_repr_position(to_second);
        }
    }

    if (p1 > p2) return 1;
    if (p1 < p2) return -1;
    return 0;

    /* effic: Assuming that the parent--child relationship is consistent
       (i.e. that the parent really does contain first and second among
       its list of children), it should be equivalent to walk along the
       children and see which we encounter first (returning 0 iff first
       == second).
       
       Given that this function is used solely for sorting, we can use a
       similar approach to do the sort: gather the things to be sorted,
       into an STL vector (to allow random access and faster
       traversals).  Do a single pass of the parent's children; for each
       child, do a pass on whatever items in the vector we haven't yet
       encountered.  If the child is found, then swap it to the
       beginning of the yet-unencountered elements of the vector.
       Continue until no more than one remains unencountered.  --
       pjrm */
}

unsigned SPRepr::position() const {
    g_return_val_if_fail(_parent != NULL, 0);
    return _parent->_childPosition(*this);
}

unsigned SPRepr::_childPosition(SPRepr const &child) const {
    if (!_cached_positions_valid) {
        unsigned position=0;
        for ( SPRepr *sibling = _children->next() ;
              sibling ; sibling = sibling->next() )
        {
            sibling->_setCachedPosition(position);
            position++;
        }
        _cached_positions_valid = true;
    }
    return child._cachedPosition();
}

/** Returns the position of \a repr among its parent's children (starting with 0 for the first
 *  child).
 *
 *  \pre repr != NULL.
 *  \pre sp_repr_parent(repr) != NULL.
 *  \pre sp_repr_parent(repr)'s list of children includes \a repr.
 */
int
sp_repr_position(SPRepr const *repr)
{
    g_assert(repr != NULL);
    return repr->position();
}

int
sp_repr_n_children(SPRepr *repr)
{
    g_assert(repr != NULL);
    return (int)repr->childCount();
}

SPRepr *SPRepr::nthChild(unsigned index) {
    SPRepr *child = _children;
    for ( ; index > 0 && child ; child = child->next() ) {
        index--;
    }
    return child;
}

SPRepr *sp_repr_nth_child(SPRepr *repr, int n) {
    g_assert(repr != NULL);
    g_return_val_if_fail(n < 0, NULL);
    return repr->nthChild((unsigned)n);
}

SPRepr *SPRepr::lastChild() {
    SPRepr *child = _children;
    if (child) {
        while ( child->next() ) {
            child = child->next();
        }
    }
    return child;
}

void
sp_repr_append_child (SPRepr * repr, SPRepr * child)
{
    g_assert (repr != NULL);
    g_assert (child != NULL);

    return repr->appendChild(child);
}

void sp_repr_unparent (SPRepr * repr)
{
    SPRepr * parent;

    g_assert (repr != NULL);

    parent = sp_repr_parent (repr);
    g_assert (parent != NULL);

    sp_repr_remove_child (parent, repr);
}

/**
 * lookup child by \a key, \a value.
 */
SPRepr *
sp_repr_lookup_child (SPRepr       *repr,
                      const gchar *key,
                      const gchar *value)
{
    g_return_val_if_fail(repr != NULL, NULL);
    for ( SPRepr *child = repr->firstChild() ; child ; child = child->next() ) {
        gchar const *child_value = child->attribute(key);
        if ( child_value == value ||
             value && child_value && !strcmp(child_value, value) )
        {
            return child;
        }
    }
    return NULL;
}

/**
 *  \brief   Recursively find the SPRepr matching the given XML name.
 *  \return  A pointer to the matching SPRepr
 *  \param   repr    The SPRepr to start from
 *  \param   name    The desired XML name
 *  
 */
SPRepr *
sp_repr_lookup_name ( SPRepr *repr, gchar const *name, gint maxdepth )
{
    g_return_val_if_fail (repr != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);

    GQuark const quark = g_quark_from_string (name);

    if ( (GQuark)repr->code() == quark ) return repr;
    if ( maxdepth == 0 ) return NULL;

    // maxdepth == -1 means unlimited
    if ( maxdepth == -1 ) maxdepth = 0;

    SPRepr * found = NULL;
    for (SPRepr *child = repr->firstChild() ; child && !found; child = child->next() ) {
        found = sp_repr_lookup_name ( child, name, maxdepth-1 );
    }

    return found;
}

/**
 * Parses the boolean value of an attribute "key" in repr and sets val accordingly, or to FALSE if
 * the attr is not set.
 *
 * \return TRUE if the attr was set, FALSE otherwise.
 */
unsigned int
sp_repr_get_boolean (SPRepr *repr, const gchar *key, unsigned int *val)
{
    const gchar *v;

    g_return_val_if_fail (repr != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);
    g_return_val_if_fail (val != NULL, FALSE);

    v = sp_repr_attr (repr, key);

    if (v != NULL) {
        if (!g_strcasecmp (v, "true") ||
            !g_strcasecmp (v, "yes" ) ||
            !g_strcasecmp (v, "y"   ) ||
            (atoi (v) != 0)) {
            *val = TRUE;
        } else {
            *val = FALSE;
        }
        return TRUE;
    } else {
        *val = FALSE;
        return FALSE;
    }
}

unsigned int
sp_repr_get_int (SPRepr *repr, const gchar *key, int *val)
{
    const gchar *v;

    g_return_val_if_fail (repr != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);
    g_return_val_if_fail (val != NULL, FALSE);

    v = sp_repr_attr (repr, key);

    if (v != NULL) {
        *val = atoi (v);
        return TRUE;
    }

    return FALSE;
}

unsigned int
sp_repr_get_double (SPRepr *repr, const gchar *key, double *val)
{
    const gchar *v;

    g_return_val_if_fail (repr != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);
    g_return_val_if_fail (val != NULL, FALSE);

    v = sp_repr_attr (repr, key);

    if (v != NULL) {
        *val = g_ascii_strtod (v, NULL);
        return TRUE;
    }

    return FALSE;
}

unsigned int
sp_repr_set_boolean (SPRepr *repr, const gchar *key, unsigned int val)
{
    g_return_val_if_fail (repr != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);

    return sp_repr_set_attr (repr, key, (val) ? "true" : "false");
}

unsigned int
sp_repr_set_int (SPRepr *repr, const gchar *key, int val)
{
    gchar c[32];

    g_return_val_if_fail (repr != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);

    g_snprintf (c, 32, "%d", val);

    return sp_repr_set_attr (repr, key, c);
}

unsigned int
sp_repr_set_double (SPRepr *repr, const gchar *key, double val)
{
	Inkscape::SVGOStringStream os;
	
    g_return_val_if_fail (repr != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);

	os << val;
	
	return sp_repr_set_attr (repr, key, os.str().c_str());
}

unsigned int
sp_repr_set_double_default (SPRepr *repr, const gchar *key, double val, double def, double e)
{
    g_return_val_if_fail (repr != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);

    if (fabs (val - def) <= e) {
        return sp_repr_set_attr (repr, key, NULL);
    } else {
        return sp_repr_set_double (repr, key, val);
    }
}



/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:72
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
