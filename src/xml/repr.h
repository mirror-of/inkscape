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

#define SP_SODIPODI_NS_URI "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
#define SP_XLINK_NS_URI "http://www.w3.org/1999/xlink"
#define SP_SVG_NS_URI "http://www.w3.org/2000/svg"

/*
 * NB! Unless explicitly stated all methods are noref/nostrcpy
 */

typedef struct _SPRepr SPXMLNode;
typedef struct _SPRepr SPXMLText;
typedef struct _SPRepr SPXMLElement;
typedef struct _SPReprAttr SPXMLAttribute;
typedef struct _SPReprDoc SPXMLDocument;
typedef struct _SPXMLNs SPXMLNs;

/* SPXMLNs */

const unsigned char *sp_xml_ns_uri_prefix (const unsigned char *uri, const unsigned char *suggested);
const unsigned char *sp_xml_ns_prefix_uri (const unsigned char *prefix);

/* SPXMLDocument */

SPXMLText *sp_xml_document_createTextNode (SPXMLDocument *doc, const unsigned char *content);
SPXMLElement *sp_xml_document_createElement (SPXMLDocument *doc, const unsigned char *name);
SPXMLElement *sp_xml_document_createElementNS (SPXMLDocument *doc, const unsigned char *ns, const unsigned char *qname);

/* SPXMLNode */

SPXMLDocument *sp_xml_node_get_Document (SPXMLNode *node);

/* SPXMLElement */

void sp_xml_element_setAttributeNS (SPXMLElement *element, const unsigned char *ns, const unsigned char *qname, const unsigned char *val);

/*
 * SPRepr is opaque
 */

typedef struct _SPRepr SPRepr;

/* Create new repr & similar */

SPRepr *sp_repr_new (const unsigned char *name);
SPRepr *sp_repr_new_text (const unsigned char *content);
SPRepr *sp_repr_ref (SPRepr *repr);
SPRepr *sp_repr_unref (SPRepr *repr);
SPRepr *sp_repr_duplicate (const SPRepr *repr);

/* Documents - 1st step in migrating to real XML */

typedef struct _SPReprDoc SPReprDoc;

SPReprDoc * sp_repr_document_new (const char * rootname);
void sp_repr_document_ref (SPReprDoc * doc);
void sp_repr_document_unref (SPReprDoc * doc);

SPRepr *sp_repr_document_root (const SPReprDoc *doc);
SPReprDoc *sp_repr_document (const SPRepr *repr);

/* Documents Utility */
unsigned int sp_repr_document_merge (SPReprDoc *doc, const SPReprDoc *src, const unsigned char *key);
unsigned int sp_repr_merge (SPRepr *repr, const SPRepr *src, const unsigned char *key);

/* Contents */

const unsigned char *sp_repr_name (const SPRepr *repr);
const unsigned char *sp_repr_content (const SPRepr *repr);
const unsigned char *sp_repr_attr (const SPRepr *repr, const unsigned char *key);

/*
 * NB! signal handler may decide, that change is not allowed
 *
 * TRUE, if successful
 */

unsigned int sp_repr_set_content (SPRepr *repr, const unsigned char *content);
unsigned int sp_repr_set_attr (SPRepr *repr, const unsigned char *key, const unsigned char *value);

#if 0
/*
 * Returns list of attribute strings
 * List should be freed by caller, but attributes not
 */
GList * sp_repr_attributes (SPRepr * repr);
#endif

#if 0
void sp_repr_set_data (SPRepr * repr, void * data);
void * sp_repr_data (SPRepr * repr);
#endif

/* Tree */
SPRepr *sp_repr_parent (SPRepr *repr);
SPRepr *sp_repr_children (SPRepr *repr);
SPRepr *sp_repr_next (SPRepr *repr);

unsigned int sp_repr_add_child (SPRepr * repr, SPRepr * child, SPRepr * ref);
unsigned int sp_repr_remove_child (SPRepr * repr, SPRepr * child);
void sp_repr_write_stream (SPRepr * repr, FILE * file, int level);

