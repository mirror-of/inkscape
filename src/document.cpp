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
#include <glib.h>
#include <string.h>
#include <gtk/gtkmain.h>
#include "xml/repr.h"
#include "xml/repr-action.h"
#include "helper/sp-marshal.h"
#include "helper/sp-intl.h"
#include "inkscape-private.h"
#include "inkscape_version.h"
#include "sp-object-repr.h"
#include "sp-root.h"
#include "sp-namedview.h"
#include "document-private.h"
#include "desktop.h"
#include "version.h"
#include "dir-util.h"

#include "display/nr-arena-item.h"
#include "display/nr-arena.h"
#include "display/canvas-arena.h"
#include "desktop-handles.h"

#define A4_WIDTH_STR "210mm"
#define A4_HEIGHT_STR "297mm"

#define SP_DOCUMENT_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE - 1)

#include <locale>
#include <sstream>

enum {
	MODIFIED,
	URI_SET,
	RESIZED,
	LAST_SIGNAL
};

static void sp_document_class_init(SPDocumentClass *klass);
static void sp_document_init(SPDocument *document);
static void sp_document_dispose(GObject *object);

static gint sp_document_idle_handler(gpointer data);

gboolean sp_document_resource_list_free(gpointer key, gpointer value, gpointer data);

static GObjectClass *parent_class;
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
			NULL
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPDocument", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_document_class_init(SPDocumentClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

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
free_id_signal(gpointer p)
{
	SigC::Signal1<void, SPObject *> *signal = reinterpret_cast<SigC::Signal1<void, SPObject *> *>(p);
	if (!signal->empty()) {
		g_warning("Lingering document id observers");
	}
	delete signal;
}

static void
sp_document_init (SPDocument *doc)
{
	SPDocumentPrivate *p;

	doc->advertize = FALSE;
	doc->keepalive = FALSE;
	doc->virgin    = TRUE;

	doc->modified_id = 0;

	doc->rdoc = NULL;
	doc->rroot = NULL;
	doc->root = NULL;

	doc->uri = NULL;
	doc->base = NULL;
	doc->name = NULL;

	doc->_collection_queue = NULL;

	p = g_new (SPDocumentPrivate, 1);

	p->iddef = g_hash_table_new (g_direct_hash, g_direct_equal);
	p->idsignals = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, free_id_signal);

	p->resources = g_hash_table_new (g_str_hash, g_str_equal);

	p->sensitive = FALSE;
	p->partial = NULL;
	p->history_size = 0;
	p->undo = NULL;
	p->redo = NULL;

	doc->priv = p;
}

void SPDocument::queueForOrphanCollection(SPObject *object) {
	g_return_if_fail(object != NULL);
	g_return_if_fail(SP_OBJECT_DOCUMENT(object) == this);

	sp_object_ref(object, NULL);
	_collection_queue = g_slist_prepend(_collection_queue, object);
}

void SPDocument::collectOrphans() {
	while (_collection_queue) {
		GSList *objects=_collection_queue;
		_collection_queue = NULL;
		for ( GSList *iter=objects ; iter ; iter = iter->next ) {
			SPObject *object=reinterpret_cast<SPObject *>(iter->data);
			object->collectOrphan();
			sp_object_unref(object, NULL);
		}
		g_slist_free(objects);
	}
}

/**
 *  This routine removes the given document object from the application,
 *  clearing the undo/redo information, releasing its root object, releasing
 *  other resources, and destructing its data members.
 */
