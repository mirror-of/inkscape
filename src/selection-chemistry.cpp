#define __SP_SELECTION_CHEMISTRY_C__

/*
 * Miscellanous operations on selected items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <gtk/gtk.h>
#include "svg/svg.h"
#include "xml/repr-private.h"
#include "xml/repr-private.h"
#include "document.h"
#include "inkscape.h"
#include "desktop.h"
#include "selection.h"
#include "desktop-handles.h"
#include "sp-item-transform.h"
#include "sp-item-group.h"
#include "sp-path.h"
#include "sp-use.h"
#include "helper/sp-intl.h"
#include "display/sp-canvas.h"
#include "path-chemistry.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-rotate-fns.h"
#include "libnr/nr-scale-ops.h"
#include "style.h"
#include "document-private.h"
#include "sp-gradient.h"
#include "sp-pattern.h"
#include "sp-use-reference.h"
using NR::X;
using NR::Y;

#include "selection-chemistry.h"

/* fixme: find a better place */
GSList *clipboard = NULL;
GSList *gradient_clipboard = NULL;


void sp_selection_delete()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL) {
        return;
    }

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Nothing was deleted."));
        return;
    }

    GSList *selected = g_slist_copy(const_cast<GSList *>(selection->itemList()));
    GSList *iter;
    for ( iter = selected ; iter ; iter = iter->next ) {
        sp_object_ref((SPObject *)iter->data, NULL);
    }
    selection->clear();

    while (selected) {
        SPItem *item = (SPItem *)selected->data;
        SP_OBJECT(item)->deleteObject();
        sp_object_unref((SPObject *)item, NULL);
        selected = g_slist_remove(selected, selected->data);
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

/* fixme: sequencing */
void sp_selection_duplicate()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select some objects to duplicate."));
        return;
    }

    GSList *reprs = g_slist_copy((GSList *) selection->reprList());

    selection->clear();

    SPRepr *parent = ((SPRepr *) reprs->data)->parent;
    gboolean sort = TRUE;
    for (GSList *i = reprs->next; i; i = i->next) {
        if ((((SPRepr *) i->data)->parent) != parent) {
            // We can duplicate items from different parents, but we cannot do sorting in this case
            sort = FALSE;
        }
    }

    if (sort)
        reprs = g_slist_sort(reprs, (GCompareFunc) sp_repr_compare_position);

    GSList *newsel = NULL;

    while (reprs) {
        parent = ((SPRepr *) reprs->data)->parent;
        SPRepr *copy = sp_repr_duplicate((SPRepr *) reprs->data);

        sp_repr_append_child(parent, copy);

        newsel = g_slist_prepend(newsel, copy);
        reprs = g_slist_remove(reprs, reprs->data);
        sp_repr_unref(copy);
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));

    selection->setReprList(newsel);

    g_slist_free(newsel);
}

void sp_edit_clear_all()
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt)
        return;

    SPDocument *doc = SP_DT_DOCUMENT(dt);
    SP_DT_SELECTION(dt)->clear();

    GSList *items = sp_item_group_item_list(SP_GROUP(sp_document_root(doc)));

    while (items) {
        SP_OBJECT (items->data)->deleteObject();
        items = g_slist_remove(items, items->data);
    }

    sp_document_done(doc);
}

void sp_edit_select_all()
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt)
        return;

    SPDocument *doc = SP_DT_DOCUMENT(dt);
    SPSelection *selection = SP_DT_SELECTION(dt);

    GSList *items = sp_item_group_item_list(SP_GROUP(sp_document_root(doc)));
    while (items) {
        SPRepr *repr = SP_OBJECT_REPR(items->data);
        if (!selection->includesRepr(repr))
            selection->addRepr(repr);
        items = g_slist_remove(items, items->data);
    }
    sp_document_done(doc);
}

static void
sp_group_cleanup(SPGroup *group)
{
    GSList *l = NULL;
    for (SPObject *child = sp_object_first_child(SP_OBJECT(group)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
        sp_object_ref(child, NULL);
        l = g_slist_prepend(l, child);
    }

    while (l) {
        if (SP_IS_GROUP(l->data)) {
            sp_group_cleanup(SP_GROUP(l->data));
        } else if (SP_IS_PATH(l->data)) {
            sp_path_cleanup(SP_PATH(l->data));
        }
        sp_object_unref(SP_OBJECT(l->data), NULL);
        l = g_slist_remove(l, l->data);
    }


    if (!strcmp(sp_repr_name(SP_OBJECT_REPR(group)), "g")) {
        gint numitems;
        numitems = 0;
        for (SPObject *child = sp_object_first_child(SP_OBJECT(group)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_ITEM(child)) numitems += 1;
        }
        if (numitems <= 1) {
            sp_item_group_ungroup(group, NULL);
        }
    }
}

void sp_selection_cleanup()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (!doc)
        return;

    if (SP_ACTIVE_DESKTOP) {
        SP_DT_SELECTION(SP_ACTIVE_DESKTOP)->clear();
    }

    SPGroup *root = SP_GROUP(SP_DOCUMENT_ROOT(doc));
    sp_group_cleanup(root);

    sp_document_done(doc);
}

