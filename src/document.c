#define __SP_DOCUMENT_C__

/*
 * SVG document implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_DOCUMENT_DEBUG_IDLE
#define noSP_DOCUMENT_DEBUG_UNDO

#include <config.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtkmain.h>
#include <xml/repr.h>
#include "helper/sp-marshal.h"
#include "helper/sp-intl.h"
#include "inkscape-private.h"
#include "sp-object-repr.h"
#include "sp-root.h"
#include "sp-namedview.h"
#include "document-private.h"
#include "desktop.h"
#include "version.h"

#define A4_WIDTH_STR "210mm"
#define A4_HEIGHT_STR "297mm"

#define SP_DOCUMENT_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE - 1)

enum {
	MODIFIED,
	URI_SET,
	RESIZED,
	LAST_SIGNAL
};

static void sp_document_class_init (SPDocumentClass * klass);
static void sp_document_init (SPDocument *document);
static void sp_document_dispose (GObject *object);

static gint sp_document_idle_handler (gpointer data);

gboolean sp_document_resource_list_free (gpointer key, gpointer value, gpointer data);

static GObjectClass * parent_class;
static guint signals[LAST_SIGNAL] = {0};
static gint doc_count = 0;

GType
sp_document_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPDocumentClass),
			NULL, NULL,
			(GClassInitFunc) sp_document_class_init,
			NULL, NULL,
			sizeof (SPDocument),
			4,
			(GInstanceInitFunc) sp_document_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPDocument", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_document_class_init (SPDocumentClass * klass)
{
	GObjectClass * object_class;

	object_class = (GObjectClass *) klass;

	parent_class = (GObjectClass*)g_type_class_peek_parent (klass);

	signals[MODIFIED] = g_signal_new ("modified",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_FIRST,
					    G_STRUCT_OFFSET (SPDocumentClass, modified),
					    NULL, NULL,
					    sp_marshal_NONE__UINT,
					    G_TYPE_NONE, 1,
					    G_TYPE_UINT);
	signals[URI_SET] =    g_signal_new ("uri_set",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_FIRST,
					    G_STRUCT_OFFSET (SPDocumentClass, uri_set),
					    NULL, NULL,
					    sp_marshal_NONE__POINTER,
					    G_TYPE_NONE, 1,
					    G_TYPE_POINTER);
	signals[RESIZED] =    g_signal_new ("resized",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_FIRST,
					    G_STRUCT_OFFSET (SPDocumentClass, uri_set),
					    NULL, NULL,
					    sp_marshal_NONE__DOUBLE_DOUBLE,
					    G_TYPE_NONE, 2,
					    G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	object_class->dispose = sp_document_dispose;
}

static void
sp_document_init (SPDocument *doc)
{
	SPDocumentPrivate *p;

	doc->advertize = FALSE;
	doc->keepalive = FALSE;

	doc->modified_id = 0;

	doc->rdoc = NULL;
	doc->rroot = NULL;
	doc->root = NULL;

	doc->uri = NULL;
	doc->base = NULL;
	doc->name = NULL;

	p = g_new (SPDocumentPrivate, 1);

	p->iddef = g_hash_table_new (g_str_hash, g_str_equal);

	p->resources = g_hash_table_new (g_str_hash, g_str_equal);

	p->sensitive = FALSE;
	p->partial = NULL;
	p->history_size = 0;
	p->undo = NULL;
	p->redo = NULL;

	doc->priv = p;
}

static void
sp_document_dispose (GObject *object)
{
	SPDocument *doc;
	SPDocumentPrivate * priv;

	doc = (SPDocument *) object;
	priv = doc->priv;

	if (priv) {
		inkscape_remove_document (doc);

		if (priv->partial) {
			sp_repr_free_log (priv->partial);
			priv->partial = NULL;
		}

		sp_document_clear_redo (doc);
		sp_document_clear_undo (doc);

		if (doc->root) {
			sp_object_invoke_release (doc->root);
			g_object_unref (G_OBJECT (doc->root));
			doc->root = NULL;
		}

		if (priv->iddef) g_hash_table_destroy (priv->iddef);

		if (doc->rdoc) sp_repr_document_unref (doc->rdoc);

		/* Free resources */
		g_hash_table_foreach_remove (priv->resources, sp_document_resource_list_free, doc);
		g_hash_table_destroy (priv->resources);

		g_free (priv);
		doc->priv = NULL;
	}

	if (doc->name) {
		g_free (doc->name);
		doc->name = NULL;
	}
	if (doc->base) {
		g_free (doc->base);
		doc->base = NULL;
	}
	if (doc->uri) {
		g_free (doc->uri);
		doc->uri = NULL;
	}

	if (doc->modified_id) {
		gtk_idle_remove (doc->modified_id);
		doc->modified_id = 0;
	}

	if (doc->keepalive) {
		inkscape_unref ();
		doc->keepalive = FALSE;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static SPDocument *
sp_document_create (SPReprDoc *rdoc,
		    const gchar *uri,
		    const gchar *base,
		    const gchar *name,
		    unsigned int advertize,
		    unsigned int keepalive)
{
	SPDocument *document;
	SPRepr *rroot;
	SPVersion sodipodi_version;

	rroot = sp_repr_document_root (rdoc);

	document = (SPDocument*)g_object_new (SP_TYPE_DOCUMENT, NULL);

	document->advertize = advertize;
	document->keepalive = keepalive;

	document->rdoc = rdoc;
	document->rroot = rroot;

	document->uri = g_strdup (uri);
	document->base = g_strdup (base);
	document->name = g_strdup (name);

	document->root = sp_object_repr_build_tree (document, rroot);

	sodipodi_version = SP_ROOT (document->root)->version.sodipodi;

	/* fixme: Not sure about this, but lets assume ::build updates */
	sp_repr_set_attr (rroot, "sodipodi:version", SODIPODI_VERSION);
	sp_repr_set_attr (rroot, "inkscape:version", VERSION);
	/* fixme: Again, I moved these here to allow version determining in ::build (Lauris) */

	/* A quick hack to get namespaces into doc */
	sp_repr_set_attr (rroot, "xmlns", SP_SVG_NS_URI);
	sp_repr_set_attr (rroot, "xmlns:sodipodi", SP_SODIPODI_NS_URI);
	sp_repr_set_attr (rroot, "xmlns:inkscape", SP_INKSCAPE_NS_URI);
	sp_repr_set_attr (rroot, "xmlns:xlink", SP_XLINK_NS_URI);
	/* End of quick hack */

	/* Quick hack 2 - get default image size into document */
	if (!sp_repr_attr (rroot, "width")) sp_repr_set_attr (rroot, "width", A4_WIDTH_STR);
	if (!sp_repr_attr (rroot, "height")) sp_repr_set_attr (rroot, "height", A4_HEIGHT_STR);
	/* End of quick hack 2 */
	/* Quick hack 3 - Set uri attributes */
	if (uri) {
		/* fixme: Think, what this means for images (Lauris) */
		sp_repr_set_attr (rroot, "sodipodi:docname", uri);
		sp_repr_set_attr (rroot, "sodipodi:docbase", document->base);
	}
	/* End of quick hack 3 */
	/* between 0 and 0.25 */
	if (sp_version_inside_range (sodipodi_version, 0, 0, 0, 25)) {
		/* Clear ancient spec violating attributes */
		sp_repr_set_attr (rroot, "SP-DOCNAME", NULL);
		sp_repr_set_attr (rroot, "SP-DOCBASE", NULL);
		sp_repr_set_attr (rroot, "docname", NULL);
		sp_repr_set_attr (rroot, "docbase", NULL);
	}

	/* Namedviews */
	if (!sp_item_group_get_child_by_name ((SPGroup *) document->root, NULL, "sodipodi:namedview")) {
		SPRepr *r;
		r = inkscape_get_repr (INKSCAPE, "template.sodipodi:namedview");
		if (!r) r = sp_repr_new ("sodipodi:namedview");
		sp_repr_set_attr (r, "id", "base");
		sp_repr_add_child (rroot, r, NULL);
		sp_repr_unref (r);
	}

	/* Defs */
	if (!SP_ROOT (document->root)->defs) {
		SPRepr *r;
		r = sp_repr_new ("defs");
		sp_repr_add_child (rroot, r, NULL);
		sp_repr_unref (r);
		g_assert (SP_ROOT (document->root)->defs);
	}

	if (keepalive) {
		inkscape_ref ();
	}

	sp_document_set_undo_sensitive (document, TRUE);

	inkscape_add_document (document);

	return document;
}

SPDocument *
sp_document_new (const gchar *uri, unsigned int advertize, unsigned int keepalive)
{
	SPDocument *doc;
	SPReprDoc *rdoc;
	gchar *base, *name;

	if (uri) {
		SPRepr *rroot;
		gchar *s, *p;
		/* Try to fetch repr from file */
		rdoc = sp_repr_read_file (uri, SP_SVG_NS_URI);
		/* If file cannot be loaded, return NULL without warning */
		if (rdoc == NULL) return NULL;
		rroot = sp_repr_document_root (rdoc);
		/* If xml file is not svg, return NULL without warning */
		/* fixme: destroy document */
		if (strcmp (sp_repr_name (rroot), "svg") != 0) return NULL;
		s = g_strdup (uri);
		p = strrchr (s, '/');
		if (p) {
			name = g_strdup (p + 1);
			p[1] = '\0';
			base = g_strdup (s);
		} else {
			base = NULL;
			name = g_strdup (uri);
		}
		g_free (s);
	} else {
		rdoc = sp_repr_document_new ("svg");
		base = NULL;
		name = g_strdup_printf (_("New document %d"), ++doc_count);
	}

	doc = sp_document_create (rdoc, uri, base, name, advertize, keepalive);

	if (base) g_free (base);
	if (name) g_free (name);

	return doc;
}

SPDocument *
sp_document_new_from_mem (const gchar *buffer, gint length, unsigned int advertize, unsigned int keepalive)
{
	SPDocument *doc;
	SPReprDoc *rdoc;
	SPRepr *rroot;
	gchar *name;

	rdoc = sp_repr_read_mem (buffer, length, SP_SVG_NS_URI);

	/* If it cannot be loaded, return NULL without warning */
	if (rdoc == NULL) return NULL;

	rroot = sp_repr_document_root (rdoc);
	/* If xml file is not svg, return NULL without warning */
	/* fixme: destroy document */
	if (strcmp (sp_repr_name (rroot), "svg") != 0) return NULL;

	name = g_strdup_printf (_("Memory document %d"), ++doc_count);

	doc = sp_document_create (rdoc, NULL, NULL, name, advertize, keepalive);

	return doc;
}

SPDocument *
sp_document_ref (SPDocument *doc)
{
	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (doc), NULL);

	g_object_ref (G_OBJECT (doc));

	return doc;
}

