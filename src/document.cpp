#define __SP_DOCUMENT_C__

/*
 * SVG document implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004-2005 MenTaLguY
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
#include "xml/sp-repr-action-fns.h"
#include "helper/sp-marshal.h"
#include <glibmm/i18n.h>
#include "inkscape-private.h"
#include "inkscape_version.h"
#include "sp-object-repr.h"
#include "sp-item-group.h"
#include "sp-root.h"
#include "sp-namedview.h"
#include "document-private.h"
#include "desktop.h"
#include "version.h"
#include "dir-util.h"
#include "unit-constants.h"

#include "display/nr-arena-item.h"
#include "display/nr-arena.h"
#include "display/canvas-arena.h"
#include "desktop-handles.h"

#include "dialogs/rdf.h"

#define A4_WIDTH_STR "210mm"
#define A4_HEIGHT_STR "297mm"

#define SP_DOCUMENT_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE - 1)

#include <locale>
#include <sstream>

static gint sp_document_idle_handler(gpointer data);

gboolean sp_document_resource_list_free(gpointer key, gpointer value, gpointer data);

static gint doc_count = 0;

SPDocument::SPDocument() {
	SPDocumentPrivate *p;

	advertize = FALSE;
	keepalive = FALSE;
	virgin    = TRUE;

	modified_id = 0;

	rdoc = NULL;
	rroot = NULL;
	root = NULL;

	uri = NULL;
	base = NULL;
	name = NULL;

	_collection_queue = NULL;

	p = new SPDocumentPrivate();

	p->iddef = g_hash_table_new (g_direct_hash, g_direct_equal);
	p->reprdef = g_hash_table_new (g_direct_hash, g_direct_equal);

	p->resources = g_hash_table_new (g_str_hash, g_str_equal);

	p->sensitive = FALSE;
	p->partial = NULL;
	p->history_size = 0;
	p->undo = NULL;
	p->redo = NULL;

	priv = p;
}

SPDocument::~SPDocument() {
	collectOrphans();

	if (priv) {
		inkscape_remove_document (this);

		if (priv->partial) {
			sp_repr_free_log (priv->partial);
			priv->partial = NULL;
		}

		sp_document_clear_redo (this);
		sp_document_clear_undo (this);

		if (root) {
			sp_object_invoke_release (root);
			g_object_unref (G_OBJECT (root));
			root = NULL;
		}

		if (priv->iddef) g_hash_table_destroy (priv->iddef);
		if (priv->reprdef) g_hash_table_destroy (priv->reprdef);

		if (rdoc) sp_repr_document_unref (rdoc);

		/* Free resources */
		g_hash_table_foreach_remove (priv->resources, sp_document_resource_list_free, this);
		g_hash_table_destroy (priv->resources);

		delete priv;
		priv = NULL;
	}

	if (name) {
		g_free (name);
		name = NULL;
	}
	if (base) {
		g_free (base);
		base = NULL;
	}
	if (uri) {
		g_free (uri);
		uri = NULL;
	}

	if (modified_id) {
		gtk_idle_remove (modified_id);
		modified_id = 0;
	}

	if (keepalive) {
		inkscape_unref ();
		keepalive = FALSE;
	}
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

	document = new SPDocument();

	document->advertize = advertize;
	document->keepalive = keepalive;

	document->rdoc = rdoc;
	document->rroot = rroot;

#ifndef WIN32
	prepend_current_dir_if_relative (&(document->uri), uri);
#else
	// FIXME: it may be that prepend_current_dir_if_relative works OK on windows too, test!
	document->uri = uri? g_strdup (uri) : NULL;
