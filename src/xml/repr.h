#ifndef __SP_REPR_H__
#define __SP_REPR_H__

/*
 * Fuzzy DOM-like tree implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stdio.h>
#include <glib/gtypes.h>
#include "gc-anchored.h"

#include <xml/xml-forward.h>

#define SP_SODIPODI_NS_URI "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
#define SP_INKSCAPE_NS_URI "http://www.inkscape.org/namespaces/inkscape"
#define SP_XLINK_NS_URI "http://www.w3.org/1999/xlink"
#define SP_SVG_NS_URI "http://www.w3.org/2000/svg"
#define SP_RDF_NS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define SP_CC_NS_URI "http://web.resource.org/cc/"
#define SP_DC_NS_URI "http://purl.org/dc/elements/1.1/"

/*
 * NB! Unless explicitly stated all methods are noref/nostrcpy
 */


/**

Though SPRepr provides "signals" for notification when individual nodes
change, there is no mechanism to receive notification for overall
document changes.

However, with the addition of the transactions code, it would not be
very hard to implement if you wanted it.

SPRepr itself doesn't use GObject signals at present -- SPReprs maintain
lists of SPReprEventVectors (added via sp_repr_add_listener), which are
used to specify callbacks when something changes.

Here are the current callbacks in an event vector (they may be NULL):

        void (* destroy) (SPRepr *repr, void *data);

Called when the repr is destroyed.

        unsigned int (* add_child) (SPRepr *repr, SPRepr *child, SPRepr *ref, void *data);

Called before a child is added; the handler can return FALSE to veto the
addition.  ref is the child after which the new child is to be added.

        void (* child_added) (SPRepr *repr, SPRepr *child, SPRepr *ref,
void *data);

Called once a child has been added.

        unsigned int (* remove_child) (SPRepr *repr, SPRepr *child,
SPRepr *ref, void *data);

Called before a child is to be removed; it may veto the removal by
returning FALSE.  ref is the child before the child to be removed.

        void (* child_removed) (SPRepr *repr, SPRepr *child, SPRepr
	*ref, void *data);

Called after a child is removed; ref is the child that used to precede
the removed child.

        unsigned int (* change_attr) (SPRepr *repr, gchar const *key,
gchar const *oldval, gchar const *newval, void *data);

For Element nodes.  Called before an attribute is changed; can veto by
returning FALSE.

        void (* attr_changed) (SPRepr *repr, gchar const *key, gchar const *oldval, gchar const *newval, void *data);

Called after an attribute has been changed.

        unsigned int (* change_content) (SPRepr *repr, gchar const
	*oldcontent, gchar const *newcontent, void *data);

For Text nodes.  Called before an element's content is changed; can veto
by returning FALSE.

        void (* content_changed) (SPRepr *repr, gchar const *oldcontent,
gchar const *newcontent, void *data);

Called after an element's content has been changed.

        unsigned int (* change_order) (SPRepr *repr, SPRepr *child,
SPRepr *oldref, SPRepr *newref, void *data);

Called before a child of repr is rearranged in its list of children.
oldref is the child currently preceding the child; the child will be
moved to the position after newref.  Can veto by returning FALSE.

        void (* order_changed) (SPRepr *repr, SPRepr *child, SPRepr
	*oldref, SPRepr *newref, void *data);

Called once the child has been moved to its new position in the child
order.

 */


/*

   SPRepr mini-FAQ

Since I'm not very familiar with this section of code but I need to use
it heavily for the RDF work, I'm going to answer various questions I run
into with my best-guess answers so others can follow along too.

Q: How do I find the root SPRepr?
A: If you have an SPDocument, use doc->rroot.  For example:
     SP_ACTIVE_DOCUMENT->rroot

Q: How do I find an SPRepr by unique key/value?
A: Use sp_repr_lookup_child

Q: How do I find an SPRepr by unique namespace name?
A: Use sp_repr_lookup_name

Q: How do I make an SPRepr namespace prefix constant in the application?
A: Add the XML namespace URL as a #define to repr.h at the top with the
   other SP_<NAMESPACE>_NS_URI #define's, and then in repr-util.cpp,
   in sp_xml_ns_register_defaults, bump "defaults" up in size one, and
   add another section.  Don't forget to increment the array offset and
   keep ".next" pointed to the next (if any) array entry.

Q: How do I create a new SPRepr?
A: Use "sp_repr_new*".  Then attach it to a parent somewhere with
   "sp_repr_append_child" and finally called "sp_repr_unref".

Q: How do I destroy an SPRepr?
A: Just call "sp_repr_unparent" on it and release any references
   you may be retaining to it.  Any attached SPObjects will
   clean themselves up automatically, as will any children.

Q: What about listeners?
A: I have no idea yet...

Q: How do I add a namespace to a newly created document?
A: The current hack is in document.cpp:sp_document_create

 Kees Cook  2004-07-01

 */

