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

#include "xml/xml-forward.h"
#include "xml/sp-repr.h"
#include "xml/sp-repr-doc.h"
#include "xml/sp-css-attr.h"

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

        void (* child_added) (SPRepr *repr, SPRepr *child, SPRepr *ref,
void *data);

Called once a child has been added.

        void (* child_removed) (SPRepr *repr, SPRepr *child, SPRepr
	*ref, void *data);

Called after a child is removed; ref is the child that used to precede
the removed child.

        void (* attr_changed) (SPRepr *repr, gchar const *key, gchar const *oldval, gchar const *newval, void *data);

Called after an attribute has been changed.

        void (* content_changed) (SPRepr *repr, gchar const *oldcontent,
gchar const *newcontent, void *data);

Called after an element's content has been changed.

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

     (but it's better to arrange for your caller to pass in the relevent
      document rather than assuming it's necessarily the active one and
      using SP_ACTIVE_DOCUMENT)

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
   parent->appendChild(child), and then use Inkscape::GC::release(child) to
   let go of it (the parent will hold it in memory).

Q: How do I destroy an SPRepr?
A: Just call "sp_repr_unparent" on it and release any references
   you may be retaining to it.  Any attached SPObjects will
   clean themselves up automatically, as will any children.

Q: What about listeners?
A: I have no idea yet...

Q: How do I add a namespace to a newly created document?
A: The current hack is in document.cpp:sp_document_create

 Kees Cook  2004-07-01
 updated MenTaLguY 2005-01-25

 */

/* SPXMLNs */

const  char *sp_xml_ns_uri_prefix(gchar const *uri, gchar const *suggested);
const  char *sp_xml_ns_prefix_uri(gchar const *prefix);

SPRepr *sp_repr_new(gchar const *name);
SPRepr *sp_repr_new_text(gchar const *content);
SPRepr *sp_repr_new_comment(gchar const *comment);

inline SPRepr *sp_repr_ref(SPRepr *repr) {
	return Inkscape::GC::anchor(repr);
}
inline SPRepr *sp_repr_unref(SPRepr *repr) {
	Inkscape::GC::release(repr);
	return NULL;
}
inline SPRepr *sp_repr_duplicate(SPRepr const *repr) {
	return repr->duplicate();
}

SPReprDoc *sp_repr_document_new(gchar const *rootname);
inline void sp_repr_document_ref(SPReprDoc *doc) {
	Inkscape::GC::anchor(doc);
}
inline void sp_repr_document_unref(SPReprDoc *doc) {
	Inkscape::GC::release(doc);
}

inline SPRepr *sp_repr_document_root(SPReprDoc const *doc) {
	return const_cast<SPRepr *>(doc->root());
}
inline SPReprDoc *sp_repr_document(SPRepr const *repr) {
	return const_cast<SPReprDoc *>(repr->document());
}


inline unsigned int sp_repr_document_merge(SPReprDoc *doc,
		                           SPReprDoc const *src,
					   char const *key)
{
	doc->root()->mergeFrom(src->root(), key);
	return true;
}

inline unsigned int sp_repr_merge(SPRepr *repr, SPRepr const *src, gchar const *key) {
	repr->mergeFrom(src, key);
	return true;
}

/* Contents */

inline const char *sp_repr_name(SPRepr const *repr) {
	return repr->name();
}
inline const char *sp_repr_content(SPRepr const *repr) {
	return repr->content();
}
inline const char *sp_repr_attr(SPRepr const *repr, gchar const *key) {
	return repr->attribute(key);
}
inline bool sp_repr_has_attr(SPRepr const *repr, gchar const *partial_name) {
	return repr->matchAttributeName(partial_name);
}

inline unsigned int sp_repr_set_content(SPRepr *repr, gchar const *content) {
	repr->setContent(content);
	return true;
}

inline unsigned int sp_repr_set_attr(SPRepr *repr, gchar const *key, gchar const *value, bool is_interactive=false) {
	repr->setAttribute(key, value, is_interactive);
	return true;
}

/* Tree */
inline SPRepr *sp_repr_parent(SPRepr const *repr) {
	return const_cast<SPRepr *>(repr->parent());
}
inline SPRepr *sp_repr_children(SPRepr *repr) {
	return ( repr ? repr->firstChild() : NULL );
}
inline SPRepr *sp_repr_next(SPRepr *repr) {
	return ( repr ? repr->next() : NULL );
}

inline unsigned int sp_repr_add_child(SPRepr *repr, SPRepr *child, SPRepr *ref)
{
	repr->addChild(child, ref);
	return true;
}
inline unsigned int sp_repr_remove_child(SPRepr *repr, SPRepr *child) {
	repr->removeChild(child);
	return true;
}

/* IO */

SPReprDoc *sp_repr_read_file(gchar const *filename, gchar const *default_ns);
SPReprDoc *sp_repr_read_mem(gchar const *buffer, int length, gchar const *default_ns);
void sp_repr_save_stream(SPReprDoc *doc, FILE *to_file, gchar const *default_ns=NULL, bool compress = false);
gboolean sp_repr_save_file(SPReprDoc *doc, gchar const *filename, gchar const *default_ns=NULL);

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

inline void sp_repr_unparent(SPRepr *repr) {
	SPRepr *parent=repr->parent();
	if (parent) {
		parent->removeChild(repr);
	}
}

inline bool sp_repr_attr_is_set(SPRepr *repr, gchar const *key) {
	return repr->attribute(key);
}

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

inline int sp_repr_position(SPRepr const *repr) {
	return repr->position();
}
inline void sp_repr_set_position_absolute(SPRepr *repr, int pos) {
	repr->setPosition(pos);
}
inline int sp_repr_n_children(SPRepr *repr) {
	return repr->childCount();
}
inline SPRepr *sp_repr_nth_child(SPRepr *repr, int n) {
	return repr->nthChild(n);
}
inline void sp_repr_append_child(SPRepr *repr, SPRepr *child) {
	repr->appendChild(child);
}

/* Searching */
SPRepr       *sp_repr_lookup_name   (SPRepr             *repr,
		                     gchar const        *name,
				     gint               maxdepth = -1 );
SPRepr       *sp_repr_lookup_child  (SPRepr    	        *repr,
				     gchar const        *key,
				     gchar const        *value);

inline unsigned int sp_repr_change_order (SPRepr *repr, SPRepr *child, SPRepr *ref) {
	repr->changeOrder(child, ref);
	return true;
}
                                                                                
SPReprDoc *sp_repr_document_new_list (GSList *reprs);
inline SPRepr *sp_repr_document_first_child(SPReprDoc const *doc) {
	return const_cast<SPRepr *>(doc->firstChild());
}

#endif