static void
sp_document_dispose (GObject *object)
{
	SPDocument *doc = (SPDocument *) object;
	SPDocumentPrivate *priv = doc->priv;

	doc->collectOrphans();

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
		if (priv->idsignals) g_hash_table_destroy (priv->idsignals);

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

#ifndef WIN32
	if (uri) { // compute absolute path
		 char *full_path = (char *) g_malloc (1000);
		inkscape_rel2abs (uri, g_get_current_dir(), full_path, 1000);
		document->uri = g_strdup (full_path);
		g_free (full_path);
	} else {
		document->uri = NULL;
	}
#else
	//TODO:WIN32: program the windows equivalent of the above code for finding out 
	// the normalized absolute path of the file, including the drive letter
	document->uri = g_strdup (uri);
#endif

	// base is simply the part of the path before filename; e.g. when running "inkscape ../file.svg" the base is "../"
	// which is why we use g_get_current_dir() in calculating the abs path above
	document->base = g_strdup (base);
	document->name = g_strdup (name);

	document->root = sp_object_repr_build_tree (document, rroot);

	sodipodi_version = SP_ROOT (document->root)->version.sodipodi;

	/* fixme: Not sure about this, but lets assume ::build updates */
	sp_repr_set_attr (rroot, "sodipodi:version", SODIPODI_VERSION);
	sp_repr_set_attr (rroot, "inkscape:version", INKSCAPE_VERSION);
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

	// creating namedview
	if (!sp_item_group_get_child_by_name ((SPGroup *) document->root, NULL, "sodipodi:namedview")) {
		// if there's none in the document already,
		SPRepr *r = NULL;
		SPRepr *rnew = NULL;
		r = inkscape_get_repr (INKSCAPE, "template.base");
		// see if there's a template with id="base" in the preferences 
		if (!r) {
			// if there's none, create an empty element
			rnew = sp_repr_new ("sodipodi:namedview");
			sp_repr_set_attr (rnew, "id", "base");
		} else {
			// otherwise, take from preferences
			rnew = sp_repr_duplicate (r);
		}
		// insert into the document
		sp_repr_add_child (rroot, rnew, NULL);
		// clean up
		sp_repr_unref (rnew);
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

	// reset undo key when selection changes, so that same-key actions on different objects are not coalesced
	if (INKSCAPE != NULL) {
		g_signal_connect (G_OBJECT (INKSCAPE), "change_selection", 
				G_CALLBACK (sp_document_reset_key), document);
		g_signal_connect (G_OBJECT (INKSCAPE), "activate_desktop", 
				G_CALLBACK (sp_document_reset_key), document);
	}
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

	g_free (base);
	g_free (name);

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

gdouble sp_document_width(SPDocument *document)
{
	g_return_val_if_fail (document != NULL, 0.0);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), 0.0);
	g_return_val_if_fail (document->priv != NULL, 0.0);
	g_return_val_if_fail (document->root != NULL, 0.0);

	return SP_ROOT (document->root)->width.computed / 1.25;
}

gdouble sp_document_height(SPDocument *document)
{
	g_return_val_if_fail (document != NULL, 0.0);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), 0.0);
	g_return_val_if_fail (document->priv != NULL, 0.0);
	g_return_val_if_fail (document->root != NULL, 0.0);

	return SP_ROOT (document->root)->height.computed / 1.25;
}

