#define __KNOT_HOLDER_C__

/*
 * Container for SPKnot visual handles
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2001-2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noKNOT_HOLDER_DEBUG


#include "document.h"
#include "sp-shape.h"
#include "knot.h"
#include "knotholder.h"
#include "knot-holder-entity.h"
#include "rect-context.h"
#include "sp-rect.h"
#include "arc-context.h"
#include "sp-ellipse.h"
#include "star-context.h"
#include "sp-star.h"
#include "spiral-context.h"
#include "sp-spiral.h"
#include "sp-offset.h"
#include "box3d.h"

#include <libnr/nr-matrix-div.h>
#include <glibmm/i18n.h>

class SPDesktop;

static void knot_clicked_handler (SPKnot *knot, guint state, gpointer data);
static void knot_moved_handler(SPKnot *knot, NR::Point const *p, guint state, gpointer data);
static void knot_ungrabbed_handler (SPKnot *knot, unsigned int state, SPKnotHolder *kh);
static void sp_knot_holder_class_init(SPKnotHolderClass *klass);

void sp_knot_holder_dispose(GObject *object);

#ifdef KNOT_HOLDER_DEBUG

static void sp_knot_holder_debug(GtkObject *object, gpointer data)
{
    g_print("sp-knot-holder-debug: [type=%s] [data=%s]\n", gtk_type_name(GTK_OBJECT_TYPE(object)), (const gchar *) data);
}
#endif

static GObjectClass *parent_class;

/**
 * Registers SPKnotHolder class and returns its type number.
 */
GType sp_knot_holder_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPKnotHolderClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_knot_holder_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof (SPKnotHolder),
            16,	/* n_preallocs */
            NULL,
            NULL
        };
        type = g_type_register_static (G_TYPE_OBJECT, "SPKnotHolder", &info, (GTypeFlags) 0);
    }
    return type;
}

/**
 * SPKnotHolder vtable initialization.
 */
static void sp_knot_holder_class_init(SPKnotHolderClass *klass){
    parent_class = (GObjectClass*) g_type_class_peek_parent(klass);
    klass->dispose = sp_knot_holder_dispose;
}

SPKnotHolder *sp_knot_holder_new(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler)
{
    Inkscape::XML::Node *repr = SP_OBJECT(item)->repr;

    g_return_val_if_fail(desktop != NULL, NULL);
    g_return_val_if_fail(item != NULL, NULL);
    g_return_val_if_fail(SP_IS_ITEM(item), NULL);

    SPKnotHolder *knot_holder = (SPKnotHolder*)g_object_new (SP_TYPE_KNOT_HOLDER, 0);
    knot_holder->desktop = desktop;
    knot_holder->item = item;
    g_object_ref(G_OBJECT(item));
    knot_holder->entity = NULL;

    knot_holder->released = relhandler;

    knot_holder->repr = repr;
    knot_holder->local_change = FALSE;

#ifdef KNOT_HOLDER_DEBUG
    g_signal_connect(G_OBJECT(desktop), "destroy", sp_knot_holder_debug, (gpointer) "SPKnotHolder::item");
#endif

    return knot_holder;
}

void sp_knot_holder_dispose(GObject *object) {
    SPKnotHolder *kh = G_TYPE_CHECK_INSTANCE_CAST((object), SP_TYPE_KNOT_HOLDER, SPKnotHolder);

    g_object_unref(G_OBJECT(kh->item));
    while (kh->entity) {
        SPKnotHolderEntity *e = (SPKnotHolderEntity *) kh->entity->data;
        g_signal_handler_disconnect(e->knot, e->_click_handler_id);
        g_signal_handler_disconnect(e->knot, e->_ungrab_handler_id);
        /* unref should call destroy */
        g_object_unref(e->knot);
        g_free(e);
        kh->entity = g_slist_remove(kh->entity, e);
    }
}

void sp_knot_holder_destroy(SPKnotHolder *kh) {
    g_object_unref(kh);
    }

void sp_knot_holder_add(
    SPKnotHolder *knot_holder,
    SPKnotHolderSetFunc knot_set,
    SPKnotHolderGetFunc knot_get,
    void (* knot_click) (SPItem *item, guint state),
    const gchar *tip
    )
{
    sp_knot_holder_add_full(knot_holder, knot_set, knot_get, knot_click, SP_KNOT_SHAPE_DIAMOND, SP_KNOT_MODE_XOR, tip);
}

void sp_knot_holder_add_full(
    SPKnotHolder *knot_holder,
    SPKnotHolderSetFunc knot_set,
    SPKnotHolderGetFunc knot_get,
    void (* knot_click) (SPItem *item, guint state),
    SPKnotShapeType     shape,
    SPKnotModeType      mode,
    const gchar *tip
    )
{
    g_return_if_fail(knot_holder != NULL);
    g_return_if_fail(knot_set != NULL);
    g_return_if_fail(knot_get != NULL);
	
    SPItem *item = SP_ITEM(knot_holder->item);

    /* create new SPKnotHolderEntry */
    SPKnotHolderEntity *e = g_new(SPKnotHolderEntity, 1);
    e->knot = sp_knot_new(knot_holder->desktop, tip);
    e->knot_set = knot_set;
    e->knot_get = knot_get;
    if (knot_click) {
        e->knot_click = knot_click;
    } else {
        e->knot_click = NULL;
    }

    g_object_set(G_OBJECT (e->knot->item), "shape", shape, NULL);
    g_object_set(G_OBJECT (e->knot->item), "mode", mode, NULL);

    // TODO: add a color argument
    //e->knot->fill [SP_KNOT_STATE_NORMAL] = 0x00ff0000;
    //g_object_set (G_OBJECT (e->knot->item), "fill_color", 0x00ff0000, NULL);

    knot_holder->entity = g_slist_append(knot_holder->entity, e);

    /* Move to current point. */
    NR::Point dp = e->knot_get(item) * sp_item_i2d_affine(item);
    sp_knot_set_position(e->knot, &dp, SP_KNOT_STATE_NORMAL);

    e->handler_id = g_signal_connect(e->knot, "moved", G_CALLBACK(knot_moved_handler), knot_holder);
    e->_click_handler_id = g_signal_connect(e->knot, "clicked", G_CALLBACK(knot_clicked_handler), knot_holder);
    e->_ungrab_handler_id = g_signal_connect(e->knot, "ungrabbed", G_CALLBACK(knot_ungrabbed_handler), knot_holder);

#ifdef KNOT_HOLDER_DEBUG
    g_signal_connect(ob, "destroy", sp_knot_holder_debug, "SPKnotHolder::knot");
#endif
    sp_knot_show(e->knot);
}