#endif

	// base is simply the part of the path before filename; e.g. when running "inkscape ../file.svg" the base is "../"
	// which is why we use g_get_current_dir() in calculating the abs path above
        //This is NULL for a new document
	if (base)
            document->base = g_strdup (base);
        else
            document->base = NULL;
	document->name = g_strdup (name);

	document->root = sp_object_repr_build_tree (document, rroot);

	sodipodi_version = SP_ROOT (document->root)->version.sodipodi;

	/* fixme: Not sure about this, but lets assume ::build updates */
	sp_repr_set_attr (rroot, "sodipodi:version", SODIPODI_VERSION);
	sp_repr_set_attr (rroot, "inkscape:version", INKSCAPE_VERSION);
	/* fixme: Again, I moved these here to allow version determining in ::build (Lauris) */

	/* Quick hack 2 - get default image size into document */
	if (!sp_repr_attr (rroot, "width")) sp_repr_set_attr (rroot, "width", A4_WIDTH_STR);
	if (!sp_repr_attr (rroot, "height")) sp_repr_set_attr (rroot, "height", A4_HEIGHT_STR);
        /* End of quick hack 2 */

	/* Quick hack 3 - Set uri attributes */
	if (uri) {
		/* fixme: Think, what this means for images (Lauris) */
		sp_repr_set_attr (rroot, "sodipodi:docname", uri);
		if (document->base)
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
		r = sp_repr_new ("svg:defs");
		sp_repr_add_child (rroot, r, NULL);
		sp_repr_unref (r);
		g_assert (SP_ROOT (document->root)->defs);
	}

	/* Default RDF */
	rdf_set_defaults ( document );

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
sp_document_new (const gchar *uri, unsigned int advertize, unsigned int keepalive, bool make_new)
{
	SPDocument *doc;
	SPReprDoc *rdoc;
	gchar *base = NULL;
        gchar *name = NULL;

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
		if (strcmp (sp_repr_name (rroot), "svg:svg") != 0) return NULL;
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
		rdoc = sp_repr_document_new ("svg:svg");
	}

	if (make_new) {
		base = NULL;
		uri = NULL;
		name = g_strdup_printf (_("New document %d"), ++doc_count);
	}

        //# These should be set by now
        g_assert (name);

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
	if (strcmp (sp_repr_name (rroot), "svg:svg") != 0) return NULL;

	name = g_strdup_printf (_("Memory document %d"), ++doc_count);

	doc = sp_document_create (rdoc, NULL, NULL, name, advertize, keepalive);

	return doc;
}

SPDocument *sp_document_new_dummy() {
	SPDocument *document = new SPDocument();
	inkscape_add_document(document);
	return document;
}

SPDocument *
sp_document_ref (SPDocument *doc)
{
	g_return_val_if_fail (doc != NULL, NULL);
	Inkscape::GC::anchor(doc);
	return doc;
}

SPDocument *
sp_document_unref (SPDocument *doc)
{
	g_return_val_if_fail (doc != NULL, NULL);
	Inkscape::GC::release(doc);
	return NULL;
}

gdouble sp_document_width(SPDocument *document)
{
	g_return_val_if_fail (document != NULL, 0.0);
	g_return_val_if_fail (document->priv != NULL, 0.0);
	g_return_val_if_fail (document->root != NULL, 0.0);

	return SP_ROOT (document->root)->width.computed;
}

gdouble sp_document_height(SPDocument *document)
{
	g_return_val_if_fail (document != NULL, 0.0);
	g_return_val_if_fail (document->priv != NULL, 0.0);
	g_return_val_if_fail (document->root != NULL, 0.0);

	return SP_ROOT (document->root)->height.computed;
}

void sp_document_set_uri(SPDocument *document, gchar const *uri)
{
	g_return_if_fail (document != NULL);

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
		prepend_current_dir_if_relative (&(document->uri), uri);
#else
		// FIXME: it may be that prepend_current_dir_if_relative works OK on windows too, test!
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
	gboolean saved = sp_document_get_undo_sensitive(document);
	sp_document_set_undo_sensitive (document, FALSE);
	if (document->base)
            sp_repr_set_attr (repr, "sodipodi:docbase", document->base);

	sp_repr_set_attr (repr, "sodipodi:docname", document->name);
	sp_document_set_undo_sensitive (document, saved);

	document->priv->uri_set_signal.emit(document->uri);
}

void
sp_document_set_size_px (SPDocument *doc, gdouble width, gdouble height)
{
	g_return_if_fail (doc != NULL);

	doc->priv->resized_signal.emit(width, height);
}

sigc::connection SPDocument::connectModified(SPDocument::ModifiedSignal::slot_type slot)
{
	return priv->modified_signal.connect(slot);
}

sigc::connection SPDocument::connectURISet(SPDocument::URISetSignal::slot_type slot)
{
	return priv->uri_set_signal.connect(slot);
}