void sp_selection_group()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // Check if something is selected.
    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select two or more objects to group."));
        return;
    }

    GSList const *l = (GSList *) selection->reprList();

    // Check if at least two objects are selected.
    if (l->next == NULL) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select at least two objects to group."));
        return;
    }

    // Check if all selected objects have common parent.
    GSList *reprs = g_slist_copy((GSList *) selection->reprList());
    SPRepr *parent = ((SPRepr *) reprs->data)->parent;
    for (GSList *i = reprs->next; i; i = i->next) {
        if ((((SPRepr *) i->data)->parent) != parent) {
            sp_view_set_statusf_error(SP_VIEW(desktop), _("You cannot group objects from different groups or layers."));
            return;
        }
    }

    GSList *p = g_slist_copy((GSList *) l);

    selection->clear();

    p = g_slist_sort(p, (GCompareFunc) sp_repr_compare_position);

    // Remember the position of the topmost object.
    gint topmost = sp_repr_position((SPRepr *) g_slist_last(p)->data);

    SPRepr *group = sp_repr_new("g");

    while (p) {
        SPRepr *spnew;
        SPRepr *current = (SPRepr *) p->data;
        spnew = sp_repr_duplicate(current);
        sp_repr_unparent(current);
        sp_repr_append_child(group, spnew);
        sp_repr_unref(spnew);
        topmost --;
        p = g_slist_remove(p, current);
    }

    // Add the new group to the group members' common parent.
    sp_repr_append_child(parent, group);

    // Move to the position of the topmost, reduced by the number of deleted items.
    sp_repr_set_position_absolute(group, topmost > 0 ? topmost + 1 : 0);

    sp_document_done(SP_DT_DOCUMENT(desktop));

    selection->setRepr(group);
    sp_repr_unref(group);
}

void sp_selection_ungroup()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select a group to ungroup."));
        return;
    }

    // Get a copy of current selection.
    GSList *new_select = NULL;
    bool ungrouped = false;
    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next)
    {
        SPItem *group = (SPItem *) items->data;

        /* We do not allow ungrouping <svg> etc. (lauris) */
        if (strcmp(sp_repr_name(SP_OBJECT_REPR(group)), "g")) {
            // keep the non-group item in the new selection
            new_select = g_slist_prepend(new_select, group);
            continue;
        }

        GSList *children = NULL;
        /* This is not strictly required, but is nicer to rely on group ::destroy (lauris) */
        sp_item_group_ungroup(SP_GROUP(group), &children, false);
        ungrouped = true;
        // Add ungrouped items to the new selection.
        new_select = g_slist_concat(new_select, children);
    }

    if (new_select) { // Set new selection.
        selection->clear();
        selection->setItemList(new_select);
        g_slist_free(new_select);
    }
    if (!ungrouped) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("No groups to ungroup in the selection."));
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

static SPGroup *
sp_item_list_common_parent_group(const GSList *items)
{
    if (!items) {
        return NULL;
    }
    SPObject *parent = SP_OBJECT_PARENT(items->data);
    /* Strictly speaking this CAN happen, if user selects <svg> from XML editor */
    if (!SP_IS_GROUP(parent)) {
        return NULL;
    }
    for (items = items->next; items; items = items->next) {
        if (SP_OBJECT_PARENT(items->data) != parent) {
            return NULL;
        }
    }

    return SP_GROUP(parent);
}

/** Finds out the minimum common bbox of the selected items
 */
NR::Rect
enclose_items(const GSList *items)
{
    g_assert(items != NULL);

    NR::Rect r = sp_item_bbox_desktop((SPItem *) items->data);

    for (GSList *i = items->next; i; i = i->next) {
        r = NR::Rect::union_bounds(r, sp_item_bbox_desktop((SPItem *) i->data));
    }

    return r;
}

SPObject *
prev_sibling(SPObject *child)
{
    SPObject *parent = SP_OBJECT_PARENT(child);
    if (!SP_IS_GROUP(parent)) {
        return NULL;
    }
    for ( SPObject *i = sp_object_first_child(parent) ; i; i = SP_OBJECT_NEXT(i) ) {
        if (i->next == child)
            return i;
    }
    return NULL;
}