SPDocument *
sp_document_unref (SPDocument *doc)
{
	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (doc), NULL);

	g_object_unref (G_OBJECT (doc));

	return NULL;
}

gdouble
sp_document_width (SPDocument * document)
{
	g_return_val_if_fail (document != NULL, 0.0);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), 0.0);
	g_return_val_if_fail (document->priv != NULL, 0.0);
	g_return_val_if_fail (document->root != NULL, 0.0);

	return SP_ROOT (document->root)->width.computed / 1.25;
}

gdouble
sp_document_height (SPDocument * document)
{
	g_return_val_if_fail (document != NULL, 0.0);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), 0.0);
	g_return_val_if_fail (document->priv != NULL, 0.0);
	g_return_val_if_fail (document->root != NULL, 0.0);

	return SP_ROOT (document->root)->height.computed / 1.25;
}

void
sp_document_set_uri (SPDocument *doc, const gchar *uri)
{
	g_return_if_fail (doc != NULL);
	g_return_if_fail (SP_IS_DOCUMENT (doc));

	if (doc->name) {
		g_free (doc->name);
		doc->name = NULL;
	}
	if (doc->base) {
		g_free (doc->base);
		doc->base = NULL;
	}
	if (doc->uri) {
		g_free (doc->uri);
		doc->uri = NULL;
	}

	if (uri) {
		gchar *s, *p;
		doc->uri = g_strdup (uri);
		/* fixme: Think, what this means for images (Lauris) */
		s = g_strdup (uri);
		p = strrchr (s, '/');
		if (p) {
			doc->name = g_strdup (p + 1);
			p[1] = '\0';
			doc->base = g_strdup (s);
		} else {
			doc->base = NULL;
			doc->name = g_strdup (doc->uri);
		}
		g_free (s);
	} else {
		doc->uri = g_strdup_printf (_("Unnamed document %d"), ++doc_count);
		doc->base = NULL;
		doc->name = g_strdup (doc->uri);
	}

	g_signal_emit (G_OBJECT (doc), signals [URI_SET], 0, doc->uri);
}