sigc::connection SPDocument::connectResized(SPDocument::ResizedSignal::slot_type slot)
{
	return priv->resized_signal.connect(slot);
}

void SPDocument::_emitModified() {
	static const guint flags = SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG;
	root->emitModified(0);
	priv->modified_signal.emit(flags);
}

void SPDocument::bindObjectToId(gchar const *id, SPObject *object) {
	GQuark idq = g_quark_from_string(id);

	if (object) {
		g_assert(g_hash_table_lookup(priv->iddef, GINT_TO_POINTER(idq)) == NULL);
		g_hash_table_insert(priv->iddef, GINT_TO_POINTER(idq), object);
	} else {
		g_assert(g_hash_table_lookup(priv->iddef, GINT_TO_POINTER(idq)) != NULL);
		g_hash_table_remove(priv->iddef, GINT_TO_POINTER(idq));
	}

	SPDocumentPrivate::IDChangedSignalMap::iterator pos;

	pos = priv->id_changed_signals.find(idq);
	if ( pos != priv->id_changed_signals.end() ) {
		if (!(*pos).second.empty()) {
			(*pos).second.emit(object);
		} else { // discard unused signal
			priv->id_changed_signals.erase(pos);
		}
	}
}

SPObject *SPDocument::getObjectById(const gchar *id) {
	g_return_val_if_fail (id != NULL, NULL);

	GQuark idq = g_quark_from_string(id);
	return (SPObject*)g_hash_table_lookup (priv->iddef, GINT_TO_POINTER(idq));
}

sigc::connection SPDocument::connectIdChanged(const gchar *id, SPDocument::IDChangedSignal::slot_type slot) {
	return priv->id_changed_signals[g_quark_from_string(id)].connect(slot);
}

void SPDocument::bindObjectToRepr(SPRepr *repr, SPObject *object) {
	if (object) {
		g_assert(g_hash_table_lookup(priv->reprdef, repr) == NULL);
		g_hash_table_insert(priv->reprdef, repr, object);
	} else {
		g_assert(g_hash_table_lookup(priv->reprdef, repr) != NULL);
		g_hash_table_remove(priv->reprdef, repr);
	}
}

SPObject *SPDocument::getObjectByRepr(SPRepr *repr) {
	g_return_val_if_fail (repr != NULL, NULL);
	return (SPObject*)g_hash_table_lookup(priv->reprdef, repr);
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
			ctx.vp.x1 = 21.0 * PX_PER_CM; 
			ctx.vp.y1 = 29.7 * PX_PER_CM;
			nr_matrix_set_identity (&ctx.i2vp);
			doc->root->updateDisplay((SPCtx *)&ctx, 0);
		}
		doc->_emitModified();
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

	doc = static_cast<SPDocument *>(data);

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
		ctx.vp.x1 = 21.0 * PX_PER_CM;
		ctx.vp.y1 = 29.7 * PX_PER_CM;
		nr_matrix_set_identity (&ctx.i2vp);

		gboolean saved = sp_document_get_undo_sensitive(doc);
		sp_document_set_undo_sensitive (doc, FALSE);

		doc->root->updateDisplay((SPCtx *)&ctx, 0);

		sp_document_set_undo_sensitive (doc, saved);
		/* if (doc->root->uflags & SP_OBJECT_MODIFIED_FLAG) return TRUE; */
	}

	doc->_emitModified();

	repeat = (doc->root->uflags || doc->root->mflags);
	if (!repeat) doc->modified_id = 0;
	return repeat;
}

static int
is_within (const NRRect *area, const NRRect *box)
{
	if (box->x0 > box->x1 || box->y0 > box->y1) // invalid (=empty) bbox, may happen e.g. with a whitespace-only text object
		return false;

	return (box->x0 > area->x0) && (box->x1 < area->x1)
	    && (box->y0 > area->y0) && (box->y1 < area->y1);
}

static int
overlaps (const NRRect *area, const NRRect *box)
{
	if (box->x0 > box->x1 || box->y0 > box->y1) // invalid (=empty) bbox, may happen e.g. with a whitespace-only text object
		return false;

	return (((area->x0 > box->x0) && (area->x0 < box->x1)) ||
	        ((area->x1 > box->x0) && (area->x1 < box->x1))) &&
	       (((area->y0 > box->y0) && (area->y0 < box->y1)) ||
	        ((area->y1 > box->y0) && (area->y1 < box->y1)));
}

