#define __SP_REPR_IO_C__

/*
 * Dirty DOM-like  tree
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <glib.h>

#include "repr-private.h"

static const gchar *sp_svg_doctype_str =
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"\n"
"\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n";

static SPReprDoc *sp_repr_do_read (xmlDocPtr doc, const gchar *default_ns);
static SPRepr * sp_repr_svg_read_node (SPXMLDocument *doc, xmlNodePtr node, const gchar *default_ns, GHashTable *prefix_map);
static void sp_repr_set_xmlns_attr (const gchar *prefix, const gchar *uri, SPRepr *repr);
static gint sp_repr_qualified_name (gchar *p, gint len, xmlNsPtr ns, const xmlChar *name, const gchar *default_ns, GHashTable *prefix_map);

#ifdef HAVE_LIBWMF
static xmlDocPtr sp_wmf_convert (const char * file_name);
static char * sp_wmf_image_name (void * context);
#endif /* HAVE_LIBWMF */

/**
 * Reads XML from a file, including WMF files, and returns the SPReprDoc.
 * The default namespace can also be specified, if desired.
 */
SPReprDoc *
sp_repr_read_file (const gchar * filename, const gchar *default_ns)
{
	xmlDocPtr doc;
	SPReprDoc * rdoc;

	xmlSubstituteEntitiesDefault(1);

	g_return_val_if_fail (filename != NULL, NULL);

	// TODO: bulia, please look over
	gsize bytesRead = 0;
	gsize bytesWritten = 0;
	GError* error = NULL;
	gchar* localFilename = g_filename_from_utf8 ( filename,
												  -1,
												  &bytesRead,
												  &bytesWritten,
												  &error);

#ifdef HAVE_LIBWMF
	if (strlen (localFilename) > 4) {
		if ( (strcmp (localFilename + strlen (localFilename) - 4,".wmf") == 0)
		  || (strcmp (localFilename + strlen (localFilename) - 4,".WMF") == 0))
			doc = sp_wmf_convert (localFilename);
		else
			doc = xmlParseFile (localFilename);
	}
	else {
		doc = xmlParseFile (localFilename);
	}
#else /* HAVE_LIBWMF */
	doc = xmlParseFile (localFilename);
#endif /* HAVE_LIBWMF */

	rdoc = sp_repr_do_read (doc, default_ns);
	if (doc) {
		xmlFreeDoc (doc);
	}

	if ( localFilename != NULL )
	{
		g_free (localFilename);
	}

	return rdoc;
}

/**
 * Reads and parses XML from a buffer, returning it as an SPReprDoc
 */
SPReprDoc *
sp_repr_read_mem (const gchar * buffer, gint length, const gchar *default_ns)
{
	xmlDocPtr doc;
	SPReprDoc * rdoc;

	xmlSubstituteEntitiesDefault(1);

	g_return_val_if_fail (buffer != NULL, NULL);

	doc = xmlParseMemory ((gchar *) buffer, length);

	rdoc = sp_repr_do_read (doc, default_ns);
	if (doc) {
		xmlFreeDoc (doc);
	}
	return rdoc;
}

/**
 * Reads in a XML file to create a SPReprDoc
 */
SPReprDoc *
sp_repr_do_read (xmlDocPtr doc, const gchar *default_ns)
{
	SPReprDoc * rdoc;
	SPRepr * repr;
	GHashTable * prefix_map;
	xmlNodePtr node;

	if (doc == NULL) return NULL;
	node = xmlDocGetRootElement (doc);
	if (node == NULL) return NULL;
	rdoc = sp_repr_document_new ("void");


	prefix_map = g_hash_table_new (g_str_hash, g_str_equal);

	repr = NULL;
	for (node = xmlDocGetRootElement(doc); node != NULL; node = node->next) {
		if (node->type == XML_ELEMENT_NODE) {
			repr = sp_repr_svg_read_node (rdoc, node, default_ns, prefix_map);
			break;
		}
	}

	if (repr != NULL) {
		if (default_ns) {
			sp_repr_set_attr (repr, "xmlns", default_ns);
		}
		g_hash_table_foreach (prefix_map, (GHFunc)sp_repr_set_xmlns_attr, repr);
		/* always include Sodipodi and Inkscape namespaces */
		sp_repr_set_xmlns_attr (sp_xml_ns_uri_prefix (SP_SODIPODI_NS_URI, "sodipodi"), SP_SODIPODI_NS_URI, repr);
		sp_repr_set_xmlns_attr (sp_xml_ns_uri_prefix (SP_INKSCAPE_NS_URI, "inkscape"), SP_INKSCAPE_NS_URI, repr);

		if (!strcmp (sp_repr_name (repr), "svg") && default_ns && !strcmp (default_ns, SP_SVG_NS_URI)) {
			
			sp_repr_set_attr ((SPRepr *) rdoc, "doctype", sp_svg_doctype_str);
			/* always include XLink namespace */
			sp_repr_set_xmlns_attr (sp_xml_ns_uri_prefix (SP_XLINK_NS_URI, "xlink"), SP_XLINK_NS_URI, repr);
		}

		sp_repr_document_set_root (rdoc, repr);
		sp_repr_unref (repr);
	}
	g_hash_table_destroy (prefix_map);

	return rdoc;
}

