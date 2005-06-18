#define __SP_TEXT_CHEMISTRY_C__

/*
 * Text commands
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2004 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include "libnr/nr-macros.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include "sp-object.h"
#include "sp-path.h"
#include "sp-rect.h"
#include "sp-text.h"
#include "sp-tspan.h"
#include "style.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "text-editing.h"
#include "sp-flowtext.h"
#include "sp-flowregion.h"
#include "sp-flowdiv.h"


SPItem *
text_in_selection(Inkscape::Selection *selection)
{
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_TEXT(items->data))
            return ((SPItem *) items->data);
    }
    return NULL;
}

SPItem *
flowtext_in_selection(Inkscape::Selection *selection)
{
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_FLOWTEXT(items->data))
            return ((SPItem *) items->data);
    }
    return NULL;
}

SPItem *
shape_in_selection(Inkscape::Selection *selection)
{
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_SHAPE(items->data))
            return ((SPItem *) items->data);
    }
    return NULL;
}

void
text_put_on_path()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return;

    Inkscape::Selection *selection = SP_DT_SELECTION(desktop);

    SPItem *text = text_in_selection(selection);
    SPItem *shape = shape_in_selection(selection);

    if (!text || !shape || g_slist_length((GSList *) selection->itemList()) != 2) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a text and a path</b> to put text on path."));
        return;
    }

    if (SP_IS_TEXT_TEXTPATH(text)) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("This text object is <b>already put to a path</b>. Remove it from the path first. Use <b>Shift+D</b> to look up its path."));
        return;
    }

    if (SP_IS_RECT(shape)) {
        // rect is the only SPShape which is not <path> yet, and thus SVG forbids us from putting text on it
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("You cannot put text on a rectangle in this version. Convert rectangle to path first."));
        return;
    }

    Inkscape::Text::Layout const *layout = te_get_layout(text);
    Inkscape::Text::Layout::Alignment text_alignment = layout->paragraphAlignment(layout->begin());

    // remove transform from text, but recursively scale text's fontsize by the expansion
    SP_TEXT(text)->_adjustFontsizeRecursive (text, NR::expansion(SP_ITEM(text)->transform));
    sp_repr_set_attr(SP_OBJECT_REPR(text), "transform", NULL);

    // make a list of text children
    GSList *text_reprs = NULL;
    for (SPObject *o = SP_OBJECT(text)->children; o != NULL; o = o->next) {
        text_reprs = g_slist_prepend(text_reprs, SP_OBJECT_REPR(o));
    }

    // create textPath and put it into the text
    Inkscape::XML::Node *textpath = sp_repr_new("svg:textPath");
    // reference the shape
    sp_repr_set_attr(textpath, "xlink:href", g_strdup_printf("#%s", SP_OBJECT_REPR(shape)->attribute("id")));
    if (text_alignment == Inkscape::Text::Layout::RIGHT)
        sp_repr_set_attr(textpath, "startOffset", "100%");
    else if (text_alignment == Inkscape::Text::Layout::CENTER)
        sp_repr_set_attr(textpath, "startOffset", "50%");
    sp_repr_add_child(SP_OBJECT_REPR(text), textpath, NULL);

    for ( GSList *i = text_reprs ; i ; i = i->next ) {
        // make a copy of each text child
        Inkscape::XML::Node *copy = ((Inkscape::XML::Node *) i->data)->duplicate();
        // We cannot have multiline in textpath, so remove line attrs from tspans
        if (!strcmp(copy->name(), "svg:tspan")) {
            sp_repr_set_attr(copy, "sodipodi:role", NULL);
            sp_repr_set_attr(copy, "x", NULL);
            sp_repr_set_attr(copy, "y", NULL);
        }
        // remove the old repr from under text
        sp_repr_remove_child(SP_OBJECT_REPR(text), (Inkscape::XML::Node *) i->data);
        // put its copy into under textPath
        sp_repr_add_child(textpath, copy, NULL); // fixme: copy id
    }

    // x/y are useless with textpath, and confuse Batik 1.5
    sp_repr_set_attr(SP_OBJECT_REPR(text), "x", NULL);
    sp_repr_set_attr(SP_OBJECT_REPR(text), "y", NULL);

    sp_document_done(SP_DT_DOCUMENT(desktop));
    g_slist_free(text_reprs);
}

void
text_remove_from_path()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a text on path</b> to remove it from path."));
        return;
    }

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        if (!SP_IS_TEXT_TEXTPATH(SP_OBJECT(items->data))) {
            continue;
        }

        SPObject *tp = sp_object_first_child(SP_OBJECT(items->data));

        did = true;

        sp_textpath_to_text(tp);
    }

    if (!did) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No texts-on-paths</b> in the selection."));
    } else {
        selection->setList(g_slist_copy((GSList *) selection->itemList())); // reselect to update statusbar description
        sp_document_done(SP_DT_DOCUMENT(desktop));
    }
}

void
text_remove_all_kerns_recursively(SPObject *o)
{
    sp_repr_set_attr(SP_OBJECT_REPR(o), "dx", NULL);
    sp_repr_set_attr(SP_OBJECT_REPR(o), "dy", NULL);
    sp_repr_set_attr(SP_OBJECT_REPR(o), "rotate", NULL);

    for (SPObject *i = sp_object_first_child(o); i != NULL; i = SP_OBJECT_NEXT(i)) {
        text_remove_all_kerns_recursively(i);
    }
}

//FIXME: must work with text selection
void
text_remove_all_kerns()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>text(s)</b> to remove kerns from."));
        return;
    }

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        if (!SP_IS_TEXT(SP_OBJECT(items->data))) {
            continue;
        }

        text_remove_all_kerns_recursively(SP_OBJECT(items->data));
        SP_OBJECT(items->data)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
        did = true;
    }

    if (!did) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Select <b>text(s)</b> to remove kerns from."));
    } else {
        sp_document_done(SP_DT_DOCUMENT(desktop));
    }
}

void
text_flow_into_shape()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return;

    SPDocument *doc = SP_DT_DOCUMENT (desktop);

    Inkscape::Selection *selection = SP_DT_SELECTION(desktop);

    SPItem *text = text_in_selection(selection);
    SPItem *shape = shape_in_selection(selection);

    if (!text || !shape || g_slist_length((GSList *) selection->itemList()) < 2) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a text</b> and one or more <b>paths or shapes</b> to flow text into frame."));
        return;
    }

    // remove transform from text, but recursively scale text's fontsize by the expansion
    SP_TEXT(text)->_adjustFontsizeRecursive(text, NR::expansion(SP_ITEM(text)->transform));
    sp_repr_set_attr(SP_OBJECT_REPR(text), "transform", NULL);

    Inkscape::XML::Node *root_repr = sp_repr_new("svg:flowRoot");
    sp_repr_set_attr(root_repr, "style", SP_OBJECT_REPR(text)->attribute("style")); // fixme: transfer style attrs too
    SP_OBJECT_REPR(SP_OBJECT_PARENT(shape))->appendChild(root_repr);
    SPObject *root_object = doc->getObjectByRepr(root_repr);
    g_return_if_fail(SP_IS_FLOWTEXT(root_object));

    Inkscape::XML::Node *region_repr = sp_repr_new("svg:flowRegion");
    root_repr->appendChild(region_repr);
    SPObject *object = doc->getObjectByRepr(region_repr);
    g_return_if_fail(SP_IS_FLOWREGION(object));

    /* Add clones */
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        SPItem *item = SP_ITEM(items->data);
        if (SP_IS_SHAPE(item)){
            Inkscape::XML::Node *clone = sp_repr_new("svg:use");
            sp_repr_set_attr(clone, "x", "0");
            sp_repr_set_attr(clone, "y", "0");
            sp_repr_set_attr(clone, "xlink:href", g_strdup_printf("#%s", SP_OBJECT_REPR(item)->attribute("id")));

            // add the new clone to the region
            region_repr->appendChild(clone);
        }
    }

    Inkscape::XML::Node *div_repr = sp_repr_new("svg:flowDiv");
    sp_repr_set_attr(div_repr, "xml:space", "preserve"); // we preserve spaces in the text objects we create
    root_repr->appendChild(div_repr);
    SPObject *div_object = doc->getObjectByRepr(div_repr);
    g_return_if_fail(SP_IS_FLOWDIV(div_object));

    Inkscape::XML::Node *para_repr = sp_repr_new("svg:flowPara");
    div_repr->appendChild(para_repr);
    object = doc->getObjectByRepr(para_repr);
    g_return_if_fail(SP_IS_FLOWPARA(object));

    Inkscape::Text::Layout const *layout = te_get_layout(text);
    Glib::ustring text_ustring = sp_te_get_string_multiline(text, layout->begin(), layout->end());

    Inkscape::XML::Node *text_repr = sp_repr_new_text(text_ustring.c_str()); // FIXME: transfer all formatting!!!
    para_repr->appendChild(text_repr);

    SP_OBJECT(text)->deleteObject (true);

    sp_document_done(doc);

    SP_DT_SELECTION(desktop)->set(SP_ITEM(root_object));

    sp_repr_unref(root_repr);
    sp_repr_unref(div_repr);
    sp_repr_unref(region_repr);
    sp_repr_unref(para_repr);
    sp_repr_unref(text_repr);
}