static GSList *
find_items_in_area (GSList *s, SPGroup *group, unsigned int dkey, NRRect const *area,
                    int (*test)(const NRRect *, const NRRect *), bool take_insensitive = false)
{
	g_return_val_if_fail (SP_IS_GROUP (group), s);

	for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (!SP_IS_ITEM (o)) continue;
		if (SP_IS_GROUP (o) &&
		    SP_GROUP (o)->effectiveLayerMode(dkey) == SPGroup::LAYER )
		{
			s = find_items_in_area(s, SP_GROUP(o), dkey, area, test);
		} else {
			SPItem *child = SP_ITEM(o);
			NRRect box;
			sp_item_bbox_desktop (child, &box);
			if (test (area, &box) && (take_insensitive || child->isVisibleAndUnlocked(dkey))) {
				s = g_slist_append (s, child);
			}
		}
	}

	return s;
}

extern gdouble nr_arena_global_delta;

/**
Returns true if an item is among the descendants of group (recursively).
 */
bool item_is_in_group (SPItem *item, SPGroup *group)
{
	for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (!SP_IS_ITEM (o)) continue;
		if (SP_ITEM (o) == item)
			return true;
		if (SP_IS_GROUP (o))
			if (item_is_in_group (item, SP_GROUP(o)))
				return true;
	}
	return false;
}

/**
Returns the bottommost item from the list which is at the point, or NULL if none. 
*/
SPItem*
sp_document_item_from_list_at_point_bottom (unsigned int dkey, SPGroup *group, const GSList *list, NR::Point const p, bool take_insensitive)
{
	g_return_val_if_fail (group, NULL);

	for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {

		if (!SP_IS_ITEM (o)) continue;

		SPItem *item = SP_ITEM (o);
		NRArenaItem *arenaitem = sp_item_get_arenaitem(item, dkey);
		if (nr_arena_item_invoke_pick (arenaitem, p, nr_arena_global_delta, 1) != NULL 
                      && (take_insensitive || item->isVisibleAndUnlocked(dkey))) {
			if (g_slist_find((GSList *) list, item) != NULL)
				return item;
		}

		if (SP_IS_GROUP (o)) {
			SPItem *found = sp_document_item_from_list_at_point_bottom (dkey, SP_GROUP(o), list, p, take_insensitive);
			if (found)
				return found;
		}

	}
	return NULL;
}

/**
Returns the topmost (in z-order) item from the descendants of group (recursively) which
is at the point p, or NULL if none. Honors into_groups on whether to recurse into
non-layer groups or not. Honors take_insensitive on whether to return insensitive
items. If upto != NULL, then if item upto is encountered (at any level), stops searching
upwards in z-order and returns what it has found so far (i.e. the found item is
guaranteed to be lower than upto).
 */
SPItem*
find_item_at_point (unsigned int dkey, SPGroup *group, NR::Point const p, gboolean into_groups, bool take_insensitive = false, SPItem *upto = NULL)
{
	SPItem *seen = NULL, *newseen = NULL;

	for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (!SP_IS_ITEM (o)) continue;

		if (upto && SP_ITEM (o) == upto)
			break;

		if (SP_IS_GROUP (o) && (SP_GROUP (o)->effectiveLayerMode(dkey) == SPGroup::LAYER || into_groups))	{
			// if nothing found yet, recurse into the group
			newseen = find_item_at_point (dkey, SP_GROUP (o), p, into_groups, take_insensitive, upto);
			if (newseen) {
				seen = newseen;
				newseen = NULL;
			}

			if (item_is_in_group (upto, SP_GROUP (o)))
				break;

		} else {
			SPItem *child = SP_ITEM(o);
			NRArenaItem *arenaitem = sp_item_get_arenaitem(child, dkey);

			// seen remembers the last (topmost) of items pickable at this point
			if (nr_arena_item_invoke_pick (arenaitem, p, nr_arena_global_delta, 1) != NULL 
                           && (take_insensitive || child->isVisibleAndUnlocked(dkey))) {
				seen = child;
			}
		}
	}
	return seen;
}

