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

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <glib.h>

#include "repr-private.h"

static const unsigned char *sp_svg_doctype_str =
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"\n"
"\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n";
static const unsigned char *sp_comment_str =
"<!-- Created with Sodipodi (\"http://www.sodipodi.com/\") -->\n";

static SPReprDoc *sp_repr_do_read (xmlDocPtr doc, const gchar *default_ns);
static SPRepr * sp_repr_svg_read_node (SPXMLDocument *doc, xmlNodePtr node, const gchar *default_ns, GHashTable *prefix_map);
static void sp_repr_set_xmlns_attr (const unsigned char *prefix, const unsigned char *uri, SPRepr *repr);
static gint sp_repr_qualified_name (unsigned char *p, gint len, xmlNsPtr ns, const xmlChar *name, const gchar *default_ns, GHashTable *prefix_map);

#ifdef HAVE_LIBWMF
static xmlDocPtr sp_wmf_convert (const char * file_name);
static char * sp_wmf_image_name (void * context);
#endif /* HAVE_LIBWMF */

SPReprDoc *
sp_repr_read_file (const gchar * filename, const gchar *default_ns)
{
	xmlDocPtr doc;
	SPReprDoc * rdoc;

	xmlSubstituteEntitiesDefault(1);

	g_return_val_if_fail (filename != NULL, NULL);

#ifdef HAVE_LIBWMF
	if (strlen (filename) > 4) {
		if ( (strcmp (filename + strlen (filename) - 4,".wmf") == 0)
		  || (strcmp (filename + strlen (filename) - 4,".WMF") == 0))
			doc = sp_wmf_convert (filename);
		else
			doc = xmlParseFile (filename);
	}
	else {
		doc = xmlParseFile (filename);
	}
#else /* HAVE_LIBWMF */
	doc = xmlParseFile (filename);
#endif /* HAVE_LIBWMF */

	rdoc = sp_repr_do_read (doc, default_ns);
	if (doc) {
		xmlFreeDoc (doc);
	}
	return rdoc;
}

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
		/* always include Sodipodi namespace */
		sp_repr_set_xmlns_attr (sp_xml_ns_uri_prefix (SP_SODIPODI_NS_URI, "sodipodi"), SP_SODIPODI_NS_URI, repr);

		if (!strcmp (sp_repr_name (repr), "svg") && default_ns && !strcmp (default_ns, SP_SVG_NS_URI)) {
			
			sp_repr_set_attr ((SPRepr *) rdoc, "doctype", sp_svg_doctype_str);
			sp_repr_set_attr ((SPRepr *) rdoc, "comment", sp_comment_str);
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
sp_repr_set_xmlns_attr (const unsigned char *prefix, const unsigned char *uri, SPRepr *repr)
{
	gchar *name;
	name = g_strconcat ("xmlns:", prefix, NULL);
	sp_repr_set_attr (repr, name, uri);
	g_free (name);
}

gint
sp_repr_qualified_name (unsigned char *p, gint len, xmlNsPtr ns, const xmlChar *name, const gchar *default_ns, GHashTable *prefix_map)
{
	const gchar *prefix;
	if (ns) {
		if (!ns->href) {
			prefix = ns->prefix;
		} else if (default_ns && !strcmp (ns->href, default_ns)) {
			prefix = NULL;
		} else {
			prefix = sp_xml_ns_uri_prefix (ns->href, ns->prefix);
			g_hash_table_insert (prefix_map, (gpointer)prefix, (gpointer)ns->href);
		}
	} else {
		prefix = NULL;
	}

	if (prefix) {
		return g_snprintf (p, len, "%s:%s", prefix, name);
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
	g_print ("Node %d %s contains %s\n", node->type, node->name, node->content);
#endif

	if (node->type == XML_TEXT_NODE) {
		xmlChar *p;
		for (p = node->content; p && *p; p++) {
			if (!isspace (*p)) {
				xmlChar *e;
				unsigned char *s;
				SPRepr *rdoc;
				e = p + strlen (p) - 1;
				while (*e && isspace (*e)) e -= 1;
				s = g_new (unsigned char, e - p + 2);
				memcpy (s, p, e - p + 1);
				s[e - p + 1] = '\0';
				rdoc = sp_xml_document_createTextNode (doc, s);
				g_free (s);
				return rdoc;
			}
		}
		return NULL;
	}

	if (node->type == XML_COMMENT_NODE) return NULL;
	if (node->type == XML_ENTITY_DECL) return NULL;

	sp_repr_qualified_name (c, 256, node->ns, node->name, default_ns, prefix_map);
	repr = sp_repr_new (c);

	for (prop = node->properties; prop != NULL; prop = prop->next) {
		if (prop->children) {
			sp_repr_qualified_name (c, 256, prop->ns, prop->name, default_ns, prefix_map);
			sp_repr_set_attr (repr, c, prop->children->content);
		}
	}

	if (node->content)
		sp_repr_set_content (repr, node->content);

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
	const unsigned char *str;

	/* fixme: do this The Right Way */

	fputs ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n", fp);

	str = sp_repr_attr ((SPRepr *) doc, "doctype");
	if (str) fputs (str, fp);
	str = sp_repr_attr ((SPRepr *) doc, "comment");
	if (str) fputs (str, fp);

	repr = sp_repr_document_root (doc);

	sp_repr_write_stream (repr, fp, 0);
}

void
sp_repr_save_file (SPReprDoc *doc, const gchar *filename)
{
	FILE * file;

	file = fopen (filename, "w");
	g_return_if_fail (file != NULL);

	sp_repr_save_stream (doc, file);

	fclose (file);

	return;
}

void
sp_repr_print (SPRepr * repr)
{
	sp_repr_write_stream (repr, stdout, 0);

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
sp_repr_write_stream (SPRepr * repr, FILE * file, gint level)
{
	SPReprAttr *attr;
	SPRepr *child;
	const gchar *key, *val;
	gboolean loose;
	gint i;

	g_return_if_fail (repr != NULL);
	g_return_if_fail (file != NULL);

	if (level > 16) level = 16;
	for (i = 0; i < level; i++) fputs ("  ", file);
	fprintf (file, "<%s", sp_repr_name (repr));

	for (attr = repr->attributes; attr != NULL; attr = attr->next) {
		key = SP_REPR_ATTRIBUTE_KEY (attr);
		val = SP_REPR_ATTRIBUTE_VALUE (attr);
		fputs ("\n", file);
		for (i = 0; i < level + 1; i++) fputs ("  ", file);
		fprintf (file, " %s=\"", key);
		repr_quote_write (file, val);
		putc ('"', file);
	}
	
	if (repr->children || sp_repr_content (repr)) {
		if (sp_repr_content (repr)) {
			fputs (">", file);
			repr_quote_write (file, sp_repr_content (repr));
		} else {
			fputs (">\n", file);
		}

		loose = TRUE;
		for (child = repr->children; child != NULL; child = child->next) {
			if (child->type == SP_XML_TEXT_NODE) {
				repr_quote_write (file, sp_repr_content (child));
				loose = FALSE;
			} else {
				sp_repr_write_stream (child, file, (loose) ? (level + 1) : 0);
				loose = TRUE;
			}
		}
		
		if (loose) {
			for (i = 0; i < level; i++) fputs ("  ", file);
		}
		fprintf (file, "</%s>\n", sp_repr_name (repr));
	} else {
		fputs (" />\n", file);
	}
}

#ifdef HAVE_LIBWMF

#include <math.h>

#include <libwmf/api.h>
#include <libwmf/svg.h>

#define SP_WMF_MAXWIDTH  (596)
#define SP_WMF_MAXHEIGHT (812)

typedef struct _SPImageContext SPImageContext;

struct _SPImageContext
{	wmfAPI* API;

	int number;

	char* prefix;
};

static xmlDocPtr
sp_wmf_convert (const char * file_name)
{
	float wmf_width;
	float wmf_height;

	float ratio_wmf;
	float ratio_bounds;

	unsigned long flags;
	unsigned long max_flags;
	unsigned long length;

	char * buffer = 0;

	static char * Default_Description = "sodipodi";

	SPImageContext IC;

	wmf_error_t err;

	wmf_svg_t * ddata = 0;

	wmfAPI * API = 0;

	wmfAPI_Options api_options;

	wmfD_Rect bbox;

	xmlDocPtr doc = 0;

	flags = WMF_OPT_IGNORE_NONFATAL;

	flags |= WMF_OPT_FUNCTION;
	api_options.function = wmf_svg_function;

	err = wmf_api_create (&API, flags, &api_options);

	if (err != wmf_E_None) {
		if (API) wmf_api_destroy (API);
		return (0);
	}

	err = wmf_file_open (API, (char *) file_name);

	if (err != wmf_E_None) {
		wmf_api_destroy (API);
		return (0);
	}

	err = wmf_scan (API, 0, &bbox);

	if (err != wmf_E_None) {
		wmf_api_destroy (API);
		return (0);
	}

/* Okay, got this far, everything seems cool.
 */
	ddata = WMF_SVG_GetData (API);

	ddata->out = wmf_stream_create (API, 0);

	ddata->Description = Default_Description;

	ddata->bbox = bbox;

	wmf_width  = ddata->bbox.BR.x - ddata->bbox.TL.x;
	wmf_height = ddata->bbox.BR.y - ddata->bbox.TL.y;

	if ((wmf_width <= 0) || (wmf_height <= 0)) { /* Bad image size - but this error shouldn't occur... */
		wmf_api_destroy (API);
		return (0);
	}

	max_flags = 0;

	if ((wmf_width > (float) SP_WMF_MAXWIDTH) || (wmf_height > (float) SP_WMF_MAXHEIGHT)) {
		if (max_flags == 0)
			max_flags = 1;
	}

	if (max_flags) { /* scale the image */
		ratio_wmf = wmf_height / wmf_width;
		ratio_bounds = (float) SP_WMF_MAXHEIGHT / (float) SP_WMF_MAXWIDTH;

		if (ratio_wmf > ratio_bounds) {
			ddata->height = SP_WMF_MAXHEIGHT;
			ddata->width  = (unsigned int) ((float) ddata->height / ratio_wmf);
		}
		else {
			ddata->width  = SP_WMF_MAXWIDTH;
			ddata->height = (unsigned int) ((float) ddata->width  * ratio_wmf);
		}
	}
	else {
		ddata->width  = (unsigned int) ceil ((double) wmf_width );
		ddata->height = (unsigned int) ceil ((double) wmf_height);
	}

	IC.API = API;
	IC.number = 0;
	IC.prefix = (char *) wmf_malloc (API, strlen (file_name) + 1);
	if (IC.prefix) {
		strcpy (IC.prefix, file_name);
		IC.prefix[strlen (file_name) - 4] = 0;
		ddata->image.context = (void *) (&IC);
		ddata->image.name = sp_wmf_image_name;
	}

	err = wmf_play (API, 0, &bbox);

	if (err == wmf_E_None) {
		wmf_stream_destroy (API, ddata->out, &buffer, &length);

		doc = xmlParseMemory (buffer, (int) length);
	}

	wmf_api_destroy (API);

	return (doc);
}

static char *
sp_wmf_image_name (void * context)
{
	SPImageContext * IC = (SPImageContext *) context;

	int length;

	char * name = 0;

	length = strlen (IC->prefix) + 16;

	name = wmf_malloc (IC->API, length);

	if (name == 0) return (0);

	IC->number++;

	sprintf (name,"%s-%d.png", IC->prefix, IC->number);

	return (name);
}

#endif /* HAVE_LIBWMF */