void sp_document_set_uri(SPDocument *document, gchar const *uri)
{
	g_return_if_fail (document != NULL);
	g_return_if_fail (SP_IS_DOCUMENT (document));

	if (document->name) {
		g_free (document->name);
		document->name = NULL;
	}
	if (document->base) {
		g_free (document->base);
		document->base = NULL;
	}
	if (document->uri) {
		g_free (document->uri);
		document->uri = NULL;
	}

	if (uri) {

#ifndef WIN32
		// compute absolute path
		char *full_path;
		full_path = (char *) g_malloc (1000);
		inkscape_rel2abs (uri, g_get_current_dir(), full_path, 1000);
		document->uri = g_strdup (full_path);
		g_free (full_path);
#else
	//TODO:WIN32: program the windows equivalent of the above code for finding out 
	// the normalized absolute path of the file, including the drive letter
		document->uri = g_strdup (uri);
#endif
	
		/* fixme: Think, what this means for images (Lauris) */
		document->base = g_path_get_dirname (document->uri);
		document->name = g_path_get_basename (document->uri);

	} else {
		document->uri = g_strdup_printf (_("Unnamed document %d"), ++doc_count);
		document->base = NULL;
		document->name = g_strdup (document->uri);
	}

	// Update saveable repr attributes.
	SPRepr *repr = sp_document_repr_root(document);
	// changing uri in the document repr must not be not undoable
	sp_document_set_undo_sensitive (document, FALSE);
	sp_repr_set_attr (repr, "sodipodi:docbase", document->base);
	sp_repr_set_attr (repr, "sodipodi:docname", document->name);
	sp_document_set_undo_sensitive (document, TRUE);

	g_signal_emit (G_OBJECT (document), signals [URI_SET], 0, document->uri);
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

void sp_document_def_id(SPDocument *document, gchar const *id, SPObject *object)
{
	GQuark idq = g_quark_from_string(id);
	SigC::Signal1<void, SPObject *> *signal
	  = reinterpret_cast<SigC::Signal1<void, SPObject *> *>(g_hash_table_lookup(document->priv->idsignals,
										    GINT_TO_POINTER(idq)));

	if (object) {
		g_assert(g_hash_table_lookup(document->priv->iddef, GINT_TO_POINTER(idq)) == NULL);
		g_hash_table_insert(document->priv->iddef, GINT_TO_POINTER(idq), object);
	} else {
		g_assert(g_hash_table_lookup(document->priv->iddef, GINT_TO_POINTER(idq)) != NULL);
		g_hash_table_remove(document->priv->iddef, GINT_TO_POINTER(idq));
	}

	if (signal) {
		if (!signal->empty()) {
			signal->emit(object);
		} else {
			/* dispose of unused signal */
			g_hash_table_remove(document->priv->idsignals, GINT_TO_POINTER(idq));
			signal = NULL;
		}
	}
}

SigC::Connection
sp_document_id_changed_connect(SPDocument *doc, const gchar *id,
                               SigC::Slot1<void, SPObject *> slot)
{
	SigC::Signal1<void, SPObject *> *signal;
	GQuark idq;

	idq = g_quark_from_string(id);
	signal = reinterpret_cast<SigC::Signal1<void, SPObject *> *>(g_hash_table_lookup(doc->priv->idsignals, GINT_TO_POINTER(idq)));
	if (!signal) {
		signal = new SigC::Signal1<void, SPObject *>();
		g_hash_table_insert(doc->priv->idsignals, GINT_TO_POINTER(idq), reinterpret_cast<gpointer>(signal));
	}

	return signal->connect(slot);
}

SPObject *
sp_document_lookup_id (SPDocument *doc, const gchar *id)
{
	GQuark idq;

	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (doc), NULL);
	g_return_val_if_fail (id != NULL, NULL);

	idq = g_quark_from_string(id);

	return (SPObject*)g_hash_table_lookup (doc->priv->iddef, GINT_TO_POINTER(idq));
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
	lc = 32;
	while (doc->root->uflags || doc->root->mflags) {
		lc -= 1;
		if (lc < 0) {
			g_warning ("More than 32 iterations while updating document '%s'", doc->uri);
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
			nr_matrix_set_identity (&ctx.i2doc);
			/* Set up viewport in case svg has it defined as percentages */
			ctx.vp.x0 = 0.0;
			ctx.vp.y0 = 0.0;
			ctx.vp.x1 = 21.0 / 2.54 * 72.0 * 1.25;
			ctx.vp.y1 = 29.7 / 2.54 * 72.0 * 1.25;
			nr_matrix_set_identity (&ctx.i2vp);
			doc->root->updateDisplay((SPCtx *)&ctx, 0);
		}
		/* Emit "modified" signal on objects */
		doc->root->emitModified(0);
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
		nr_matrix_set_identity (&ctx.i2doc);
		/* Set up viewport in case svg has it defined as percentages */
		ctx.vp.x0 = 0.0;
		ctx.vp.y0 = 0.0;
		ctx.vp.x1 = 21.0 / 2.54 * 72.0 * 1.25;
		ctx.vp.y1 = 29.7 / 2.54 * 72.0 * 1.25;
		nr_matrix_set_identity (&ctx.i2vp);
		doc->root->updateDisplay((SPCtx *)&ctx, 0);
		/* if (doc->root->uflags & SP_OBJECT_MODIFIED_FLAG) return TRUE; */
	}

	/* Emit "modified" signal on objects */
	doc->root->emitModified(0);

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
is_within (const NRRect *what, const NRRect *box)
{
	return (box->x0 > what->x0) && (box->x1 < what->x1)
	    && (box->y0 > what->y0) && (box->y1 < what->y1);
}

static int
overlaps (const NRRect *what, const NRRect *box)
{
	return (((what->x0 > box->x0) && (what->x0 < box->x1)) ||
	        ((what->x1 > box->x0) && (what->x1 < box->x1))) &&
	       (((what->y0 > box->y0) && (what->y0 < box->y1)) ||
	        ((what->y1 > box->y0) && (what->y1 < box->y1)));
}

static GSList *
find_items_in_area (GSList *s, SPGroup *group, NRRect const *area,
                    int (*test)(const NRRect *, const NRRect *), bool take_insensitive = false)
{
	g_return_val_if_fail (SP_IS_GROUP (group), s);

	for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (!SP_IS_ITEM (o)) continue;
		if (SP_IS_GROUP (o) &&
		    SP_GROUP (o)->mode == SP_GROUP_MODE_LAYER)
		{
			s = find_items_in_area (s, SP_GROUP (o), area, test);
		} else {
			SPItem *child = SP_ITEM(o);
			NRRect box;
			sp_item_bbox_desktop (child, &box);
			if (test (area, &box) && (take_insensitive || child->sensitive)) {
				s = g_slist_append (s, child);
			}
		}
	}

	return s;
}

