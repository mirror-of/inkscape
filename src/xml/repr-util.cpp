#define __SP_REPR_UTIL_C__

/*
 * Miscellaneous helpers for reprs
 *
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

#include "repr-private.h"
#include "../svg/stringstream.h"

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
        val = fabs (val);
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
 * There are the default XML namespaces to use for the URIs defined
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
    unsigned int key;
    SPXMLNs *iter;
    const char *prefix;

    if (!uri) return NULL;

    if (!namespaces) {
        sp_xml_ns_register_defaults ();
    }

    key = g_quark_from_string (uri);
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
    unsigned int key;
    SPXMLNs *iter;
    const char *uri;

    if (!prefix) return NULL;

    if (!namespaces) {
        sp_xml_ns_register_defaults ();
    }

    key = g_quark_from_string (prefix);
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
    SPXMLText *text;

    text = sp_repr_new ("text");
    text->type = SP_XML_TEXT_NODE;
    sp_repr_set_content (text, data);

    return text;
}

SPXMLElement *
sp_xml_document_createElement (SPXMLDocument *doc, const gchar *name)
{
    return sp_repr_new (name);
}

SPXMLElement *
sp_xml_document_createElementNS (SPXMLDocument *doc, const gchar *ns, const gchar *qname)
{
    if (!strncmp (qname, "svg:", 4)) qname += 4;

    return sp_repr_new (qname);
}

/* SPXMLNode */

SPXMLDocument *
sp_xml_node_get_Document (SPXMLNode *node)
{
    g_warning ("sp_xml_node_get_Document: unimplemented");

    return NULL;
}

/* SPXMLElement */

SPRepr *
sp_repr_children (SPRepr *repr)
{
    g_return_val_if_fail (repr != NULL, NULL);

    return repr->children;
}

SPRepr *
sp_repr_next (SPRepr *repr)
{
    g_return_val_if_fail (repr != NULL, NULL);

    return repr->next;
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

const char *
sp_repr_doc_attr (SPRepr * repr, const char * key)
{
    SPRepr * p;

    p = sp_repr_parent (repr);

    while (p != NULL) {
        repr = p;
        p = sp_repr_parent (p);
    }

    return sp_repr_attr (repr, key);
}

int
sp_repr_compare_position(SPRepr *first, SPRepr *second)
{
    g_assert( sp_repr_parent(first) == sp_repr_parent(second) );

    int const p1 = sp_repr_position(first);
    int const p2 = sp_repr_position(second);

    if (p1 > p2) return 1;
    if (p1 < p2) return -1;
    return 0;

    /* effic: Assuming that the parent--child relationship is consistent (i.e. that the parent
       really does contain first and second among its list of children), it should be equivalent to
       walk along the children and see which we encounter first (returning 0 iff first == second).

       Given that this function is used solely for sorting, we can use a similar approach to do the
       sort: gather the things to be sorted, into an STL vector (to allow random access and faster
       traversals).  Do a single pass of the parent's children; for each child, do a pass on
       whatever items in the vector we haven't yet encountered.  If the child is found, then swap
       it to the beginning of the yet-unencountered elements of the vector.  Continue until no more
       than one remains unencountered.  -- pjrm */
}


/** Returns the position of \a repr among its parent's children (starting with 0 for the first
 *  child).
 *
 *  Requires: repr != NULL
 *         && sp_repr_parent(repr) != NULL
 *         && sp_repr_parent(repr)'s list of children includes \a repr.
 */
int
sp_repr_position(SPRepr const *repr)
{
    g_assert(repr != NULL);
    SPRepr const *parent = sp_repr_parent(repr);
    g_assert(parent != NULL);

    int pos = 0;
    for (SPRepr *sibling = parent->children; sibling != NULL; sibling = sibling->next) {
        if ( repr == sibling ) {
            return pos;
        }
        ++pos;
    }

    g_assert_not_reached();
    return -1;
}

int
sp_repr_n_children(SPRepr *repr)
{
    SPRepr * child;
    int n;

    g_assert (repr != NULL);

    n = 0;
    for (child = repr->children; child != NULL; child = child->next) n++;

    return n;
}

void
sp_repr_append_child (SPRepr * repr, SPRepr * child)
{
    SPRepr * ref;

    g_assert (repr != NULL);
    g_assert (child != NULL);

    ref = NULL;
    if (repr->children) {
        ref = repr->children;
        while (ref->next) ref = ref->next;
    }

    sp_repr_add_child (repr, child, ref);
}

void sp_repr_unparent (SPRepr * repr)
{
    SPRepr * parent;

    g_assert (repr != NULL);

    parent = sp_repr_parent (repr);
    g_assert (parent != NULL);

    sp_repr_remove_child (parent, repr);
}

SPRepr * sp_repr_duplicate_and_parent (SPRepr * repr)
{
    SPRepr * parent, * new_repr;

    g_assert (repr != NULL);

    parent = sp_repr_parent (repr);
    g_assert (parent != NULL);

    new_repr = sp_repr_duplicate (repr);
    sp_repr_append_child (parent, new_repr);
    sp_repr_unref (new_repr);

    return new_repr;
}

const gchar *
sp_repr_attr_inherited (SPRepr *repr, const gchar *key)
{
    SPRepr *current;
    const char *val;

    g_assert (repr != NULL);
    g_assert (key != NULL);

    for (current = repr; current != NULL; current = sp_repr_parent (current)) {
        val = sp_repr_attr (current, key);
        if (val != NULL)
            return val;
    }
    return NULL;
}

unsigned int
sp_repr_set_attr_recursive (SPRepr *repr, const gchar *key, const gchar *value)
{
    SPRepr *child;

    if (!sp_repr_set_attr (repr, key, value)) return FALSE;

    for (child = repr->children; child != NULL; child = child->next) {
        sp_repr_set_attr (child, key, NULL);
    }

    return TRUE;
}

/**
 * sp_repr_lookup_child:
 * @repr: 
 * @key: 
 * @value: 
 *
 * lookup child by (@key, @value)
 */
SPRepr *
sp_repr_lookup_child (SPRepr       *repr,
                      const gchar *key,
                      const gchar *value)
{
    SPRepr *child;
    SPReprAttr *attr;
    unsigned int quark;

    g_return_val_if_fail (repr != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);
    g_return_val_if_fail (value != NULL, NULL);

    quark = g_quark_from_string (key);

    /* Fixme: we should use hash table for faster lookup? */
    
    for (child = repr->children; child != NULL; child = child->next) {
        for (attr = child->attributes; attr != NULL; attr = attr->next) {
            if ( (static_cast< unsigned int > (attr->key) == quark) && !strcmp (attr->value, value))
                return child;
        }
    }

    return NULL;
}

/**
Parses the boolean value of an attribute "key" in repr and sets val accordingly, or to FALSE if the attr is not set. Returns TRUE if the attr was set, FALSE otherwise.
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
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