void
sp_selection_raise()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    GSList const *items = (GSList *) selection->itemList();
    if (!items) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select some objects to raise."));
        return;
    }

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("You cannot raise/lower objects from different groups or layers."));
        return;
    }

    SPRepr *grepr = SP_OBJECT_REPR(group);

    /* construct reverse-ordered list of selected children */
    GSList *rev = g_slist_copy((GSList *) items);
    rev = g_slist_sort(rev, (GCompareFunc) sp_item_repr_compare_position);

    // find out the common bbox of the selected items
    NR::Rect selected = enclose_items(items);

    // for all objects in the selection (starting from top)
    while (rev) {
        SPObject *child = SP_OBJECT(rev->data);
        // for each selected object, find the next sibling
        for (SPObject *newref = child->next; newref; newref = newref->next) {
            // if the sibling is an item AND overlaps our selection,
            if (SP_IS_ITEM(newref) && selected.intersects(sp_item_bbox_desktop(SP_ITEM(newref)))) {
                // AND if it's not one of our selected objects,
                if (!g_slist_find((GSList *) items, newref)) {
                    // move the selected object after that sibling
                    sp_repr_change_order(grepr, SP_OBJECT_REPR(child), SP_OBJECT_REPR(newref));
                }
                break;
            }
        }
        rev = g_slist_remove(rev, child);
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

void sp_selection_raise_to_top()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPDocument *document = SP_DT_DOCUMENT(desktop);
    SPSelection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select some objects to raise to top."));
        return;
    }

    GSList *rl = g_slist_copy((GSList *) selection->reprList());
    rl = g_slist_sort(rl, (GCompareFunc) sp_repr_compare_position);

    for (GSList *l = rl; l != NULL; l = l->next) {
        SPRepr *repr = (SPRepr *) l->data;
        sp_repr_set_position_absolute(repr, -1);
    }

    g_slist_free(rl);

    sp_document_done(document);
}

void
sp_selection_lower()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    GSList const *items = (GSList *) selection->itemList();
    if (!items) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select some objects to lower."));
        return;
    }

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("You cannot raise/lower objects from different groups or layers."));
        return;
    }

    SPRepr *grepr = SP_OBJECT_REPR(group);

    // find out the common bbox of the selected items
    NR::Rect selected = enclose_items(items);

    /* construct direct-ordered list of selected children */
    GSList *rev = g_slist_copy((GSList *) items);
    rev = g_slist_sort(rev, (GCompareFunc) sp_item_repr_compare_position);
    rev = g_slist_reverse(rev);

    // for all objects in the selection (starting from top)
    while (rev) {
        SPObject *child = SP_OBJECT(rev->data);
        // for each selected object, find the prev sibling
        for (SPObject *newref = prev_sibling(child); newref; newref = prev_sibling(newref)) {
            // if the sibling is an item AND overlaps our selection,
            if (SP_IS_ITEM(newref) && selected.intersects(sp_item_bbox_desktop(SP_ITEM(newref)))) {
                // AND if it's not one of our selected objects,
                if (!g_slist_find((GSList *) items, newref)) {
                    // move the selected object before that sibling
                    SPObject *put_after = prev_sibling(newref);
                    if (put_after)
                        sp_repr_change_order(grepr, SP_OBJECT_REPR(child), SP_OBJECT_REPR(put_after));
                    else
                        sp_repr_set_position_absolute(SP_OBJECT_REPR(child), 0);
                }
                break;
            }
        }
        rev = g_slist_remove(rev, child);
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));

}

void sp_selection_lower_to_bottom()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPDocument *document = SP_DT_DOCUMENT(desktop);
    SPSelection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select some objects to lower to bottom."));
        return;
    }

    GSList *rl;
    rl = g_slist_copy((GSList *) selection->reprList());
    rl = g_slist_sort(rl, (GCompareFunc) sp_repr_compare_position);
    rl = g_slist_reverse(rl);

    for (GSList *l = rl; l != NULL; l = l->next) {
        gint minpos;
        SPObject *pp, *pc;
        SPRepr *repr = (SPRepr *) l->data;
        pp = document->getObjectByRepr(sp_repr_parent(repr));
        minpos = 0;
        g_assert(SP_IS_GROUP(pp));
        pc = sp_object_first_child(pp);
        while (!SP_IS_ITEM(pc)) {
            minpos += 1;
            pc = pc->next;
        }
        sp_repr_set_position_absolute(repr, minpos);
    }

    g_slist_free(rl);

    sp_document_done(document);
}

void
sp_undo(SPDesktop *desktop, SPDocument *doc)
{
    if (SP_IS_DESKTOP(desktop)) {
        if (!sp_document_undo(SP_DT_DOCUMENT(desktop)))
            sp_view_set_statusf_flash(SP_VIEW(desktop), _("Nothing to undo."));
    }
}

void
sp_redo(SPDesktop *desktop, SPDocument *doc)
{
    if (SP_IS_DESKTOP(desktop)) {
        if (!sp_document_redo(SP_DT_DOCUMENT(desktop)))
            sp_view_set_statusf_flash(SP_VIEW(desktop), _("Nothing to redo."));
    }
}

void sp_selection_cut()
{
    sp_selection_copy();
    sp_selection_delete();
}