/* SPXMLNs */

const  char *sp_xml_ns_uri_prefix(gchar const *uri, gchar const *suggested);
const  char *sp_xml_ns_prefix_uri(gchar const *prefix);

/* SPXMLDocument */

SPXMLText *sp_xml_document_createTextNode(SPXMLDocument *doc, gchar const *content);
SPXMLElement *sp_xml_document_createElement(SPXMLDocument *doc, gchar const *name);
SPXMLElement *sp_xml_document_createElementNS(SPXMLDocument *doc, gchar const *ns, char const *qname);
/* SPXMLNode */

SPXMLDocument *sp_xml_node_get_Document(SPXMLNode *node);

/* SPXMLElement */

/*
 * SPRepr is opaque
 */

struct SPRepr;

/* Create new repr & similar */

SPRepr *sp_repr_new(gchar const *name);
SPRepr *sp_repr_new_text(gchar const *content);
SPRepr *sp_repr_new_comment(gchar const *comment);
SPRepr *sp_repr_ref(SPRepr *repr);
SPRepr *sp_repr_unref(SPRepr *repr);
SPRepr *sp_repr_duplicate(SPRepr const *repr);

/* Documents - 1st step in migrating to real XML */

SPReprDoc *sp_repr_document_new(gchar const *rootname);
void sp_repr_document_ref(SPReprDoc *doc);
void sp_repr_document_unref(SPReprDoc *doc);

SPRepr *sp_repr_document_root(SPReprDoc const *doc);
SPReprDoc *sp_repr_document(SPRepr const *repr);


/* Documents Utility */
unsigned int sp_repr_document_merge(SPReprDoc *doc, SPReprDoc const *src, char const *key);
unsigned int sp_repr_merge(SPRepr *repr, SPRepr const *src, gchar const *key);

/* Contents */

const  char *sp_repr_name(SPRepr const *repr);
const  char *sp_repr_content(SPRepr const *repr);
const  char *sp_repr_attr(SPRepr const *repr, gchar const *key);
bool sp_repr_has_attr(SPRepr const *repr, gchar const *partial_name);

/*
 * NB! signal handler may decide, that change is not allowed
 *
 * TRUE, if successful
 */

unsigned int sp_repr_set_content(SPRepr *repr, gchar const *content);
unsigned int sp_repr_set_attr(SPRepr *repr, gchar const *key, gchar const *value, bool is_interactive=false);

/* Tree */
SPRepr *sp_repr_parent(SPRepr const *repr);
SPRepr *sp_repr_children(SPRepr *repr);
SPRepr *sp_repr_next(SPRepr *repr);

namespace Inkscape {
namespace Traits {

template <typename T> struct TreeIterator;

template <>
struct TreeIterator<SPRepr *> {
	typedef SPRepr *Node;

	static bool is_null(SPRepr *repr) { return repr == NULL; }
	static SPRepr *null() { return NULL; }

	static SPRepr *node(SPRepr *repr) { return repr; }

	static SPRepr *first_child(SPRepr *repr) {
		return sp_repr_children(repr);
	}
	static SPRepr *parent(SPRepr *repr) {
		return sp_repr_parent(repr);
	}
	static SPRepr *next(SPRepr *repr) {
		return sp_repr_next(repr);
	}
};

}
}

unsigned int sp_repr_add_child(SPRepr *repr, SPRepr *child, SPRepr *ref);
unsigned int sp_repr_remove_child(SPRepr *repr, SPRepr *child);