void
sp_repr_set_xmlns_attr (const gchar *prefix, const gchar *uri, SPRepr *repr)
{
	gchar *name;
	name = g_strconcat ("xmlns:", prefix, NULL);
	sp_repr_set_attr (repr, name, uri);
	g_free (name);
}

gint
sp_repr_qualified_name (gchar *p, gint len, xmlNsPtr ns, const xmlChar *name, const gchar *default_ns, GHashTable *prefix_map)
{
	const xmlChar *prefix;
	if (ns) {
		if (!ns->href) {
			prefix = ns->prefix;
		} else if (default_ns && !strcmp ((gchar*)ns->href, default_ns)) {
			prefix = NULL;
		} else {
			prefix = (xmlChar*)sp_xml_ns_uri_prefix ((gchar*)ns->href, (char*)ns->prefix);
			g_hash_table_insert (prefix_map, (gpointer)prefix, (gpointer)ns->href);
		}
	} else {
		prefix = NULL;
	}

	if (prefix) {
		return g_snprintf (p, len, "%s:%s", (gchar*)prefix, name);
	} else {
		return g_snprintf (p, len, "%s", name);
	}
}

static SPRepr *
sp_repr_svg_read_node (SPXMLDocument *doc, xmlNodePtr node, const gchar *default_ns, GHashTable *prefix_map)
{
	SPRepr *repr, *crepr;
	xmlAttrPtr prop;
	xmlNodePtr child;
	gchar c[256];

#ifdef SP_REPR_IO_VERBOSE
	g_print ("Node type %d, name %s, contains |%s|\n", node->type, node->name, node->content);
#endif

	if (node->type == XML_TEXT_NODE || node->type == XML_CDATA_SECTION_NODE) {

		if (node->content == NULL || *(node->content) == '\0')
			return NULL; // empty text node

		bool preserve = (xmlNodeGetSpacePreserve (node) == 1);

		xmlChar *p;
		for (p = node->content; *p && g_ascii_isspace (*p) && !preserve; p++)
			; // skip all whitespace

		if (!(*p)) { // this is an all-whitespace node, and preserve == default
			return NULL; // we do not preserve all-whitespace nodes unless we are asked to
		}

		gchar *s = g_strdup ((const gchar*) node->content); // otherwise, use the node verbatim
		SPRepr *rdoc = sp_xml_document_createTextNode (doc, s);
		g_free (s);
		return rdoc;
	}

	if (node->type == XML_COMMENT_NODE) {
		return sp_repr_new_comment((const gchar *)node->content);
	}

	if (node->type == XML_ENTITY_DECL) return NULL;

	sp_repr_qualified_name (c, 256, node->ns, node->name, default_ns, prefix_map);
	repr = sp_repr_new (c);

	for (prop = node->properties; prop != NULL; prop = prop->next) {
		if (prop->children) {
			sp_repr_qualified_name (c, 256, prop->ns, prop->name, default_ns, prefix_map);
			sp_repr_set_attr (repr, c, (gchar*)prop->children->content);
		}
	}

	if (node->content) {
		sp_repr_set_content (repr, (gchar*)node->content);
	}

	child = node->xmlChildrenNode;
	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		crepr = sp_repr_svg_read_node (doc, child, default_ns, prefix_map);
		if (crepr) {
			sp_repr_append_child (repr, crepr);
			sp_repr_unref (crepr);
		}
	}

	return repr;
}