void sp_copy_gradient (SPGradient *gradient)
{
    SPGradient *ref = gradient;
    SPRepr *grad_repr;

    while ( !SP_GRADIENT_HAS_STOPS(gradient) && ref ) {

        grad_repr =sp_repr_duplicate (SP_OBJECT_REPR(gradient));
        gradient_clipboard = g_slist_prepend(gradient_clipboard, grad_repr);

        gradient = ref;
        ref = gradient->ref->getObject();
    }

    grad_repr = sp_repr_duplicate(SP_OBJECT_REPR(gradient));
    gradient_clipboard = g_slist_prepend(gradient_clipboard, grad_repr);
}

void sp_copy_pattern (SPPattern *pattern)
{
    SPRepr *pattern_repr;

    pattern_repr = sp_repr_duplicate(SP_OBJECT_REPR(pattern));
    gradient_clipboard = g_slist_prepend(gradient_clipboard, pattern_repr);
}


void sp_copy_stuff_used_by_item (SPItem *item)
{
    SPRepr *repr = SP_OBJECT_REPR(item);
    SPStyle *style = sp_style_new();
    sp_style_read_from_repr (style, repr);

    if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) { 
        SPObject *server = SP_OBJECT_STYLE_FILL_SERVER(item);
        if (SP_IS_LINEARGRADIENT (server) || SP_IS_RADIALGRADIENT (server))
            sp_copy_gradient (SP_GRADIENT(server));
        if (SP_IS_PATTERN (server))
            sp_copy_pattern (SP_PATTERN(server));
    }

    if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) { 
        SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER(item);
        if (SP_IS_LINEARGRADIENT (server) || SP_IS_RADIALGRADIENT (server))
            sp_copy_gradient (SP_GRADIENT(server));
        if (SP_IS_PATTERN (server))
            sp_copy_pattern (SP_PATTERN(server));
    }

    // recurse
    for (SPObject *o = SP_OBJECT(item)->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o))
            sp_copy_stuff_used_by_item (SP_ITEM (o));
    }
}

void sp_selection_copy()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Nothing was copied."));
        return;
    }

    const GSList *items = (GSList *) selection->itemList();

    /* Clear old gradient clipboard */
    while (gradient_clipboard) {
        sp_repr_unref((SPRepr *) gradient_clipboard->data);
        gradient_clipboard = g_slist_remove (gradient_clipboard, gradient_clipboard->data);
    }

    // copy stuff referenced by the item to gradient_clipboard
    for (; items != NULL; items = items->next) {
        sp_copy_stuff_used_by_item (SP_ITEM (items->data));
    }

    GSList *reprs = g_slist_copy ((GSList *) selection->reprList());

     SPRepr *parent = ((SPRepr *) reprs->data)->parent;
     gboolean sort = TRUE;
     for (GSList *i = reprs->next; i; i = i->next) {
         if ((((SPRepr *) i->data)->parent) != parent) {
             // We can copy items from different parents, but we cannot do sorting in this case
             sort = FALSE;
         }
     }

     if (sort)
        reprs = g_slist_sort(reprs, (GCompareFunc) sp_repr_compare_position);

    /* Clear old clipboard */
    while (clipboard) {
        sp_repr_unref((SPRepr *) clipboard->data);
        clipboard = g_slist_remove(clipboard, clipboard->data);
    }

    while (reprs != NULL) {
        SPRepr *repr = (SPRepr *) reprs->data;
        reprs = g_slist_remove (reprs, repr);
        SPCSSAttr *css = sp_repr_css_attr_inherited(repr, "style");
        SPRepr *copy = sp_repr_duplicate(repr);
        sp_repr_css_set(copy, css, "style");
        sp_repr_css_attr_unref(css);

        clipboard = g_slist_prepend(clipboard, copy);
    }

    clipboard = g_slist_reverse(clipboard);
    gradient_clipboard = g_slist_reverse(gradient_clipboard);
}

