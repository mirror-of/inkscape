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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <glib.h>

#include "xml/repr.h"
#include "xml/sp-repr-attr.h"

#include "io/sys.h"

#include <map>
#include <glibmm/ustring.h>
#include <glibmm/quark.h>
#include "util/shared-c-string-ptr.h"

static SPReprDoc *sp_repr_do_read (xmlDocPtr doc, const gchar *default_ns);
static SPRepr *sp_repr_svg_read_node (xmlNodePtr node, const gchar *default_ns, GHashTable *prefix_map);
static gint sp_repr_qualified_name (gchar *p, gint len, xmlNsPtr ns, const xmlChar *name, const gchar *default_ns, GHashTable *prefix_map);
static void sp_repr_write_stream_root_element (SPRepr *repr, FILE *file, gboolean add_whitespace, gchar const *default_ns);
static void sp_repr_write_stream (SPRepr *repr, FILE *file, gint indent_level, gboolean add_whitespace, Glib::QueryQuark elide_prefix);
static void sp_repr_write_stream_element (SPRepr *repr, FILE *file, gint indent_level, gboolean add_whitespace, Glib::QueryQuark elide_prefix, SPReprAttr const *attributes);

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
    // TODO: need to replace with our own fopen and reading
    gchar* localFilename = g_filename_from_utf8 ( filename,
                                 -1,  &bytesRead,  &bytesWritten, &error);

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
#else /* !HAVE_LIBWMF */
    doc = xmlParseFile (localFilename);
#endif /* !HAVE_LIBWMF */

    rdoc = sp_repr_do_read (doc, default_ns);
    if (doc)
        xmlFreeDoc (doc);

    if ( localFilename != NULL )
        g_free (localFilename);

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
    if (doc)
        xmlFreeDoc (doc);
    return rdoc;
}

namespace Inkscape {

struct compare_quark_ids {
    bool operator()(Glib::QueryQuark const &a, Glib::QueryQuark const &b) {
        return a.id() < b.id();
    }
};

}

namespace {

typedef std::map<Glib::QueryQuark, Glib::QueryQuark, Inkscape::compare_quark_ids> PrefixMap;

Glib::QueryQuark qname_prefix(Glib::QueryQuark qname) {
    static PrefixMap prefix_map;
    PrefixMap::iterator iter = prefix_map.find(qname);
    if ( iter != prefix_map.end() ) {
        return (*iter).second;
    } else {
        gchar const *name_string=g_quark_to_string(qname);
        gchar const *prefix_end=strchr(name_string, ':');
        if (prefix_end) {
            Glib::Quark prefix=Glib::ustring(name_string, prefix_end);
            prefix_map.insert(PrefixMap::value_type(qname, prefix));
            return prefix;
        } else {
            return GQuark(0);
        }
    }
}

}

namespace {

void promote_to_svg_namespace(SPRepr *repr) {
    if ( repr->type() == SP_XML_ELEMENT_NODE ) {
        GQuark code = repr->code();
        if (!qname_prefix(code).id()) {
            gchar *svg_name = g_strconcat("svg:", g_quark_to_string(code), NULL);
            repr->setCodeUnsafe(g_quark_from_string(svg_name));
            g_free(svg_name);
        }
        for ( SPRepr *child = sp_repr_children(repr) ; child ; child = sp_repr_next(child) ) {
            promote_to_svg_namespace(child);
        }
    }
}

}

/**
 * Reads in a XML file to create a SPReprDoc
 */