void
sp_document_set_size_px (SPDocument *doc, gdouble width, gdouble height)
{
	g_return_if_fail (doc != NULL);
	g_return_if_fail (SP_IS_DOCUMENT (doc));
	g_return_if_fail (width > 0.001);
	g_return_if_fail (height > 0.001);

	g_signal_emit (G_OBJECT (doc), signals [RESIZED], 0, width / 1.25, height / 1.25);
}

void
sp_document_def_id (SPDocument * document, const gchar * id, SPObject * object)
{
	g_assert (g_hash_table_lookup (document->priv->iddef, id) == NULL);

	g_hash_table_insert (document->priv->iddef, (gchar *) id, object);
}

void
sp_document_undef_id (SPDocument * document, const gchar * id)
{
	g_assert (g_hash_table_lookup (document->priv->iddef, id) != NULL);

	g_hash_table_remove (document->priv->iddef, id);
}

SPObject *
sp_document_lookup_id (SPDocument *doc, const gchar *id)
{
	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (doc), NULL);
	g_return_val_if_fail (id != NULL, NULL);

	return (SPObject*)g_hash_table_lookup (doc->priv->iddef, id);
}

/* Object modification root handler */

void
sp_document_request_modified (SPDocument *doc)
{
	if (!doc->modified_id) {
		doc->modified_id = gtk_idle_add_priority (SP_DOCUMENT_UPDATE_PRIORITY, sp_document_idle_handler, doc);
	}
}