void
text_unflow ()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return;

    SPDocument *doc = SP_DT_DOCUMENT (desktop);

    Inkscape::Selection *selection = SP_DT_SELECTION(desktop);


    if (!flowtext_in_selection(selection) || g_slist_length((GSList *) selection->itemList()) < 1) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a flowed text</b> to unflow it."));
        return;
    }

    GSList *new_objs = NULL;
    GSList *old_objs = NULL;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        if (!SP_IS_FLOWTEXT(SP_OBJECT(items->data))) {
            continue;
        }

        SPItem *flowtext = SP_ITEM(items->data);

        /* Create <text> */
        Inkscape::XML::Node *rtext = sp_repr_new("svg:text");
        sp_repr_set_attr(rtext, "xml:space", "preserve"); // we preserve spaces in the text objects we create

        /* Set style */
        sp_repr_set_attr(rtext, "style", SP_OBJECT_REPR(flowtext)->attribute("style")); // fixme: transfer style attrs too; and from descendants

        NRRect bbox;
        sp_item_invoke_bbox(SP_ITEM(flowtext), &bbox, sp_item_i2doc_affine(SP_ITEM(flowtext)), TRUE);
        NR::Point xy(bbox.x0, bbox.y0);
        if (xy[NR::X] != 1e18 && xy[NR::Y] != 1e18) {
            sp_repr_set_double(rtext, "x", xy[NR::X]);
            sp_repr_set_double(rtext, "y", xy[NR::Y]);
        }

        /* Create <tspan> */
        Inkscape::XML::Node *rtspan = sp_repr_new("svg:tspan");
        sp_repr_set_attr(rtspan, "sodipodi:role", "line"); // otherwise, why bother creating the tspan?
        sp_repr_add_child(rtext, rtspan, NULL);

        Inkscape::Text::Layout const *layout = te_get_layout(flowtext);
        Glib::ustring text_ustring = sp_te_get_string_multiline(flowtext, layout->begin(), layout->end());

        Inkscape::XML::Node *text_repr = sp_repr_new_text(text_ustring.c_str()); // FIXME: transfer all formatting!!!
        rtspan->appendChild(text_repr);

        SP_OBJECT_REPR(SP_OBJECT_PARENT(flowtext))->appendChild(rtext);
        SPObject *text_object = doc->getObjectByRepr(rtext);

        new_objs = g_slist_prepend (new_objs, text_object);
        old_objs = g_slist_prepend (old_objs, flowtext);

        sp_repr_unref(rtext);
        sp_repr_unref(rtspan);
        sp_repr_unref(text_repr);
    }

    selection->clear();
    selection->setList(new_objs);
    for (GSList *i = old_objs; i; i = i->next) {
        SP_OBJECT(i->data)->deleteObject (true);
    }

    g_slist_free (old_objs);
    g_slist_free (new_objs);

    sp_document_done(doc);
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