/**
Returns the topmost non-layer group from the descendants of group which is at point
p, or NULL if none. Recurses into layers but not into groups.
 */
SPItem*
find_group_at_point (unsigned int dkey, SPGroup *group, NR::Point const p)
{
	SPItem *seen = NULL;

	for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (!SP_IS_ITEM (o)) continue;
		if (SP_IS_GROUP (o) && SP_GROUP (o)->effectiveLayerMode(dkey) == SPGroup::LAYER) {
			SPItem *newseen = find_group_at_point (dkey, SP_GROUP (o), p);
			if (newseen) {
				seen = newseen;
			}
		}
		if (SP_IS_GROUP (o) && SP_GROUP (o)->effectiveLayerMode(dkey) != SPGroup::LAYER ) {
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
sp_document_items_in_box (SPDocument *document, unsigned int dkey, NRRect const *box)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);
	g_return_val_if_fail (box != NULL, NULL);

	return find_items_in_area (NULL, SP_GROUP (document->root),
	                           dkey, box, is_within);
}

/*
 * Return list of items, that the parts of the item contained in box
 *
 * Assumes box is normalized (and g_asserts it!)
 *
 */

GSList *
sp_document_partial_items_in_box (SPDocument *document, unsigned int dkey, NRRect const *box)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);
	g_return_val_if_fail (box != NULL, NULL);

	return find_items_in_area (NULL, SP_GROUP (document->root),
	                           dkey, box, overlaps);
}

SPItem *
sp_document_item_at_point(SPDocument *document, unsigned const key, NR::Point const p,
			  gboolean const into_groups, SPItem *upto)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);

 	return find_item_at_point (key, SP_GROUP (document->root), p, into_groups, false, upto);
}

SPItem*
sp_document_group_at_point (SPDocument *document, unsigned int key, NR::Point const p)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (document->priv != NULL, NULL);

 	return find_group_at_point (key, SP_GROUP (document->root), p);
}


/* Resource management */

gboolean
sp_document_add_resource (SPDocument *document, const gchar *key, SPObject *object)
{
	GSList *rlist;
	GQuark q=g_quark_from_string(key);

	g_return_val_if_fail (document != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (object != NULL, FALSE);
	g_return_val_if_fail (SP_IS_OBJECT (object), FALSE);

	rlist = (GSList*)g_hash_table_lookup (document->priv->resources, key);
	g_return_val_if_fail (!g_slist_find (rlist, object), FALSE);
	rlist = g_slist_prepend (rlist, object);
	g_hash_table_insert (document->priv->resources, (gpointer) key, rlist);

	document->priv->resources_changed_signals[q].emit();

	return TRUE;
}

gboolean
sp_document_remove_resource (SPDocument *document, const gchar *key, SPObject *object)
{
	GSList *rlist;
	GQuark q=g_quark_from_string(key);

	g_return_val_if_fail (document != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (object != NULL, FALSE);
	g_return_val_if_fail (SP_IS_OBJECT (object), FALSE);

	rlist = (GSList*)g_hash_table_lookup (document->priv->resources, key);
	g_return_val_if_fail (rlist != NULL, FALSE);
	g_return_val_if_fail (g_slist_find (rlist, object), FALSE);
	rlist = g_slist_remove (rlist, object);
	g_hash_table_insert (document->priv->resources, (gpointer) key, rlist);

	document->priv->resources_changed_signals[q].emit();

	return TRUE;
}

const GSList *
sp_document_get_resource_list (SPDocument *document, const gchar *key)
{
	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);
	g_return_val_if_fail (*key != '\0', NULL);

	return (GSList*)g_hash_table_lookup (document->priv->resources, key);
}

sigc::connection sp_document_resources_changed_connect(
	SPDocument *document,
	const gchar *key,
	SPDocument::ResourcesChangedSignal::slot_type slot
) {
	GQuark q=g_quark_from_string(key);
	return document->priv->resources_changed_signals[q].connect(slot);
}

/* Helpers */

gboolean
sp_document_resource_list_free (gpointer key, gpointer value, gpointer data)
{
	g_slist_free ((GSList *) value);
	return TRUE;
}