void sp_selection_paste(bool in_place)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL) return;
    g_assert(SP_IS_DESKTOP(desktop));

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is in the clipboard
    if (clipboard == NULL) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Nothing in the clipboard."));
        return;
    }

    selection->clear();
    SPDocument *doc = SP_DT_DOCUMENT(desktop);
    SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(doc);
    // add gradients referenced by copied objects to defs
    for (GSList *gl = gradient_clipboard; gl != NULL; gl = gl->next) {
        SPRepr *repr = (SPRepr *) gl->data;
        SPObject *exists = doc->getObjectByRepr(repr);
        if (!exists){
            SPRepr *copy = sp_repr_duplicate(repr);
            sp_repr_add_child(SP_OBJECT_REPR(defs), copy, NULL);
            sp_repr_unref(copy);
        }
    }

    GSList *copied = NULL;
    // add objects to document
    for (GSList *l = clipboard; l != NULL; l = l->next) {
        SPRepr *repr = (SPRepr *) l->data;
        SPRepr *copy = sp_repr_duplicate(repr);
        desktop->currentLayer()->appendChildRepr(copy);
        copied = g_slist_append(copied, copy);
        sp_repr_unref(copy);
    }

    // add new documents to selection (would have done this above but screws with fill/stroke dialog)
    for (GSList *l = copied; l != NULL; l = l->next) {
        SPRepr *repr = (SPRepr *) l->data;
        selection->addRepr(repr);
    }

    if (!in_place) {
        sp_document_ensure_up_to_date(SP_DT_DOCUMENT(desktop));

        NR::Point m( sp_desktop_point(desktop) - selection->bounds().midpoint() );

        /* Snap the offset of the new item(s) to the grid */
        /* FIXME: this gridsnap fiddling is a hack. */
        gdouble const curr_gridsnap = desktop->gridsnap;
        desktop->gridsnap = 1e18;
        sp_desktop_free_snap(desktop, m);
        desktop->gridsnap = curr_gridsnap;
        sp_selection_move_relative(selection, m[NR::X], m[NR::Y]);
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

void sp_selection_paste_style()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL) return;
    g_assert(SP_IS_DESKTOP(desktop));

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is in the clipboard
    if (clipboard == NULL) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Nothing in the clipboard."));
        return;
    }

    // check if something is selected
    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select objects to paste style to."));
        return;
    }

    GSList *selected = g_slist_copy((GSList *) selection->itemList());
    SPDocument *doc = SP_DT_DOCUMENT(desktop);

    for (GSList *l = selected; l != NULL; l = l->next) {

        // take the style from first object on clipboard
        SPStyle *style = sp_style_new();
        sp_style_read_from_repr(style, (SPRepr *) clipboard->data);
        // if its using a gradient we need the def too
        if (style && style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {    /* Object has pattern or gradient fill*/
            SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(SP_DT_DOCUMENT(desktop));
            // add gradients referenced by clipboard objects to defs
            for (GSList *gl = gradient_clipboard; gl != NULL; gl = gl->next) {
                SPRepr *repr = (SPRepr *) gl->data;
                SPObject *exists = doc->getObjectByRepr(repr);
                if (!exists){
                    SPRepr *copy = sp_repr_duplicate(repr);
                    sp_repr_add_child(SP_OBJECT_REPR(defs), copy, NULL);
                    sp_repr_unref(copy);
                }
            }
        }

        // merge it with the current object's style, if any
        const gchar *old_style = sp_repr_attr(SP_OBJECT_REPR(l->data), "style");
        if (old_style)
            sp_style_merge_from_style_string(style, old_style);

        // calculate the difference between the current object and its parent styles
        gchar *newcss = sp_style_write_difference(style, SP_OBJECT_STYLE(SP_OBJECT_PARENT(l->data)));

        // write the result to the object repr
        sp_repr_set_attr(SP_OBJECT_REPR(l->data), "style", (newcss && *newcss) ? newcss : NULL);
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

void sp_selection_apply_affine(SPSelection *selection, NR::Matrix const &affine)
{
    if (selection->isEmpty())
        return;

    for (GSList const *l = selection->itemList(); l != NULL; l = l-> next) {
        SPItem *item = SP_ITEM(l->data);

        // see comment in seltrans.cpp/sp_sel_trans_ungrab
        if (affine.is_translation() && SP_IS_USE(item) && selection->includesItem(SP_USE(item)->ref->getObject())) {
            ; //do nothing
        } else {
            sp_item_set_i2d_affine(item, sp_item_i2d_affine(item) * affine);
            /* update repr -  needed for undo */
            sp_item_write_transform(item, SP_OBJECT_REPR(item), &item->transform);
        }
    }
}

void sp_selection_remove_transform()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    GSList const *l = (GSList *) selection->reprList();
    while (l != NULL) {
        sp_repr_set_attr((SPRepr*)l->data,"transform", NULL);
        l = l->next;
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

void
sp_selection_scale_absolute(SPSelection *selection,
                            double const x0, double const x1,
                            double const y0, double const y1)
{
    if (selection->isEmpty())
        return;

    NR::Rect const bbox(selection->bounds());

    NR::translate const p2o(-bbox.min());

    NR::scale const newSize(x1 - x0,
                            y1 - y0);
    NR::scale const scale( newSize / NR::scale(bbox.dimensions()) );
    NR::translate const o2n(x0, y0);
    NR::Matrix const final( p2o * scale * o2n );

    sp_selection_apply_affine(selection, final);
}


void sp_selection_scale_relative(SPSelection *selection, NR::Point const &align, NR::scale const &scale)
{
    if (selection->isEmpty())
        return;

    // don't try to scale above 1 Mpt, it won't display properly and will crash sooner or later anyway
    NR::Rect const bbox(selection->bounds());
    if ( bbox.extent(NR::X) * scale[NR::X] > 1e6  ||
         bbox.extent(NR::Y) * scale[NR::Y] > 1e6 )
    {
        return;
    }

    NR::translate const n2d(-align);
    NR::translate const d2n(align);
    NR::Matrix const final( n2d * scale * d2n );
    sp_selection_apply_affine(selection, final);
}

void
sp_selection_rotate_relative(SPSelection *selection, NR::Point const &center, gdouble const angle_degrees)
{
    NR::translate const d2n(center);
    NR::translate const n2d(-center);
    NR::rotate const rotate(rotate_degrees(angle_degrees));
    NR::Matrix const final( NR::Matrix(n2d) * rotate * d2n );
    sp_selection_apply_affine(selection, final);
}

void
sp_selection_skew_relative(SPSelection *selection, NR::Point const &align, double dx, double dy)
{
    NR::translate const d2n(align);
    NR::translate const n2d(-align);
    NR::Matrix const skew(1, dy,
                          dx, 1,
                          0, 0);
    NR::Matrix const final( n2d * skew * d2n );
    sp_selection_apply_affine(selection, final);
}

void sp_selection_move_relative(SPSelection *selection, double dx, double dy)
{
    sp_selection_apply_affine(selection, NR::Matrix(NR::translate(dx, dy)));
}


/**
 * \brief sp_selection_rotate_90
 *
 * This function rotates selected objects 90 degrees clockwise.
 *
 */

void sp_selection_rotate_90_cw()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!SP_IS_DESKTOP(desktop))
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty())
        return;

    GSList const *l = selection->itemList();
    NR::rotate const rot_neg_90(NR::Point(0, -1));
    for (GSList const *l2 = l ; l2 != NULL ; l2 = l2->next) {
        SPItem *item = SP_ITEM(l2->data);
        sp_item_rotate_rel(item, rot_neg_90);
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}


/**
 * \brief sp_selection_rotate_90_ccw
 *
 * This function rotates selected objects 90 degrees counter-clockwise.
 *
 */

void sp_selection_rotate_90_ccw()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!SP_IS_DESKTOP(desktop))
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty())
        return;

    GSList const *l = selection->itemList();
    NR::rotate const rot_neg_90(NR::Point(0, 1));
    for (GSList const *l2 = l ; l2 != NULL ; l2 = l2->next) {
        SPItem *item = SP_ITEM(l2->data);
        sp_item_rotate_rel(item, rot_neg_90);
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

void
sp_selection_rotate(SPSelection *selection, gdouble const angle_degrees)
{
    if (selection->isEmpty())
        return;

    NR::Point const center(selection->bounds().midpoint());

    sp_selection_rotate_relative(selection, center, angle_degrees);

    sp_document_maybe_done(SP_DT_DOCUMENT(selection->desktop()),
                           ( ( angle_degrees > 0 )
                             ? "selector:rotate:ccw"
                             : "selector:rotate:cw" ));
}

/**
\param  angle   the angle in "angular pixels", i.e. how many visible pixels must move the outermost point of the rotated object
*/
void
sp_selection_rotate_screen(SPSelection *selection, gdouble angle)
{
    if (selection->isEmpty())
        return;

    NR::Rect const bbox(selection->bounds());
    NR::Point const center(bbox.midpoint());

    gdouble const zoom = SP_DESKTOP_ZOOM(selection->desktop());
    gdouble const zmove = angle / zoom;
    gdouble const r = NR::L2(bbox.max() - center);

    gdouble const zangle = 180 * atan2(zmove, r) / M_PI;

    sp_selection_rotate_relative(selection, center, zangle);

    sp_document_maybe_done(SP_DT_DOCUMENT(selection->desktop()),
                           ( (angle > 0)
                             ? "selector:rotate:ccw"
                             : "selector:rotate:cw" ));
}

void
sp_selection_scale(SPSelection *selection, gdouble grow)
{
    if (selection->isEmpty())
        return;

    NR::Rect const bbox(selection->bounds());
    NR::Point const center(bbox.midpoint());
    double const max_len = bbox.maxExtent();

    // you can't scale "do nizhe pola" (below zero)
    if ( max_len + 2 * grow <= 0 ) {
        return;
    }

    double const times = 1.0 + grow / max_len;
    sp_selection_scale_relative(selection, center, NR::scale(times, times));

    sp_document_maybe_done(SP_DT_DOCUMENT(selection->desktop()),
                           ( (grow > 0)
                             ? "selector:scale:larger"
                             : "selector:scale:smaller" ));
}

void
sp_selection_scale_screen(SPSelection *selection, gdouble grow_pixels)
{
    sp_selection_scale(selection,
                       grow_pixels / SP_DESKTOP_ZOOM(selection->desktop()));
}

void
sp_selection_scale_times(SPSelection *selection, gdouble times)
{
    if (selection->isEmpty())
        return;

    NR::Point const center(selection->bounds().midpoint());
    sp_selection_scale_relative(selection, center, NR::scale(times, times));
    sp_document_done(SP_DT_DOCUMENT(selection->desktop()));
}

void
sp_selection_move(gdouble dx, gdouble dy)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    g_return_if_fail(SP_IS_DESKTOP(desktop));
    SPSelection *selection = SP_DT_SELECTION(desktop);
    if (selection->isEmpty()) {
        return;
    }

    sp_selection_move_relative(selection, dx, dy);

    if (dx == 0) {
        sp_document_maybe_done(SP_DT_DOCUMENT(desktop), "selector:move:vertical");
    } else if (dy == 0) {
        sp_document_maybe_done(SP_DT_DOCUMENT(desktop), "selector:move:horizontal");
    } else {
        sp_document_done(SP_DT_DOCUMENT(desktop));
    }
}

