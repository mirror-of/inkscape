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
#include <glib.h> // Needed for gchar.

#define SP_SODIPODI_NS_URI "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
#define SP_INKSCAPE_NS_URI "http://www.inkscape.org/namespaces/inkscape"
#define SP_XLINK_NS_URI "http://www.w3.org/1999/xlink"
#define SP_SVG_NS_URI "http://www.w3.org/2000/svg"
#define SP_RDF_NS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"

/*
 * NB! Unless explicitly stated all methods are noref/nostrcpy
 */

typedef struct _SPRepr SPXMLNode;
typedef struct _SPRepr SPXMLText;
typedef struct _SPRepr SPXMLElement;
typedef struct _SPReprAttr SPXMLAttribute;
typedef struct _SPReprDoc SPXMLDocument;
typedef struct _SPXMLNs SPXMLNs;


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

        void (* destroy) (SPRepr *repr, void * data);

Called when the repr is destroyed.

        unsigned int (* add_child) (SPRepr *repr, SPRepr *child, SPRepr
	*ref, void * data);

Called before a child is added; the handler can return FALSE to veto the
addition.  ref is the child after which the new child is to be added.

        void (* child_added) (SPRepr *repr, SPRepr *child, SPRepr *ref,
void * data);

Called once a child has been added.

        unsigned int (* remove_child) (SPRepr *repr, SPRepr *child,
SPRepr *ref, void * data);

Called before a child is to be removed; it may veto the removal by
returning FALSE.  ref is the child before the child to be removed.

        void (* child_removed) (SPRepr *repr, SPRepr *child, SPRepr
	*ref, void * data);

Called after a child is removed; ref is the child that used to precede
the removed child.

        unsigned int (* change_attr) (SPRepr *repr, const gchar *key,
const gchar *oldval, const gchar *newval, void * data);

For Element nodes.  Called before an attribute is changed; can veto by
returning FALSE.

        void (* attr_changed) (SPRepr *repr, const gchar *key, const
gchar *oldval, const gchar *newval, void * data);

Called after an attribute has been changed.

        unsigned int (* change_content) (SPRepr *repr, const gchar
	*oldcontent, const gchar *newcontent, void * data);

For Text nodes.  Called before an element's content is changed; can veto
by returning FALSE.

        void (* content_changed) (SPRepr *repr, const gchar *oldcontent,
const gchar *newcontent, void * data);

Called after an element's content has been changed.

        unsigned int (* change_order) (SPRepr *repr, SPRepr *child,
SPRepr *oldref, SPRepr *newref, void * data);

Called before a child of repr is rearranged in its list of children.
oldref is the child currently preceding the child; the child will be
moved to the position after newref.  Can veto by returning FALSE.

        void (* order_changed) (SPRepr *repr, SPRepr *child, SPRepr
	*oldref, SPRepr *newref, void * data);

Called once the child has been moved to its new position in the child
order.

NOTE!!!!! the veto callbacks are currently not useful because some
functions SPObjects register for callbacks have side-effects -- by the
time the veto was made, other callbacks might already have modified
things...

-- MenTaLguY, Dec 5, 2003

 */

/* SPXMLNs */

const  char *sp_xml_ns_uri_prefix (const gchar *uri, const gchar *suggested);
const  char *sp_xml_ns_prefix_uri (const gchar *prefix);

/* SPXMLDocument */

SPXMLText *sp_xml_document_createTextNode (SPXMLDocument *doc, const gchar *content);
SPXMLElement *sp_xml_document_createElement (SPXMLDocument *doc, const gchar *name);
SPXMLElement *sp_xml_document_createElementNS (SPXMLDocument *doc, const gchar *ns, const  char *qname);
/* SPXMLNode */

SPXMLDocument *sp_xml_node_get_Document (SPXMLNode *node);

/* SPXMLElement */

/*
 * SPRepr is opaque
 */

typedef struct _SPRepr SPRepr;

/* Create new repr & similar */

SPRepr *sp_repr_new (const gchar *name);
SPRepr *sp_repr_new_text (const gchar *content);
SPRepr *sp_repr_new_comment (const gchar *comment);
SPRepr *sp_repr_ref (SPRepr *repr);
SPRepr *sp_repr_unref (SPRepr *repr);
SPRepr *sp_repr_duplicate (const SPRepr *repr);

/* Documents - 1st step in migrating to real XML */

typedef struct _SPReprDoc SPReprDoc;

SPReprDoc * sp_repr_document_new (const gchar * rootname);
void sp_repr_document_ref (SPReprDoc * doc);
void sp_repr_document_unref (SPReprDoc * doc);

SPRepr *sp_repr_document_root (const SPReprDoc *doc);
SPReprDoc *sp_repr_document (const SPRepr *repr);


