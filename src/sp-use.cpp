#define __SP_USE_C__

/*
 * SVG <svg> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <glib-object.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include "helper/sp-intl.h"
#include "svg/svg.h"
#include "display/nr-arena-group.h"
#include "attributes.h"
#include "document.h"
#include "sp-object-repr.h"
#include "uri-references.h"
#include "macros.h"
#include "xml/repr.h"
#include "xml/repr-private.h"
#include "sp-item.h"
#include "enums.h"
#include "prefs-utils.h"

#include "sp-use.h"

/* fixme: */
#include "desktop-events.h"

static void sp_use_class_init (SPUseClass *classname);
static void sp_use_init (SPUse *use);

static void sp_use_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_use_release (SPObject *object);
static void sp_use_set (SPObject *object, unsigned int key, const gchar *value);
static SPRepr *sp_use_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_use_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_use_modified (SPObject *object, guint flags);

static void sp_use_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags);
static void sp_use_print (SPItem *item, SPPrintContext *ctx);
static gchar * sp_use_description (SPItem * item);
static NRArenaItem *sp_use_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_use_hide (SPItem *item, unsigned int key);

//static void sp_use_href_changed (SPUse * use);
static void sp_use_href_changed (SPObject *old_ref, SPObject *ref, SPUse * use);

static SPItemClass * parent_class;

//void mm_print (gchar *say, NR::Matrix m)
//{ g_print ("%s %g %g %g %g %g %g\n", say, m[0], m[1], m[2], m[3], m[4], m[5]); }

GType
sp_use_get_type (void)
{
	static GType use_type = 0;
	if (!use_type) {
		GTypeInfo use_info = {
			sizeof (SPUseClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_use_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPUse),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_use_init,
			NULL,	/* value_table */
		};
		use_type = g_type_register_static (SP_TYPE_ITEM, "SPUse", &use_info, (GTypeFlags)0);
	}
	return use_type;
}

static void
sp_use_class_init (SPUseClass *classname)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	gobject_class = (GObjectClass *) classname;
	sp_object_class = (SPObjectClass *) classname;
	item_class = (SPItemClass *) classname;

	parent_class = (SPItemClass*)g_type_class_ref (SP_TYPE_ITEM);

	sp_object_class->build = sp_use_build;
	sp_object_class->release = sp_use_release;
	sp_object_class->set = sp_use_set;
	sp_object_class->write = sp_use_write;
	sp_object_class->update = sp_use_update;
	sp_object_class->modified = sp_use_modified;

	item_class->bbox = sp_use_bbox;
	item_class->description = sp_use_description;
	item_class->print = sp_use_print;
	item_class->show = sp_use_show;
	item_class->hide = sp_use_hide;
}

static void
sp_use_init (SPUse * use)
{
	use->ref = new SPUseReference(SP_OBJECT(use));
	use->ref->changedSignal().connect(SigC::bind(SigC::slot(sp_use_href_changed), use));

	sp_svg_length_unset (&use->x, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&use->y, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&use->width, SP_SVG_UNIT_PERCENT, 1.0, 1.0);
	sp_svg_length_unset (&use->height, SP_SVG_UNIT_PERCENT, 1.0, 1.0);
	use->href = NULL;
	use->original_repr_changed = false;
}

static void
sp_use_build (SPObject * object, SPDocument * document, SPRepr * repr)
{
	if (((SPObjectClass *) parent_class)->build) {
		(* ((SPObjectClass *) parent_class)->build) (object, document, repr);
	}

	sp_object_read_attr (object, "x");
	sp_object_read_attr (object, "y");
	sp_object_read_attr (object, "width");
	sp_object_read_attr (object, "height");
	sp_object_read_attr (object, "xlink:href");

	// We don't need to create child here: 
	// reading xlink:href will attach ref, and that will cause the changed signal to be emitted,
	// which will call sp_use_href_changed, and that will take care of the child
}

static void
sp_use_release (SPObject *object)
{
	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);

	SPUse *use=SP_USE (object);

	use->child = NULL;

	g_free (use->href);

	if (use->ref) {
		use->ref->detach();
		delete use->ref;
		use->ref = NULL;
	}
}