gint
sp_document_ensure_up_to_date (SPDocument *doc)
{
	int lc;
	lc = 16;
	while (doc->root->uflags || doc->root->mflags) {
		lc -= 1;
		if (lc < 0) {
			g_warning ("More than 16 iterations while updating document '%s'", doc->uri);
			if (doc->modified_id) {
				/* Remove handler */
				gtk_idle_remove (doc->modified_id);
				doc->modified_id = 0;
			}
			return FALSE;
		}
		/* Process updates */
		if (doc->root->uflags) {
			SPItemCtx ctx;
			ctx.ctx.flags = 0;
			nr_matrix_d_set_identity (&ctx.i2doc);
			/* Set up viewport in case svg has it defined as percentages */
			ctx.vp.x0 = 0.0;
			ctx.vp.y0 = 0.0;
			ctx.vp.x1 = 21.0 / 2.54 * 72.0 * 1.25;
			ctx.vp.y1 = 29.7 / 2.54 * 72.0 * 1.25;
			nr_matrix_d_set_identity (&ctx.i2vp);
			sp_object_invoke_update (doc->root, (SPCtx *) &ctx, 0);
		}
		/* Emit "modified" signal on objects */
		sp_object_invoke_modified (doc->root, 0);
		/* Emit our own "modified" signal */
		g_signal_emit (G_OBJECT (doc), signals [MODIFIED], 0,
			       SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG);
	}
	if (doc->modified_id) {
		/* Remove handler */
		gtk_idle_remove (doc->modified_id);
		doc->modified_id = 0;
	}
	return TRUE;
}

static gint
sp_document_idle_handler (gpointer data)
{
	SPDocument *doc;
	int repeat;

	doc = SP_DOCUMENT (data);

#ifdef SP_DOCUMENT_DEBUG_IDLE
	g_print ("->\n");
#endif

	/* Process updates */
	if (doc->root->uflags) {
		SPItemCtx ctx;
		ctx.ctx.flags = 0;
		nr_matrix_d_set_identity (&ctx.i2doc);
		/* Set up viewport in case svg has it defined as percentages */
		ctx.vp.x0 = 0.0;
		ctx.vp.y0 = 0.0;
		ctx.vp.x1 = 21.0 / 2.54 * 72.0 * 1.25;
		ctx.vp.y1 = 29.7 / 2.54 * 72.0 * 1.25;
		nr_matrix_d_set_identity (&ctx.i2vp);
		sp_object_invoke_update (doc->root, (SPCtx *) &ctx, 0);
		/* if (doc->root->uflags & SP_OBJECT_MODIFIED_FLAG) return TRUE; */
	}

	/* Emit "modified" signal on objects */
	sp_object_invoke_modified (doc->root, 0);

#ifdef SP_DOCUMENT_DEBUG_IDLE
	g_print ("\n->");
#endif

	/* Emit our own "modified" signal */
	g_signal_emit (G_OBJECT (doc), signals [MODIFIED], 0,
			 SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG);

#ifdef SP_DOCUMENT_DEBUG_IDLE
	g_print (" S ->\n");
#endif

	repeat = (doc->root->uflags || doc->root->mflags);
	if (!repeat) doc->modified_id = 0;
	return repeat;
}