int sp_repr_n_children(SPRepr *repr);
SPRepr *sp_repr_nth_child(SPRepr *repr, int n);
int sp_repr_pos_of(SPRepr *repr);

/* IO */

SPReprDoc *sp_repr_read_file(gchar const *filename, gchar const *default_ns);
SPReprDoc *sp_repr_read_mem(gchar const *buffer, int length, gchar const *default_ns);
void sp_repr_save_stream(SPReprDoc *doc, FILE *to_file);
gboolean sp_repr_save_file(SPReprDoc *doc, gchar const *filename);

void sp_repr_print(SPRepr *repr);

/* CSS stuff */

SPCSSAttr *sp_repr_css_attr_new(void);
void sp_repr_css_attr_unref(SPCSSAttr *css);
SPCSSAttr *sp_repr_css_attr(SPRepr *repr, gchar const *attr);
SPCSSAttr *sp_repr_css_attr_inherited(SPRepr *repr, gchar const *attr);

gchar const *sp_repr_css_property(SPCSSAttr *css, gchar const *name, gchar const *defval);
void sp_repr_css_set_property(SPCSSAttr *css, gchar const *name, gchar const *value);
double sp_repr_css_double_property(SPCSSAttr *css, gchar const *name, double defval);

void sp_repr_css_set(SPRepr *repr, SPCSSAttr *css, gchar const *key);
void sp_repr_css_merge (SPCSSAttr * dst, SPCSSAttr * src);
void sp_repr_css_attr_add_from_string (SPCSSAttr *css, const gchar *data);
void sp_repr_css_change(SPRepr *repr, SPCSSAttr *css, gchar const *key);
void sp_repr_css_change_recursive(SPRepr *repr, SPCSSAttr *css, gchar const *key);

void sp_repr_css_print (SPCSSAttr * css);

/* Utility finctions */

void sp_repr_unparent(SPRepr *repr);

int sp_repr_attr_is_set(SPRepr *repr, gchar const *key);

/* Convenience */
unsigned int sp_repr_get_boolean(SPRepr *repr, gchar const *key, unsigned int *val);
unsigned int sp_repr_get_int(SPRepr *repr, gchar const *key, int *val);
unsigned int sp_repr_get_double(SPRepr *repr, gchar const *key, double *val);
unsigned int sp_repr_set_boolean(SPRepr *repr, gchar const *key, unsigned int val);
unsigned int sp_repr_set_int(SPRepr *repr, gchar const *key, int val);
unsigned int sp_repr_set_double(SPRepr *repr, gchar const *key, double val);
/* Defaults */
unsigned int sp_repr_set_double_default(SPRepr *repr, gchar const *key, double val, double def, double e);

/* Deprecated */
double sp_repr_get_double_attribute(SPRepr *repr, gchar const *key, double def);
int sp_repr_get_int_attribute(SPRepr *repr, gchar const *key, int def);
/* End Deprecated? */

int sp_repr_compare_position(SPRepr *first, SPRepr *second);

int sp_repr_position(SPRepr const *repr);
void sp_repr_set_position_absolute(SPRepr *repr, int pos);
int sp_repr_n_children(SPRepr *repr);
void sp_repr_append_child(SPRepr *repr, SPRepr *child);

gchar const *sp_repr_doc_attr(SPRepr *repr, gchar const *key);

SPRepr *sp_repr_duplicate_and_parent(SPRepr *repr);

gchar const *sp_repr_attr_inherited(SPRepr *repr, gchar const *key);
unsigned int sp_repr_set_attr_recursive(SPRepr *repr, gchar const *key, gchar const *value);

/* Searching */
SPRepr       *sp_repr_lookup_name   (SPRepr             *repr,
		                     gchar const        *name,
				     gint               maxdepth = -1 );
SPRepr       *sp_repr_lookup_child  (SPRepr    	        *repr,
				     gchar const        *key,
				     gchar const        *value);

unsigned int   p_repr_overwrite     (SPRepr             *repr,
				     SPRepr const       *src,
				     gchar const        *key);

#endif