void
sp_selection_move_screen(gdouble dx, gdouble dy)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    g_return_if_fail(SP_IS_DESKTOP(desktop));

    SPSelection *selection = SP_DT_SELECTION(desktop);
    if (selection->isEmpty()) {
        return;
    }

    // same as sp_selection_move but divide deltas by zoom factor
    gdouble const zoom = SP_DESKTOP_ZOOM(desktop);
    gdouble const zdx = dx / zoom;
    gdouble const zdy = dy / zoom;
    sp_selection_move_relative(selection, zdx, zdy);

    if (dx == 0) {
        sp_document_maybe_done(SP_DT_DOCUMENT(desktop), "selector:move:vertical");
    } else if (dy == 0) {
        sp_document_maybe_done(SP_DT_DOCUMENT(desktop), "selector:move:horizontal");
    } else {
        sp_document_done(SP_DT_DOCUMENT(desktop));
    }
}

void
sp_selection_item_next(void)
{
    SPDocument *document = SP_ACTIVE_DOCUMENT;
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    g_return_if_fail(document != NULL);
    g_return_if_fail(desktop != NULL);
    if (!SP_IS_DESKTOP(desktop)) {
        return;
    }
    SPSelection *selection = SP_DT_SELECTION(desktop);

    // Get item list.
    GSList *children = NULL;
    if (SP_CYCLING == SP_CYCLE_VISIBLE) {
        NRRect dbox;
        sp_desktop_get_display_area(desktop, &dbox);
        children = sp_document_items_in_box(document, &dbox);
    } else {
        children = sp_item_group_item_list(SP_GROUP(sp_document_root(document)));
    }

    // Compute next item.
    if (children == NULL) {
        return;
    }
    SPItem *item = NULL;
    if (selection->isEmpty()) {
        item = SP_ITEM(children->data);
    } else {
        GSList *l = g_slist_find(children, selection->itemList()->data);
        if ( ( l == NULL ) || ( l->next == NULL ) ) {
            item = SP_ITEM(children->data);
        } else {
            item = SP_ITEM(l->next->data);
        }
    }

    // Set selection to item.
    if (item != NULL) {
        selection->setItem(item);
    } else {
        return;
    }

    g_slist_free(children);

    // Adjust visible area to see whole new selection.
    if (SP_CYCLING == SP_CYCLE_FOCUS) {
        scroll_to_show_item(desktop, item);
    }
}