void
sp_repr_save_stream (SPReprDoc *doc, FILE *fp)
{
	SPRepr *repr;
	const gchar *str;

	/* fixme: do this The Right Way */

	fputs ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n", fp);

	str = sp_repr_attr ((SPRepr *) doc, "doctype");
	if (str) fputs (str, fp);

	repr = sp_repr_document_root (doc);

	sp_repr_write_stream (repr, fp, 0, TRUE);
}

void
sp_repr_save_file (SPReprDoc *doc, const gchar *filename)
{
	FILE * file;

	g_return_if_fail (filename != NULL);

	// TODO: bulia, please look over
	gsize bytesRead = 0;
	gsize bytesWritten = 0;
	GError* error = NULL;
	gchar* localFilename = g_filename_from_utf8 ( filename,
												  -1,
												  &bytesRead,
												  &bytesWritten,
												  &error);

	file = fopen (localFilename, "w");
	g_return_if_fail (file != NULL);

	sp_repr_save_stream (doc, file);

	fclose (file);

	if ( localFilename != NULL )
	{
		g_free (localFilename);
	}

	return;
}

void
sp_repr_print (SPRepr * repr)
{
	sp_repr_write_stream (repr, stdout, 0, TRUE);

	return;
}

/* (No doubt this function already exists elsewhere.) */
static void
repr_quote_write (FILE * file, const gchar * val)
{
	for (; *val != '\0'; val++) {
		switch (*val) {
		case '"': fputs ("&quot;", file); break;
		case '&': fputs ("&amp;", file); break;
		case '<': fputs ("&lt;", file); break;
		case '>': fputs ("&gt;", file); break;
		default: putc (*val, file); break;
		}
	}
}

void
sp_repr_write_stream (SPRepr * repr, FILE * file, gint indent_level,
                      gboolean add_whitespace)
{
	SPReprAttr *attr;
	SPRepr *child;
	gboolean loose;
	gint i;

	g_return_if_fail (repr != NULL);
	g_return_if_fail (file != NULL);

	if ( indent_level > 16 ) {
		indent_level = 16;
	}

	if (add_whitespace) {
		for ( i = 0 ; i < indent_level ; i++ ) {
			fputs ("  ", file);
		}
	}

	fprintf (file, "<%s", sp_repr_name (repr));

	// if this is a <text> element, suppress formatting whitespace
	// for its content and children:

	if (!strcmp (sp_repr_name (repr), "text")) {
		add_whitespace = FALSE; 
	}

	for ( attr = repr->attributes ; attr != NULL ; attr = attr->next ) {
		const gchar *key, *val;

		key = SP_REPR_ATTRIBUTE_KEY (attr);
		val = SP_REPR_ATTRIBUTE_VALUE (attr);
		fputs ("\n", file);
		for ( i = 0 ; i < indent_level + 1 ; i++ ) {
			fputs ("  ", file);
		}
		fprintf (file, " %s=\"", key);
		repr_quote_write (file, val);
		putc ('"', file);
	}

	loose = TRUE;
	for (child = repr->children; child != NULL; child = child->next) {
		if (child->type == SP_XML_TEXT_NODE) {
			loose = FALSE;
			break;
		}
	}
	if (repr->children) {
		fputs (">", file);
		if (loose && add_whitespace) {
			fputs ("\n", file);
		}
		for (child = repr->children; child != NULL; child = child->next) {
			if (child->type == SP_XML_TEXT_NODE) {
				repr_quote_write (file, sp_repr_content (child));
			} else if (child->type == SP_XML_COMMENT_NODE) {
				fprintf (file, "<!--%s-->", sp_repr_content (child));
			} else {
				sp_repr_write_stream (child, file, (loose) ? (indent_level + 1) : 0, add_whitespace); 
			}
		}
		
		if (loose && add_whitespace) {
			for (i = 0; i < indent_level; i++) {
				fputs ("  ", file);
			}
		}
		fprintf (file, "</%s>", sp_repr_name (repr));
	} else {
		fputs (" />", file);
	}

	// text elements cannot nest, so we can output newline
	// after closing text

	if (add_whitespace || !strcmp (sp_repr_name (repr), "text")) {
		fputs ("\n", file); 
	}
}