SPReprDoc *
sp_repr_do_read (xmlDocPtr doc, const gchar *default_ns)
{
    if (doc == NULL) return NULL;
    xmlNodePtr node=xmlDocGetRootElement (doc);
    if (node == NULL) return NULL;

    GHashTable * prefix_map;
    prefix_map = g_hash_table_new (g_str_hash, g_str_equal);

    GSList *reprs=NULL;
    SPRepr *root=NULL;

    for ( node = doc->children ; node != NULL ; node = node->next ) {
        if (node->type == XML_ELEMENT_NODE) {
            SPRepr *repr=sp_repr_svg_read_node (node, default_ns, prefix_map);
            reprs = g_slist_append(reprs, repr);

            if (!root) {
                root = repr;
            } else {
                root = NULL;
                break;
            }
        } else if ( node->type == XML_COMMENT_NODE ) {
            SPRepr *comment=sp_repr_svg_read_node(node, default_ns, prefix_map);
            reprs = g_slist_append(reprs, comment);
        }
    }

    SPReprDoc *rdoc=NULL;

    if (root != NULL) {
        /* promote elements of SVG documents that don't use namespaces
         * into the SVG namespace */
        if ( default_ns && !strcmp(default_ns, SP_SVG_NS_URI)
             && !strcmp(sp_repr_name(root), "svg") )
        {
            promote_to_svg_namespace(root);
        }

        rdoc = sp_repr_document_new_list(reprs);
    }

    for ( GSList *iter = reprs ; iter ; iter = iter->next ) {
        SPRepr *repr=(SPRepr *)iter->data;
        sp_repr_unref(repr);
    }
    g_slist_free(reprs);

    g_hash_table_destroy (prefix_map);

    return rdoc;
}

gint
sp_repr_qualified_name (gchar *p, gint len, xmlNsPtr ns, const xmlChar *name, const gchar *default_ns, GHashTable *prefix_map)
{
    const xmlChar *prefix;
    if ( ns && ns->href ) {
        prefix = (xmlChar*)sp_xml_ns_uri_prefix ((gchar*)ns->href, (char*)ns->prefix);
        g_hash_table_insert (prefix_map, (gpointer)prefix, (gpointer)ns->href);
    } else {
        prefix = NULL;
    }

    if (prefix)
        return g_snprintf (p, len, "%s:%s", (gchar*)prefix, name);
    else
        return g_snprintf (p, len, "%s", name);
}

static SPRepr *
sp_repr_svg_read_node (xmlNodePtr node, const gchar *default_ns, GHashTable *prefix_map)
{
    SPRepr *repr, *crepr;
    xmlAttrPtr prop;
    xmlNodePtr child;
    gchar c[256];

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

        SPRepr *rdoc = sp_repr_new_text((const gchar *)node->content);
        return rdoc;
    }

    if (node->type == XML_COMMENT_NODE)
        return sp_repr_new_comment((const gchar *)node->content);

    if (node->type == XML_ENTITY_DECL) return NULL;

    sp_repr_qualified_name (c, 256, node->ns, node->name, default_ns, prefix_map);
    repr = sp_repr_new (c);
    /* TODO remember node->ns->prefix if node->ns != NULL */

    for (prop = node->properties; prop != NULL; prop = prop->next) {
        if (prop->children) {
            sp_repr_qualified_name (c, 256, prop->ns, prop->name, default_ns, prefix_map);
            sp_repr_set_attr (repr, c, (gchar*)prop->children->content);
            /* TODO remember prop->ns->prefix if prop->ns != NULL */
        }
    }

    if (node->content)
        sp_repr_set_content (repr, (gchar*)node->content);

    child = node->xmlChildrenNode;
    for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
        crepr = sp_repr_svg_read_node (child, default_ns, prefix_map);
        if (crepr) {
            sp_repr_append_child (repr, crepr);
            sp_repr_unref (crepr);
        }
    }

    return repr;
}

void
sp_repr_save_stream (SPReprDoc *doc, FILE *fp, gchar const *default_ns)
{
    SPRepr *repr;
    const gchar *str;

    /* fixme: do this The Right Way */

    fputs ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n", fp);

    str = sp_repr_attr ((SPRepr *) doc, "doctype");
    if (str) fputs (str, fp);

    repr = sp_repr_document_first_child(doc);
    for ( repr = sp_repr_document_first_child(doc) ;
          repr ; repr = sp_repr_next(repr) )
    {
        if ( repr->type() == SP_XML_ELEMENT_NODE ) {
            sp_repr_write_stream_root_element(repr, fp, TRUE, default_ns);
        } else if ( repr->type() == SP_XML_COMMENT_NODE ) {
            sp_repr_write_stream(repr, fp, 0, TRUE, GQuark(0));
            fputc('\n', fp);
        } else {
            sp_repr_write_stream(repr, fp, 0, TRUE, GQuark(0));
        }
    }
}