/* TODO: Much copy & paste code here; see if can merge with sp_selection_item_next. */
void
sp_selection_item_prev(void)
{
    SPDocument *document = SP_ACTIVE_DOCUMENT;
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    g_return_if_fail(document != NULL);
    g_return_if_fail(desktop != NULL);
    if (!SP_IS_DESKTOP(desktop)) {
        return;
    }
    SPSelection *selection = SP_DT_SELECTION(desktop);

    // Get item list.
    GSList *children = NULL;
    if (SP_CYCLING == SP_CYCLE_VISIBLE) {
        NRRect dbox;
        sp_desktop_get_display_area(desktop, &dbox);
        children = sp_document_items_in_box(document, &dbox);
    } else {
        children = sp_item_group_item_list(SP_GROUP(sp_document_root(document)));
    }

    // Compute prev item.
    if (children == NULL) {
        return;
    }
    SPItem *item = NULL;
    if (selection->isEmpty()) {
        item = SP_ITEM(g_slist_last(children)->data);
    } else {
        GSList *l = children;
        while ((l->next != NULL) && (l->next->data != selection->itemList()->data)) {
            l = l->next;
        }
        item = SP_ITEM(l->data);
    }

    // Set selection to item.
    if (item != NULL) {
        selection->setItem(item);
    } else {
        return;
    }

    g_slist_free(children);

    // Adjust visible area to see whole new selection.
    if (SP_CYCLING == SP_CYCLE_FOCUS) {
        scroll_to_show_item(desktop, item);
    }
}

/**
 * If \a item is not entirely visible then adjust visible area to centre on the centre on of
 * \a item.
 */