/**
 * \param p In desktop coordinates.
 */

static void knotholder_update_knots(SPKnotHolder *knot_holder, SPItem *item)
{
    NR::Matrix const i2d(sp_item_i2d_affine(item));

    for (GSList *el = knot_holder->entity; el; el = el->next) {
        SPKnotHolderEntity *e = (SPKnotHolderEntity *) el->data;
        GObject *kob = e->knot;

        NR::Point dp( e->knot_get(item) * i2d );
        g_signal_handler_block(kob, e->handler_id);
        sp_knot_set_position(e->knot, &dp, SP_KNOT_STATE_NORMAL);
        g_signal_handler_unblock(kob, e->handler_id);
    }
}

static void knot_clicked_handler(SPKnot *knot, guint state, gpointer data)
{
    SPKnotHolder *knot_holder = (SPKnotHolder *) data;
    SPItem *item  = SP_ITEM (knot_holder->item);

    g_object_ref(knot_holder);
    for (GSList *el = knot_holder->entity; el; el = el->next) {
        SPKnotHolderEntity *e = (SPKnotHolderEntity *) el->data;
        if (e->knot == knot) {
            if (e->knot_click) {
                e->knot_click(item, state);
            }
            break;
        }
    }

    if (SP_IS_SHAPE(item)) {
        sp_shape_set_shape(SP_SHAPE(item));
    }

    knotholder_update_knots(knot_holder, item);
    g_object_unref(knot_holder);

    unsigned int object_verb = SP_VERB_NONE;

    if (SP_IS_RECT(item))
        object_verb = SP_VERB_CONTEXT_RECT;
    else if (SP_IS_3DBOX(item))
        object_verb = SP_VERB_CONTEXT_3DBOX;
    else if (SP_IS_GENERICELLIPSE(item))
        object_verb = SP_VERB_CONTEXT_ARC;
    else if (SP_IS_STAR(item))
        object_verb = SP_VERB_CONTEXT_STAR;
    else if (SP_IS_SPIRAL(item))
        object_verb = SP_VERB_CONTEXT_SPIRAL;
    else if (SP_IS_OFFSET(item)) {
        if (SP_OFFSET(item)->sourceHref)
            object_verb = SP_VERB_SELECTION_LINKED_OFFSET;
        else
            object_verb = SP_VERB_SELECTION_DYNAMIC_OFFSET;
    }

    // for drag, this is done by ungrabbed_handler, but for click we must do it here
    sp_document_done(SP_OBJECT_DOCUMENT(knot_holder->item), object_verb, 
                     _("Change handle"));
}

static void knot_moved_handler(SPKnot *knot, NR::Point const *p, guint state, gpointer data)
{
    SPKnotHolder *knot_holder = (SPKnotHolder *) data;
    SPItem *item  = SP_ITEM (knot_holder->item);
    // this was a local change and the knotholder does not need to be recreated:
    knot_holder->local_change = TRUE;

    for (GSList *el = knot_holder->entity; el; el = el->next) {
        SPKnotHolderEntity *e = (SPKnotHolderEntity *) el->data;
        if (e->knot == knot) {
            NR::Point const q = *p / sp_item_i2d_affine(item);
            e->knot_set(item, q, e->knot->drag_origin / sp_item_i2d_affine(item), state);
            break;
        }
    }

    if (SP_IS_SHAPE (item)) {
        sp_shape_set_shape(SP_SHAPE (item));
    }

    knotholder_update_knots(knot_holder, item);
}

static void knot_ungrabbed_handler(SPKnot *knot, unsigned int state, SPKnotHolder *kh)
{
    if (kh->released) {
        kh->released(kh->item);
    } else {
        SPObject *object = (SPObject *) kh->item;
        object->updateRepr(object->repr, SP_OBJECT_WRITE_EXT);

        unsigned int object_verb = SP_VERB_NONE;

        if (SP_IS_RECT(object))
            object_verb = SP_VERB_CONTEXT_RECT;
        else if (SP_IS_3DBOX(object))
            object_verb = SP_VERB_CONTEXT_3DBOX;
        else if (SP_IS_GENERICELLIPSE(object))
            object_verb = SP_VERB_CONTEXT_ARC;
        else if (SP_IS_STAR(object))
            object_verb = SP_VERB_CONTEXT_STAR;
        else if (SP_IS_SPIRAL(object))
            object_verb = SP_VERB_CONTEXT_SPIRAL;
        else if (SP_IS_OFFSET(object)) {
            if (SP_OFFSET(object)->sourceHref)
                object_verb = SP_VERB_SELECTION_LINKED_OFFSET;
            else
                object_verb = SP_VERB_SELECTION_DYNAMIC_OFFSET;
        }
        
        sp_document_done(SP_OBJECT_DOCUMENT (object), object_verb,
                         _("Move handle"));
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