/* Returns TRUE if file successfully saved; FALSE if not
 */
gboolean
sp_repr_save_file (SPReprDoc *doc, const gchar *filename,
                   gchar const *default_ns)
{
    if (filename == NULL) {
        return FALSE;
    }
    Inkscape::IO::dump_fopen_call( filename, "B" );
    FILE *file = Inkscape::IO::fopen_utf8name(filename, "w");
    if (file == NULL) {
        return FALSE;
    }

    sp_repr_save_stream (doc, file, default_ns);

    if (fclose (file) != 0) {
        return FALSE;
    }

    return TRUE;
}

void
sp_repr_print (SPRepr * repr)
{
    sp_repr_write_stream (repr, stdout, 0, TRUE, GQuark(0));

    return;
}

/* (No doubt this function already exists elsewhere.) */
static void
repr_quote_write (FILE * file, const gchar * val)
{
    if (!val) return;

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

namespace {

typedef std::map<Glib::QueryQuark, gchar const *, Inkscape::compare_quark_ids> LocalNameMap;
typedef std::map<Glib::QueryQuark, Inkscape::Util::SharedCStringPtr, Inkscape::compare_quark_ids> NSMap;

gchar const *qname_local_name(Glib::QueryQuark qname) {
    static LocalNameMap local_name_map;
    LocalNameMap::iterator iter = local_name_map.find(qname);
    if ( iter != local_name_map.end() ) {
        return (*iter).second;
    } else {
        gchar const *name_string=g_quark_to_string(qname);
        gchar const *prefix_end=strchr(name_string, ':');
        if (prefix_end) {
            return prefix_end + 1;
        } else {
            return name_string;
        }
    }
}

void add_ns_map_entry(NSMap &ns_map, Glib::QueryQuark prefix) {
    using Inkscape::Util::SharedCStringPtr;

    static const Glib::QueryQuark xml_prefix("xml");

    NSMap::iterator iter=ns_map.find(prefix);
    if ( iter == ns_map.end() ) {
        if (prefix.id()) {
            gchar const *uri=sp_xml_ns_prefix_uri(g_quark_to_string(prefix));
            if (uri) {
                ns_map.insert(NSMap::value_type(prefix, SharedCStringPtr::coerce(uri)));
            } else if ( prefix != xml_prefix ) {
                g_warning("No namespace known for normalized prefix %s", g_quark_to_string(prefix));
            }
        } else {
            ns_map.insert(NSMap::value_type(prefix, SharedCStringPtr()));
        }
    }
}

void populate_ns_map(NSMap &ns_map, SPRepr &repr) {
    if ( repr.type() == SP_XML_ELEMENT_NODE ) {
        add_ns_map_entry(ns_map, qname_prefix(repr.code()));
        for ( SPReprAttr const *attr=repr.attributeList() ;
              attr ; attr = attr->next )
        {
            Glib::QueryQuark prefix=qname_prefix(attr->key);
            if (prefix.id()) {
                add_ns_map_entry(ns_map, prefix);
            }
        }
        for ( SPRepr *child=sp_repr_children(&repr) ;
              child ; child = sp_repr_next(child) )
        {
            populate_ns_map(ns_map, *child);
        }
    }
}

}

void
sp_repr_write_stream_root_element (SPRepr *repr, FILE *file, gboolean add_whitespace, gchar const *default_ns)
{
    using Inkscape::Util::SharedCStringPtr;
    g_assert(repr != NULL);

    NSMap ns_map;
    populate_ns_map(ns_map, *repr);

    Glib::QueryQuark elide_prefix=GQuark(0);
    if ( default_ns && ns_map.find(GQuark(0)) == ns_map.end() ) {
        elide_prefix = g_quark_from_string(sp_xml_ns_uri_prefix(default_ns, NULL));
    }

    SPReprAttr const *attributes=repr->attributeList();
    for ( NSMap::iterator iter=ns_map.begin() ; iter != ns_map.end() ; ++iter ) 
    {
        Glib::QueryQuark prefix=(*iter).first;
        SharedCStringPtr ns_uri=(*iter).second;

        if (prefix.id()) {
            if ( elide_prefix == prefix ) {
                attributes = new SPReprAttr(g_quark_from_static_string("xmlns"), ns_uri, const_cast<SPReprAttr *>(attributes));
            }

            Glib::ustring attr_name="xmlns:";
            attr_name.append(g_quark_to_string(prefix));
            GQuark key = g_quark_from_string(attr_name.c_str());
            attributes = new SPReprAttr(key, ns_uri, const_cast<SPReprAttr *>(attributes)); 
        } else {
            // if there are non-namespaced elements, we can't globally
            // use a default namespace
            elide_prefix = GQuark(0);
        }
    }

    return sp_repr_write_stream_element(repr, file, 0, add_whitespace, elide_prefix, attributes);
}

void
sp_repr_write_stream (SPRepr *repr, FILE *file, gint indent_level,
                      gboolean add_whitespace, Glib::QueryQuark elide_prefix)
{
    if (repr->type() == SP_XML_TEXT_NODE) {
        repr_quote_write (file, sp_repr_content (repr));
    } else if (repr->type() == SP_XML_COMMENT_NODE) {
        fprintf (file, "<!--%s-->", sp_repr_content (repr));
    } else if (repr->type() == SP_XML_ELEMENT_NODE) {
        sp_repr_write_stream_element(repr, file, indent_level, add_whitespace, elide_prefix, repr->attributeList());
    } else {
        g_assert_not_reached();
    }
}

void
sp_repr_write_stream_element (SPRepr * repr, FILE * file, gint indent_level,
                              gboolean add_whitespace,
                              Glib::QueryQuark elide_prefix,
                              SPReprAttr const *attributes)
{
    SPRepr *child;
    gboolean loose;
    gint i;

    g_return_if_fail (repr != NULL);
    g_return_if_fail (file != NULL);

    if ( indent_level > 16 )
        indent_level = 16;

    if (add_whitespace) {
        for ( i = 0 ; i < indent_level ; i++ ) {
            fputs ("  ", file);
        }
    }

    GQuark code = repr->code();
    gchar const *element_name;
    if ( elide_prefix == qname_prefix(code) ) {
        element_name = qname_local_name(code);
    } else {
        element_name = g_quark_to_string(code);
    }
    fprintf (file, "<%s", element_name);

    // TODO: this should depend on xml:space, not the element name

    // if this is a <text> element, suppress formatting whitespace
    // for its content and children:

    if (!strcmp(sp_repr_name(repr), "svg:text")) {
        add_whitespace = FALSE;
    }

    for ( SPReprAttr const *attr = attributes ; attr != NULL ; attr = attr->next ) {
        gchar const * const key = SP_REPR_ATTRIBUTE_KEY(attr);
        gchar const * const val = SP_REPR_ATTRIBUTE_VALUE(attr);
        fputs ("\n", file);
        for ( i = 0 ; i < indent_level + 1 ; i++ ) {
            fputs ("  ", file);
        }
        fprintf (file, " %s=\"", key);
        repr_quote_write (file, val);
        putc ('"', file);
    }

    loose = TRUE;
    for (child = repr->firstChild() ; child != NULL; child = child->next()) {
        if (child->type() == SP_XML_TEXT_NODE) {
            loose = FALSE;
            break;
        }
    }
    if (repr->firstChild()) {
        fputs (">", file);
        if (loose && add_whitespace) {
            fputs ("\n", file);
        }
        for (child = repr->firstChild(); child != NULL; child = child->next()) {
            sp_repr_write_stream (child, file, (loose) ? (indent_level + 1) : 0, add_whitespace, elide_prefix);
        }

        if (loose && add_whitespace) {
            for (i = 0; i < indent_level; i++) {
                fputs ("  ", file);
            }
        }
        fprintf (file, "</%s>", element_name);
    } else {
        fputs (" />", file);
    }

    // text elements cannot nest, so we can output newline
    // after closing text

    if (add_whitespace || !strcmp (sp_repr_name (repr), "svg:text")) {
        fputs("\n", file);
    }
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