void scroll_to_show_item(SPDesktop *desktop, SPItem *item)
{
    NRRect dbox;
    sp_desktop_get_display_area(desktop, &dbox);
    NRRect sbox;
    sp_item_bbox_desktop(item, &sbox);
    if ( dbox.x0 > sbox.x0  ||
         dbox.y0 > sbox.y0  ||
         dbox.x1 < sbox.x1  ||
         dbox.y1 < sbox.y1 )
    {
        NR::Point const s_dt(( sbox.x0 + sbox.x1 ) / 2,
                             ( sbox.y0 + sbox.y1 ) / 2);
        NR::Point const s_w( s_dt * desktop->d2w );
        NR::Point const d_dt(( dbox.x0 + dbox.x1 ) / 2,
                             ( dbox.y0 + dbox.y1 ) / 2);
        NR::Point const d_w( d_dt * desktop->d2w );
        NR::Point const moved_w( d_w - s_w );
        gint const dx = (gint) moved_w[X];
        gint const dy = (gint) moved_w[Y];
        sp_desktop_scroll_world(desktop, dx, dy);
    }
}


void
sp_selection_clone()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select an object to clone."));
        return;
    }

    // Check if more than one object is selected.
    if (g_slist_length((GSList *) selection->itemList()) > 1) {
        sp_view_set_statusf_error(SP_VIEW(desktop), _("If you want to clone several objects, group them and clone the group."));
        return;
    }

    SPRepr *sel_repr = SP_OBJECT_REPR(selection->singleItem());
    SPRepr *parent = sp_repr_parent(sel_repr);

    SPRepr *clone = sp_repr_new("use");
    sp_repr_set_attr(clone, "x", "0");
    sp_repr_set_attr(clone, "y", "0");
    sp_repr_set_attr(clone, "xlink:href", g_strdup_printf("#%s", sp_repr_attr(sel_repr, "id")));

    // add the new clone to the top of the original's parent
    sp_repr_append_child(parent, clone);

    sp_document_done(SP_DT_DOCUMENT(desktop));

    selection->setRepr(clone);
    sp_repr_unref(clone);
}

void
sp_selection_unlink()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select a clone to unlink."));
        return;
    }

    // Get a copy of current selection.
    GSList *new_select = NULL;
    bool unlinked = false;
    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next)
    {
        SPItem *use = (SPItem *) items->data;

        if (!SP_IS_USE(use)) {
            // keep the non-yse item in the new selection
            new_select = g_slist_prepend(new_select, use);
            continue;
        }

        SPItem *unlink = sp_use_unlink(SP_USE(use));
        unlinked = true;
        // Add ungrouped items to the new selection.
        new_select = g_slist_prepend(new_select, unlink);
    }

    if (new_select) { // set new selection
        selection->clear();
        selection->setItemList(new_select);
        g_slist_free(new_select);
    }
    if (!unlinked) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("No clones to unlink in the selection."));
    }

    sp_document_done(SP_DT_DOCUMENT(desktop));
}

void
sp_select_clone_original()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // Check if more than two objects are selected, or not an SPUse is selected.
    if (g_slist_length((GSList *) selection->itemList()) != 1 || !SP_IS_USE(selection->singleItem())) {
        sp_view_set_statusf_error(SP_VIEW(desktop), _("Select a single clone to go to its original."));
        return;
    }

    SPItem *original = sp_use_get_original(SP_USE(selection->singleItem()));

    if (original) {
        selection->clear();
        selection->setItem(original);
        if (SP_CYCLING == SP_CYCLE_FOCUS) {
            scroll_to_show_item(desktop, original);
        }
    }
}

void
sp_selection_tile()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    SPDocument *document = SP_DT_DOCUMENT(desktop);

    SPSelection *selection = SP_DT_SELECTION(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("Select an object to tile."));
        return;
    }

    NR::Rect r = selection->bounds();

    sp_document_ensure_up_to_date(document);
    NR::Point m = (NR::Point(0, sp_document_height(document)) - (r.min() + NR::Point (0, r.extent(NR::Y))));
    sp_selection_move_relative(selection, m[NR::X], m[NR::Y]);

    GSList *reprs = g_slist_copy((GSList *) selection->reprList());

    SPRepr *parent = ((SPRepr *) reprs->data)->parent;
    gboolean sort = TRUE;
    for (GSList *i = reprs->next; i; i = i->next) {
        if ((((SPRepr *) i->data)->parent) != parent) {
            // We can tile items from different parents, but we cannot do sorting in this case
            sort = FALSE;
        }
    }

    if (sort)
        reprs = g_slist_sort(reprs, (GCompareFunc) sp_repr_compare_position);

    reprs = g_slist_reverse (reprs);

    SPRepr *rect = pattern_tile (reprs, 
                                 NR::Rect (sp_desktop_d2doc_xy_point(desktop, r.min()), sp_desktop_d2doc_xy_point(desktop, r.max())),
                                 document, NR::Matrix(NR::translate(sp_desktop_d2doc_xy_point (desktop, r.min()))));

    SPItem *rectangle = SP_ITEM(desktop->currentLayer()->appendChildRepr(rect));
    sp_repr_unref (rect);

    selection->clear();

    for (GSList *i = reprs; i != NULL; i = i->next) {
        SPObject *item = document->getObjectByRepr(((SPRepr *) i->data));
        item->deleteObject();
    }

    selection->setItem (rectangle);

    sp_document_done (document);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