#if 0
const GList *sp_repr_get_children_list (SPRepr * repr);
#endif
int sp_repr_n_children (SPRepr * repr);

/* IO */

SPReprDoc * sp_repr_read_file (const char * filename, const char *default_ns);
SPReprDoc * sp_repr_read_mem (const char * buffer, int length, const char *default_ns);
void sp_repr_save_stream (SPReprDoc * doc, FILE * to_file);
void sp_repr_save_file (SPReprDoc * doc, const char * filename);

void sp_repr_print (SPRepr * repr);

/* CSS stuff */

typedef struct _SPCSSAttr SPCSSAttr;

SPCSSAttr * sp_repr_css_attr_new (void);
void sp_repr_css_attr_unref (SPCSSAttr * css);
SPCSSAttr * sp_repr_css_attr (SPRepr * repr, const char * attr);
SPCSSAttr * sp_repr_css_attr_inherited (SPRepr * repr, const char * attr);

const char * sp_repr_css_property (SPCSSAttr * css, const char * name, const char * defval);
void sp_repr_css_set_property (SPCSSAttr * css, const char * name, const char * value);
double sp_repr_css_double_property (SPCSSAttr * css, const char * name, double defval);

void sp_repr_css_set (SPRepr * repr, SPCSSAttr * css, const char * key);
void sp_repr_css_change (SPRepr * repr, SPCSSAttr * css, const char * key);
void sp_repr_css_change_recursive (SPRepr * repr, SPCSSAttr * css, const char * key);

/* Utility finctions */

void sp_repr_unparent (SPRepr * repr);

int sp_repr_attr_is_set (SPRepr * repr, const char * key);

/* Convenience */
unsigned int sp_repr_get_boolean (SPRepr *repr, const unsigned char *key, unsigned int *val);
unsigned int sp_repr_get_int (SPRepr *repr, const unsigned char *key, int *val);
unsigned int sp_repr_get_double (SPRepr *repr, const unsigned char *key, double *val);
unsigned int sp_repr_set_boolean (SPRepr *repr, const unsigned char *key, unsigned int val);
unsigned int sp_repr_set_int (SPRepr *repr, const unsigned char *key, int val);
unsigned int sp_repr_set_double (SPRepr *repr, const unsigned char *key, double val);
/* Defaults */
unsigned int sp_repr_set_double_default (SPRepr *repr, const unsigned char *key, double val, double def, double e);

/* Deprecated */
double sp_repr_get_double_attribute (SPRepr * repr, const char * key, double def);
int sp_repr_get_int_attribute (SPRepr * repr, const char * key, int def);
unsigned int sp_repr_set_double_attribute (SPRepr * repr, const char * key, double value);
unsigned int sp_repr_set_int_attribute (SPRepr * repr, const char * key, int value);

int sp_repr_compare_position (SPRepr * first, SPRepr * second);

int sp_repr_position (SPRepr * repr);
void sp_repr_set_position_absolute (SPRepr * repr, int pos);
void sp_repr_set_position_relative (SPRepr * repr, int pos);
int sp_repr_n_children (SPRepr * repr);
void sp_repr_append_child (SPRepr * repr, SPRepr * child);

const char * sp_repr_doc_attr (SPRepr * repr, const char * key);

SPRepr * sp_repr_duplicate_and_parent (SPRepr * repr);

void sp_repr_remove_signals (SPRepr * repr);

const unsigned char *sp_repr_attr_inherited (SPRepr *repr, const unsigned char *key);
unsigned int sp_repr_set_attr_recursive (SPRepr *repr, const unsigned char *key, const unsigned char *value);

SPRepr       *sp_repr_lookup_child  (SPRepr    	        *repr,
				     const unsigned char       *key,
				     const unsigned char       *value);
unsigned int      sp_repr_overwrite     (SPRepr             *repr,
				     const SPRepr       *src,
				     const unsigned char       *key);

#endif