SPObject *
sp_document_add_repr (SPDocument *document, SPRepr *repr)
{
	GType type;

	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (repr != NULL, NULL);

	type = sp_repr_type_lookup (repr);

	if (g_type_is_a (type, SP_TYPE_ITEM)) {
		sp_repr_append_child (sp_document_repr_root(document), repr);
	} else if (g_type_is_a (type, SP_TYPE_OBJECT)) {
		sp_repr_append_child (SP_OBJECT_REPR (SP_DOCUMENT_DEFS(document)), repr);
	}

	return sp_document_lookup_id (document, sp_repr_attr (repr, "id"));
}

static int
is_within (const NRRectD *what, const NRRectF *box)
{
	return (box->x0 > what->x0) && (box->x1 < what->x1)
	    && (box->y0 > what->y0) && (box->y1 < what->y1);
}

static int
overlaps (const NRRectD *what, const NRRectF *box)
{
	return (((box->x0 > what->x0) && (box->x0 < what->x1)) ||
	        ((box->x1 > what->x0) && (box->x1 < what->x1))) &&
	       (((box->y0 > what->y0) && (box->y0 < what->y1)) ||
	        ((box->y1 > what->y0) && (box->y1 < what->y1)));
}

static GSList *
find_items_in_area (GSList *s, SPGroup *group, NRRectD *area,
                    int (*test)(const NRRectD *, const NRRectF *))
{
	SPObject * o;

	g_return_val_if_fail (SP_IS_GROUP (group), s);

	for ( o = group->children ; o != NULL ; o = o->next ) {
		if (!SP_IS_ITEM (o)) continue;
		if (SP_IS_GROUP (o) &&
		    SP_GROUP (o)->mode == SP_GROUP_MODE_LAYER)
		{
			s = find_items_in_area (s, SP_GROUP (o), area, test);
		} else {
			NRRectF box;
			SPItem * child = SP_ITEM (o);

			sp_item_bbox_desktop (child, &box);
			if (test (area, &box)) {
				s = g_slist_append (s, child);
			}
		}
	}

	return s;
}

/*
 * Return list of items, contained in box
 *
 * Assumes box is normalized (and g_asserts it!)
 *
 */

GSList *
sp_document_items_in_box (SPDocument *document, NRRectD *box)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);
	g_return_val_if_fail (box != NULL, NULL);

	return find_items_in_area (NULL, SP_GROUP (document->root),
	                           box, is_within);
}

/*
 * Return list of items, that the parts of the item contained in box
 *
 * Assumes box is normalized (and g_asserts it!)
 *
 */

GSList *
sp_document_partial_items_in_box (SPDocument *document, NRRectD *box)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);
	g_return_val_if_fail (box != NULL, NULL);

	return find_items_in_area (NULL, SP_GROUP (document->root),
	                           box, overlaps);
}

/* Resource management */

gboolean
sp_document_add_resource (SPDocument *document, const gchar *key, SPObject *object)
{
	GSList *rlist;

	g_return_val_if_fail (document != NULL, FALSE);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (object != NULL, FALSE);
	g_return_val_if_fail (SP_IS_OBJECT (object), FALSE);

	rlist = (GSList*)g_hash_table_lookup (document->priv->resources, key);
	g_return_val_if_fail (!g_slist_find (rlist, object), FALSE);
	rlist = g_slist_prepend (rlist, object);
	g_hash_table_insert (document->priv->resources, (gpointer) key, rlist);

	return TRUE;
}

gboolean
sp_document_remove_resource (SPDocument *document, const gchar *key, SPObject *object)
{
	GSList *rlist;

	g_return_val_if_fail (document != NULL, FALSE);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (object != NULL, FALSE);
	g_return_val_if_fail (SP_IS_OBJECT (object), FALSE);

	rlist = (GSList*)g_hash_table_lookup (document->priv->resources, key);
	g_return_val_if_fail (rlist != NULL, FALSE);
	g_return_val_if_fail (g_slist_find (rlist, object), FALSE);
	rlist = g_slist_remove (rlist, object);
	g_hash_table_insert (document->priv->resources, (gpointer) key, rlist);

	return TRUE;
}

const GSList *
sp_document_get_resource_list (SPDocument *document, const gchar *key)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (key != NULL, NULL);
	g_return_val_if_fail (*key != '\0', NULL);

	return (GSList*)g_hash_table_lookup (document->priv->resources, key);
}

/* Helpers */

gboolean
sp_document_resource_list_free (gpointer key, gpointer value, gpointer data)
{
	g_slist_free ((GSList *) value);
	return TRUE;
}