/* Documents Utility */
unsigned int sp_repr_document_merge (SPReprDoc *doc, const SPReprDoc *src, const  char *key);
unsigned int sp_repr_merge (SPRepr *repr, const SPRepr *src, const gchar *key);

/* Contents */

const  char *sp_repr_name (const SPRepr *repr);
const  char *sp_repr_content (const SPRepr *repr);
const  char *sp_repr_attr (const SPRepr *repr, const gchar *key);

/*
 * NB! signal handler may decide, that change is not allowed
 *
 * TRUE, if successful
 */

unsigned int sp_repr_set_content (SPRepr *repr, const gchar *content);
unsigned int sp_repr_set_attr (SPRepr *repr, const gchar *key, const gchar *value, bool is_interactive=false);

/* Tree */
SPRepr *sp_repr_parent (SPRepr *repr);
SPRepr *sp_repr_children (SPRepr *repr);
SPRepr *sp_repr_next (SPRepr *repr);

unsigned int sp_repr_add_child (SPRepr * repr, SPRepr * child, SPRepr * ref);
unsigned int sp_repr_remove_child (SPRepr * repr, SPRepr * child);
void sp_repr_write_stream (SPRepr * repr, FILE * file, int level, gboolean whitespace);

int sp_repr_n_children (SPRepr * repr);

/* IO */

SPReprDoc * sp_repr_read_file (const gchar * filename, const gchar *default_ns);
SPReprDoc * sp_repr_read_mem (const gchar * buffer, int length, const gchar *default_ns);
void sp_repr_save_stream (SPReprDoc * doc, FILE * to_file);
void sp_repr_save_file (SPReprDoc * doc, const gchar * filename);

void sp_repr_print (SPRepr * repr);

/* CSS stuff */

typedef struct _SPCSSAttr SPCSSAttr;

SPCSSAttr * sp_repr_css_attr_new (void);
void sp_repr_css_attr_unref (SPCSSAttr * css);
SPCSSAttr * sp_repr_css_attr (SPRepr * repr, const gchar * attr);
SPCSSAttr * sp_repr_css_attr_inherited (SPRepr * repr, const gchar * attr);

const gchar * sp_repr_css_property (SPCSSAttr * css, const gchar * name, const gchar * defval);
void sp_repr_css_set_property (SPCSSAttr * css, const gchar * name, const gchar * value);
double sp_repr_css_double_property (SPCSSAttr * css, const gchar * name, double defval);

void sp_repr_css_set (SPRepr * repr, SPCSSAttr * css, const gchar * key);
void sp_repr_css_change (SPRepr * repr, SPCSSAttr * css, const gchar * key);
void sp_repr_css_change_recursive (SPRepr * repr, SPCSSAttr * css, const gchar * key);

/* Utility finctions */

void sp_repr_unparent (SPRepr * repr);

int sp_repr_attr_is_set (SPRepr * repr, const gchar * key);

/* Convenience */
unsigned int sp_repr_get_boolean (SPRepr *repr, const  gchar *key, unsigned int *val);
unsigned int sp_repr_get_int (SPRepr *repr, const  gchar *key, int *val);
unsigned int sp_repr_get_double (SPRepr *repr, const  gchar *key, double *val);
unsigned int sp_repr_set_boolean (SPRepr *repr, const  gchar *key, unsigned int val);
unsigned int sp_repr_set_int (SPRepr *repr, const  gchar *key, int val);
unsigned int sp_repr_set_double (SPRepr *repr, const  gchar *key, double val);
/* Defaults */
unsigned int sp_repr_set_double_default (SPRepr *repr, const  gchar *key, double val, double def, double e);

/* Deprecated */
double sp_repr_get_double_attribute (SPRepr * repr, const gchar * key, double def);
int sp_repr_get_int_attribute (SPRepr * repr, const gchar * key, int def);

int sp_repr_compare_position (SPRepr * first, SPRepr * second);

int sp_repr_position (SPRepr * repr);
void sp_repr_set_position_absolute (SPRepr * repr, int pos);
void sp_repr_set_position_relative (SPRepr * repr, int pos);
int sp_repr_n_children (SPRepr * repr);
void sp_repr_append_child (SPRepr * repr, SPRepr * child);

const gchar * sp_repr_doc_attr (SPRepr * repr, const gchar * key);

SPRepr * sp_repr_duplicate_and_parent (SPRepr * repr);

const  gchar *sp_repr_attr_inherited (SPRepr *repr, const  gchar *key);
unsigned int sp_repr_set_attr_recursive (SPRepr *repr, const  gchar *key, const  gchar *value);

SPRepr       *sp_repr_lookup_child  (SPRepr    	        *repr,
				     const  gchar       *key,
				     const  gchar       *value);
unsigned int   p_repr_overwrite     (SPRepr             *repr,
				     const SPRepr       *src,
				     const  gchar       *key);

#endif