static void
sp_use_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPUse *use;

	use = SP_USE (object);

	switch (key) {
	case SP_ATTR_X:
		if (!sp_svg_length_read (value, &use->x)) {
			sp_svg_length_unset (&use->x, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_Y:
		if (!sp_svg_length_read (value, &use->y)) {
			sp_svg_length_unset (&use->y, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_WIDTH:
		if (!sp_svg_length_read (value, &use->width)) {
			sp_svg_length_unset (&use->width, SP_SVG_UNIT_PERCENT, 1.0, 1.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_HEIGHT:
		if (!sp_svg_length_read (value, &use->height)) {
			sp_svg_length_unset (&use->height, SP_SVG_UNIT_PERCENT, 1.0, 1.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_XLINK_HREF: {
		if (value) {
			// first, set the href field, because sp_use_href_changed will need it
			if (use->href) {
				if (strcmp (value, use->href) == 0) 
					break;
				g_free (use->href);
				use->href = g_strdup (value);
			} else {
				use->href = g_strdup (value);
			}
			// now do the attaching, which emits the changed signal
			try {
				use->ref->attach(Inkscape::URI(value));
			} catch (Inkscape::BadURIException &e) {
				g_warning("%s", e.what());
				use->ref->detach();
			}
		} else {
			if (use->href) {
				g_free (use->href);
				use->href = NULL;
			}
			use->ref->detach();
		}
		break;
	}
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static SPRepr *
sp_use_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPUse *use;

	use = SP_USE (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("use");
	}

	sp_repr_set_attr (repr, "id", object->id);
	sp_repr_set_double (repr, "x", use->x.computed);
	sp_repr_set_double (repr, "y", use->y.computed);
	sp_repr_set_double (repr, "width", use->width.computed);
	sp_repr_set_double (repr, "height", use->height.computed);

	if (use->ref->getURI()) {
		gchar *uri_string = use->ref->getURI()->toString();
		sp_repr_set_attr(repr, "xlink:href", uri_string);
		g_free(uri_string);
	}

	return repr;
}

static void
sp_use_bbox (SPItem *item, NRRect *bbox, const NRMatrix *transform, unsigned int flags)
{
	SPUse * use;

	use = SP_USE (item);

	if (use->child && SP_IS_ITEM (use->child)) {
		SPItem *child;
		NRMatrix ct, t;
		child = SP_ITEM (use->child);
		nr_matrix_set_translate (&t, use->x.computed, use->y.computed);
		nr_matrix_multiply (&ct, &t, transform);
		nr_matrix_multiply (&ct, &child->transform, &ct);
		sp_item_invoke_bbox_full (SP_ITEM (use->child), bbox, &ct, flags, FALSE);
	}
}

static void
sp_use_print (SPItem *item, SPPrintContext *ctx)
{
	SPUse * use;

	use = SP_USE (item);

	if (use->child && SP_IS_ITEM (use->child)) {
		sp_item_invoke_print (SP_ITEM (use->child), ctx);
	}
}

static gchar *
sp_use_description (SPItem * item)
{
	SPUse * use;

	use = SP_USE (item);

	if (use->child) 
		return g_strdup_printf (_("Clone of: %s. Use Shift+D to look up original"), sp_item_description (SP_ITEM (use->child))); 

	return g_strdup (_("Orphaned clone"));
}

static NRArenaItem *
sp_use_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPUse *use;

	use = SP_USE (item);

	if (use->child) {
		NRArenaItem *ai, *ac;
		NRMatrix t;
		ai = nr_arena_item_new (arena, NR_TYPE_ARENA_GROUP);
		nr_arena_group_set_transparent (NR_ARENA_GROUP (ai), FALSE);
		ac = sp_item_invoke_show (SP_ITEM (use->child), arena, key, flags);
		if (ac) {
			nr_arena_item_add_child (ai, ac, NULL);
			nr_arena_item_unref (ac);
		}
		nr_matrix_set_translate (&t, use->x.computed, use->y.computed);
		nr_arena_group_set_child_transform (NR_ARENA_GROUP (ai), &t);
		return ai;
	}
		
	return NULL;
}

static void
sp_use_hide (SPItem * item, unsigned int key)
{
	SPUse * use;

	use = SP_USE (item);

	if (use->child) sp_item_invoke_hide (SP_ITEM (use->child), key);

	if (((SPItemClass *) parent_class)->hide)
		((SPItemClass *) parent_class)->hide (item, key);
}

/**
Returns the ultimate original of a SPUse (i.e. the first object in the chain of its originals which is not an SPUse).
Note that the returned is the clone object, i.e. the child of an SPUse (of the argument one for the trivial case) and not the "true original".
*/
SPItem *
sp_use_root (SPUse *use)
{
	SPObject *orig = use->child;
	while (SP_IS_USE(orig)) {
		orig = SP_USE(orig)->child;
	}
	g_assert (SP_IS_ITEM (orig));
	return SP_ITEM (orig);
}

/**
Returns the effective transform that goes from the ultimate original to given SPUse, both ends included
 */
NR::Matrix
sp_use_get_root_transform (SPUse *use)
{
	//track the ultimate source of a chain of uses
	SPObject *orig = use->child;
	GSList *chain = NULL;
	chain = g_slist_prepend (chain, use);
	while (SP_IS_USE(orig)) {
		chain = g_slist_prepend (chain, orig);
		orig = SP_USE(orig)->child;
	}
	chain = g_slist_prepend (chain, orig);


	//calculate the accummulated transform, starting from the original
	NR::Matrix t;
	t.set_identity();
	for (GSList *i = chain; i != NULL; i = i->next) {
		SPItem *i_tem = SP_ITEM(i->data);

		// "An additional transformation translate(x,y) is appended to the end (i.e.,
		// right-side) of the transform attribute on the generated 'g', where x and y
		// represent the values of the x and y attributes on the 'use' element." - http://www.w3.org/TR/SVG11/struct.html#UseElement
		if (SP_IS_USE(i_tem)) {
			SPUse *i_use = SP_USE(i_tem);
			if ((i_use->x.set && i_use->x.computed != 0) || (i_use->y.set && i_use->y.computed != 0)) {
				t = t * NR::translate (i_use->x.set ? i_use->x.computed : 0, i_use->y.set ? i_use->y.computed : 0);
			}
		}

		t = t * NR::Matrix (&i_tem->transform);
	}

	g_slist_free (chain);
	return t;
}

/**
Returns the transform that leads to the use from its immediate original.
Does not inlcude the original's transform if any.
 */
NR::Matrix
sp_use_get_parent_transform (SPUse *use)
{
	NR::Matrix t;
	t.set_identity();

	if ((use->x.set && use->x.computed != 0) || (use->y.set && use->y.computed != 0)) {
		t = t * NR::translate (use->x.set ? use->x.computed : 0, use->y.set ? use->y.computed : 0);
	}

	t = t * NR::Matrix (&(SP_ITEM(use)->transform));
	return t;
}


using NR::X;
using NR::Y;

inline bool point_equalp(NR::Point const &a, NR::Point const &b)
{
	return ( NR_DF_TEST_CLOSE(a[X], b[X], 1e-5) &&
		 NR_DF_TEST_CLOSE(a[Y], b[Y], 1e-5) );
}

/**
Sensing a movement of the original, this function attempts to compensate for it in such a way
that the clone stays unmoved or moves in parallel (depending on user setting) regardless of the clone's transform.
*/
static void sp_use_move_compensation (SPUse *use)
{
	guint mode = prefs_get_int_attribute ("options.clonecompensation", "value", SP_CLONE_COMPENSATION_PARALLEL);

	// user wants no compensation
	if (mode == SP_CLONE_COMPENSATION_NONE)
		return;

	SPItem *ref = use->ref->getObject();

	// the clone is orphaned; or this is not a real use, but a clone of another use; 
	// we skip it, otherwise duplicate compensation will occur
	if (!ref || SP_OBJECT_IS_CLONED (use)) {
		return;
	}

	//	g_print ("=====%s, %p\n", sp_repr_attr(SP_OBJECT_REPR (use), "id"), use);

	// parent's update was not accompanied by writing to its repr (e.g. selector drag before mouse release).
	// in this case, we do not want any compensation yet.
	if (!use->original_repr_changed) {
		return;
	}

	// find out the current bbox of the original
	NRRect bbox;
	NRMatrix i2doc;
	sp_item_i2doc_affine(ref, &i2doc);
	sp_item_invoke_bbox(ref, &bbox, &i2doc, TRUE);
	
	// remember it
	NR::Rect n = bbox;
	NR::Rect o = use->original;
	use->original = n;

	// 	mm_print ("old", NR::Matrix (NR::translate (o.midpoint())));
	// 	mm_print ("new", NR::Matrix (NR::translate (n.midpoint())));

	// 	mm_print ("dim diff", NR::Matrix (NR::translate (n.dimensions() - o.dimensions())));
	// 	mm_print ("mid diff", NR::Matrix (NR::translate (n.midpoint() - o.midpoint())));

	// if dimensions changed, this is not pure move, so quit
	if (!point_equalp (n.dimensions(), o.dimensions())) {
		return;
	}

	// find out midpoints
	NR::Point np = (n.midpoint());
	NR::Point op = (o.midpoint());

	// if midpoint is the same, no need to compensate, so quit
	if (point_equalp (op, np)) {
		return;
	}

	// 	mm_print ("par", NR::Matrix (NR::translate (op - np)));

	NR::Point original_move (np - op);

	// transform both old and new midpoints
	NR::Matrix t = sp_use_get_parent_transform (use);
	np *= t;
	op *= t;

	// np - op is how the clone would move, if we allow it to
	NRMatrix clone_move;
	if (mode == SP_CLONE_COMPENSATION_PARALLEL)
		clone_move = NR::Matrix (NR::translate (op - np + original_move));
	else 
		clone_move = NR::Matrix (NR::translate (op - np));

	// commit the compensation
	SPItem *item = SP_ITEM(use);
	nr_matrix_multiply (&item->transform, &item->transform, &clone_move);
	sp_item_write_transform (item, SP_OBJECT_REPR (item), &item->transform);

	// ugly hack: with chained <use>s, the transform= may not change at all, 
	// therefore the above write_transform may fail to change the repr 
	// and the parent_repr_changed flag on its clones will not be set. 
	// So we set and remove a dummy attr to assert that we did indeed change the repr.
	sp_repr_set_attr (SP_OBJECT_REPR (item), "inkscape:dummy", "1");
	sp_repr_set_attr (SP_OBJECT_REPR (item), "inkscape:dummy", NULL);

	// having changed the repr, we need to finish transaction.
	// FIXME: we should really try to sneak under the previous transaction, not create a new one.
	sp_document_done(SP_OBJECT_DOCUMENT (item));

	// no more compensation until parent's repr changes again
	use->original_repr_changed = false;
}

static void sp_use_original_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive, gpointer data)
{
	SPUse *use = SP_USE (data);
	// set the flag telling the _compensation function that the original's repr changed
	use->original_repr_changed = true;
}

static SPReprEventVector use_repr_events = { 	 
         NULL, /* destroy */ 	 
         NULL, /* add_child */ 	 
         NULL, /* child_added */ 	 
         NULL, /* remove_child */ 	 
         NULL, /* child_removed */ 	 
         NULL, /* change_attr */ 	 
         sp_use_original_attr_changed, 	 
         NULL, /* change_list */ 	 
         NULL, /* content_changed */ 	 
         NULL, /* change_order */ 	 
         NULL  /* order_changed */ 	 
 };

static void
sp_use_href_changed (SPObject *old_ref, SPObject *ref, SPUse * use)
{
	SPItem *item = SP_ITEM (use);

	if (use->child) {
		sp_object_detach_unref (SP_OBJECT (use), use->child);
		use->child = NULL;
	}

	if (use->href) {
		SPItem *refobj = use->ref->getObject();
		if (refobj) {
			SPRepr *childrepr = SP_OBJECT_REPR (refobj);
			GType type = sp_repr_type_lookup (childrepr);
			g_return_if_fail (type > G_TYPE_NONE);
			if (g_type_is_a (type, SP_TYPE_ITEM)) {
				use->child = (SPObject*)g_object_new (type, 0);
				sp_object_attach_reref (SP_OBJECT(use), use->child, NULL);
				sp_object_invoke_build (use->child, SP_OBJECT (use)->document, childrepr, TRUE);

				for (SPItemView *v = item->display; v != NULL; v = v->next) {
					NRArenaItem *ai;
					ai = sp_item_invoke_show (SP_ITEM (use->child), NR_ARENA_ITEM_ARENA (v->arenaitem), v->key, v->flags);
					if (ai) {
						nr_arena_item_add_child (v->arenaitem, ai, NULL);
						nr_arena_item_unref (ai);
					}
				}

			}
		}
	}

        // connect signal callback for setting the original_repr_changed flag, used by _compensation
         if (old_ref && use->repr) { 	 
                 sp_repr_remove_listener_by_data (use->repr, use); 	 
                 sp_repr_unref (use->repr); 	 
                 use->repr = NULL; 	 
         } 	 
         SPItem *refobj = use->ref->getObject(); 	 
         if (SP_IS_ITEM (refobj) && SP_OBJECT_REPR (refobj)) { 	 
                 use->repr = SP_OBJECT_REPR (refobj); 	 
                 sp_repr_ref (use->repr); 	 
                 sp_repr_add_listener (use->repr, &use_repr_events, use); 	 
                 sp_repr_synthesize_events (use->repr, &use_repr_events, use); 	 
         }
}

static void
sp_use_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPItem *item;
	SPUse *use;
	SPItemCtx *ictx, cctx;
	SPItemView *v;

	item = SP_ITEM (object);
	use = SP_USE (object);
	ictx = (SPItemCtx *) ctx;
	cctx = *ictx;

	// remember the original's bbox, if it's not yet set
	SPItem *refobj = use->ref->getObject();
 	if (SP_IS_ITEM (refobj) && use->original.isEmpty()) {
		NRRect bbox;
		NRMatrix i2doc;
		sp_item_i2doc_affine(SP_ITEM(refobj), &i2doc);
		sp_item_invoke_bbox(SP_ITEM(refobj), &bbox, &i2doc, TRUE);
		use->original = bbox;
 	}

	if (((SPObjectClass *) (parent_class))->update)
		((SPObjectClass *) (parent_class))->update (object, ctx, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	/* Set up child viewport */
	if (use->x.unit == SP_SVG_UNIT_PERCENT) {
		use->x.computed = use->x.value * (ictx->vp.x1 - ictx->vp.x0);
	}
	if (use->y.unit == SP_SVG_UNIT_PERCENT) {
		use->y.computed = use->y.value * (ictx->vp.y1 - ictx->vp.y0);
	}
	if (use->width.unit == SP_SVG_UNIT_PERCENT) {
		use->width.computed = use->width.value * (ictx->vp.x1 - ictx->vp.x0);
	}
	if (use->height.unit == SP_SVG_UNIT_PERCENT) {
		use->height.computed = use->height.value * (ictx->vp.y1 - ictx->vp.y0);
	}
	cctx.vp.x0 = 0.0;
	cctx.vp.y0 = 0.0;
	cctx.vp.x1 = use->width.computed;
	cctx.vp.y1 = use->height.computed;
	nr_matrix_set_identity (&cctx.i2vp);
	flags&=~SP_OBJECT_USER_MODIFIED_FLAG_B;

	if (use->child) {
		g_object_ref (G_OBJECT (use->child));
		if (flags || (use->child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			if (SP_IS_ITEM (use->child)) {
				SPItem *chi;
				chi = SP_ITEM (use->child);
				nr_matrix_multiply (&cctx.i2doc, &chi->transform, &ictx->i2doc);
				nr_matrix_multiply (&cctx.i2vp, &chi->transform, &ictx->i2vp);
				sp_object_invoke_update (use->child, (SPCtx *) &cctx, flags);
			} else {
				sp_object_invoke_update (use->child, ctx, flags);
			}
		}
		g_object_unref (G_OBJECT (use->child));
	}

	/* As last step set additional transform of arena group */
	for (v = item->display; v != NULL; v = v->next) {
		NRMatrix t;
		nr_matrix_set_translate (&t, use->x.computed, use->y.computed);
		nr_arena_group_set_child_transform (NR_ARENA_GROUP (v->arenaitem), &t);
	}

	// see if any move compensation is needed, and if so, do it
	sp_use_move_compensation (use);
}

static void
sp_use_modified (SPObject *object, guint flags)
{
	SPUse *use_obj;
	SPObject *child;

	use_obj = SP_USE (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	child = use_obj->child;
	if (child) {
		g_object_ref (G_OBJECT (child));
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			sp_object_invoke_modified (child, flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}

SPItem *
sp_use_unlink (SPUse *use)
{
	SPRepr *repr = SP_OBJECT_REPR(use);
	SPRepr *parent = sp_repr_parent (repr);
	gchar *id = g_strdup (sp_repr_attr (repr, "id"));
      gint pos = sp_repr_position (repr);
	SPDocument *document = SP_OBJECT(use)->document;

	//track the ultimate source of a chain of uses
	SPItem *orig = sp_use_root (use);

	//calculate the accummulated transform, starting from the original
	NR::Matrix t = sp_use_get_root_transform (use);
 
	// create copy of the original
	SPRepr *copy = sp_repr_duplicate (SP_OBJECT_REPR(orig));

	// remove the use
	sp_repr_unparent (SP_OBJECT_REPR (use));

	// add it to the document, preserving id, parent, and position
	sp_repr_append_child (parent, copy);
      sp_repr_set_position_absolute (copy, pos > 0 ? pos : 0);
	sp_repr_set_attr (copy, "id", id);

	// retrieve the SPItem of the resulting repr
	SPObject *unlinked = sp_document_lookup_id (document, sp_repr_attr (copy, "id"));
	SPItem *item = SP_ITEM(unlinked);

	// set the accummulated transform
	{
		NRMatrix ctrans = t.operator const NRMatrix&();
		sp_item_write_transform (item, SP_OBJECT_REPR (item), &ctrans);
	}

	return SP_ITEM(unlinked);
}

SPItem *
sp_use_get_original (SPUse *use)
{
	SPItem *ref = use->ref->getObject();
	return ref;
}