extern gdouble nr_arena_global_delta;

SPItem*
find_item_at_point (gint dkey, SPGroup *group, NR::Point const p, gboolean into_groups, bool take_insensitive = false)
{
	SPItem *seen = NULL, *newseen = NULL;

	for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (!SP_IS_ITEM (o)) continue;
		if (SP_IS_GROUP (o) && (SP_GROUP (o)->mode == SP_GROUP_MODE_LAYER || into_groups))	{
			// if nothing found yet, recurse into the group
			newseen = find_item_at_point (dkey, SP_GROUP (o), p, into_groups);
			if (newseen) {
				seen = newseen;
				newseen = NULL;
			}
		} else {
			SPItem *child = SP_ITEM(o);
			NRArenaItem *arenaitem = sp_item_get_arenaitem(child, dkey);

			// seen remembers the last (topmost) of items pickable at this point
			if (nr_arena_item_invoke_pick (arenaitem, p, nr_arena_global_delta, 1) != NULL 
                           && (take_insensitive || child->sensitive)) {
				seen = child;
			}
		}
	}
	return seen;
}

SPItem*
find_group_at_point (gint dkey, SPGroup *group, NR::Point const p)
{
	SPItem *seen = NULL;

	for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (!SP_IS_ITEM (o)) continue;
		if (SP_IS_GROUP (o) && SP_GROUP (o)->mode != SP_GROUP_MODE_LAYER) {
			SPItem *child = SP_ITEM(o);
			NRArenaItem *arenaitem = sp_item_get_arenaitem(child, dkey);

			// seen remembers the last (topmost) of groups pickable at this point
			if (nr_arena_item_invoke_pick (arenaitem, p, nr_arena_global_delta, 1) != NULL) {
				seen = child;
			}
		}
	}
	return seen;
}

/*
 * Return list of items, contained in box
 *
 * Assumes box is normalized (and g_asserts it!)
 *
 */

GSList *
sp_document_items_in_box (SPDocument *document, NRRect const *box)
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
sp_document_partial_items_in_box (SPDocument *document, NRRect const *box)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);
	g_return_val_if_fail (box != NULL, NULL);

	return find_items_in_area (NULL, SP_GROUP (document->root),
	                           box, overlaps);
}

SPItem*
sp_document_item_at_point (SPDocument *document, unsigned int key, NR::Point p, gboolean into_groups)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);

 	return find_item_at_point (key, SP_GROUP (document->root), p, into_groups);
}

SPItem*
sp_document_group_at_point (SPDocument *document, unsigned int key, NR::Point const p)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);

 	return find_group_at_point (key, SP_GROUP (document->root), p);
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


