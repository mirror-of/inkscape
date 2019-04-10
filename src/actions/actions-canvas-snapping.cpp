// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for toggling snapping preferences. Tied to a particular document.
 *
 * Copyright (C) 2019 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!

#include "actions-canvas-snapping.h"

#include "document.h"

#include "attributes.h"
#include "desktop.h"
#include "document-undo.h"

#include "object/sp-namedview.h"

// There are four snapping lists that must be connected:
// 1. The attribute name in NamedView: e.g. "inkscape:snap-bbox".
// 2. The SPAttributeEnum value:       e.g. SP_ATTR_INKSCAPE_SNAP_BBOX.
// 3. The Inkscape::SNAPTARGET value:  e.g. Inkscape::SNAPTARGET_BBOX_CATEGORY.
// 4. The Gio::Action name:            e.g. "snap-bbox"
// It seems we could simplify this somehow.

// This might work better as a class.

static void
canvas_snapping_toggle(SPDocument* document, const int option)
{
    std::cout << "canvas_snapping_toggle: " << option << std::endl;

    Inkscape::XML::Node* repr = document->getReprNamedView();

    if (repr == nullptr) {
        std::cerr << "canvas_snapping_toggle: namedview XML repr missing!" << std::endl;
        return;
    }

    // This is a bit ackward.
    SPObject* obj = document->getObjectByRepr(repr);
    SPNamedView* nv = dynamic_cast<SPNamedView *> (obj);
    if (nv == nullptr) {
        std::cerr << "canvas_snapping_toggle: no namedview!" << std::endl;
        return;
    }

    // Disable undo
    Inkscape::DocumentUndo::ScopedInsensitive _no_undo(document);

    bool v = false;

    switch (option) {
        case SP_ATTR_INKSCAPE_SNAP_GLOBAL:
            v = nv->getSnapGlobal();
            nv->setSnapGlobal(!v); // Calls sp_repr_set_boolean
            break;

        // BBox
        case SP_ATTR_INKSCAPE_SNAP_BBOX:
            v = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_BBOX_CATEGORY);
            sp_repr_set_boolean(repr, "inkscape:snap-bbox", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_EDGE);
            sp_repr_set_boolean(repr, "inkscape:bbox-paths", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_BBOX_CORNER:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_CORNER);
            sp_repr_set_boolean(repr, "inkscape:bbox-nodes", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE_MIDPOINT:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_EDGE_MIDPOINT);
            sp_repr_set_boolean(repr, "inkscape:snap-bbox-edge-midpoints", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_BBOX_MIDPOINT:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_MIDPOINT);
            sp_repr_set_boolean(repr, "inkscape:snap-bbox-midpoints", !v);
            break;

        // Nodes
        case SP_ATTR_INKSCAPE_SNAP_NODE:
            v = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_NODE_CATEGORY);
            sp_repr_set_boolean(repr, "inkscape:snap-nodes", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_PATH:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH);
            sp_repr_set_boolean(repr, "inkscape:object-paths", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_PATH_INTERSECTION:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_INTERSECTION);
            sp_repr_set_boolean(repr, "inkscape:snap-intersection-paths", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_NODE_CUSP:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_CUSP);
            sp_repr_set_boolean(repr, "inkscape:object-nodes", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_NODE_SMOOTH:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_SMOOTH);
            sp_repr_set_boolean(repr, "inkscape:snap-smooth-nodes", !v);
            break;


        case SP_ATTR_INKSCAPE_SNAP_LINE_MIDPOINT:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_LINE_MIDPOINT);
            sp_repr_set_boolean(repr, "inkscape:snap-midpoints", !v);
            break;

        // Others
        case SP_ATTR_INKSCAPE_SNAP_OTHERS:
            v = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_OTHERS_CATEGORY);
            sp_repr_set_boolean(repr, "inkscape:snap-others", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_OBJECT_MIDPOINT:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_OBJECT_MIDPOINT);
            sp_repr_set_boolean(repr, "inkscape:snap-object-midpoints", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_ROTATION_CENTER:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_ROTATION_CENTER);
            sp_repr_set_boolean(repr, "inkscape:snap-center", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_TEXT_BASELINE:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_TEXT_BASELINE);
            sp_repr_set_boolean(repr, "inkscape:snap-text-baseline", !v);
            break;

        // Page/Grid/Guides
        case SP_ATTR_INKSCAPE_SNAP_PAGE_BORDER:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PAGE_BORDER);
            sp_repr_set_boolean(repr, "inkscape:snap-page", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_GRID:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GRID);
            sp_repr_set_boolean(repr, "inkscape:snap-grids", !v);
            break;

        case SP_ATTR_INKSCAPE_SNAP_GUIDE:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GUIDE);
            sp_repr_set_boolean(repr, "inkscape:snap-to-guides", !v);
            break;

        // Not used in default snap toolbar
        case SP_ATTR_INKSCAPE_SNAP_PATH_CLIP:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_CLIP);
            sp_repr_set_boolean(repr, "inkscape:snap-path-clip", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_PATH_MASK:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_MASK);
            sp_repr_set_boolean(repr, "inkscape:snap-path-mask", !v);
            break;

        default:
            std::cerr << "canvas_snapping_toggle: unhandled option!" << std::endl;
    }

    // Some actions depend on others... we need to update everything!
    set_actions_canvas_snapping(document);

    // The snapping preferences are stored in the document, and therefore toggling makes the document dirty.
    document->setModifiedSinceSave();

}

void
add_actions_canvas_snapping(SPDocument* document)
{
    Glib::RefPtr<Gio::SimpleActionGroup> map = document->getActionGroup();

    map->add_action_bool( "snap-global-toggle",      sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_GLOBAL));

    map->add_action_bool( "snap-bbox",               sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_BBOX));
    map->add_action_bool( "snap-bbox-edge",          sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE));
    map->add_action_bool( "snap-bbox-corner",        sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_BBOX_CORNER));
    map->add_action_bool( "snap-bbox-edge-midpoint", sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE_MIDPOINT));
    map->add_action_bool( "snap-bbox-center",        sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_BBOX_MIDPOINT));

    map->add_action_bool( "snap-node-category",      sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_NODE));
    map->add_action_bool( "snap-path",               sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_PATH));
    map->add_action_bool( "snap-path-intersection",  sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_PATH_INTERSECTION));
    map->add_action_bool( "snap-node-cusp",          sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_NODE_CUSP));
    map->add_action_bool( "snap-node-smooth",        sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_NODE_SMOOTH));
    map->add_action_bool( "snap-line-midpoint",      sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_LINE_MIDPOINT));

    map->add_action_bool( "snap-others",             sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_OTHERS));
    map->add_action_bool( "snap-object-midpoint",    sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_OBJECT_MIDPOINT));
    map->add_action_bool( "snap-rotation-center",    sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_ROTATION_CENTER));
    map->add_action_bool( "snap-text-baseline",      sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_TEXT_BASELINE));

    map->add_action_bool( "snap-page-border",        sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_PAGE_BORDER));
    map->add_action_bool( "snap-grid",               sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_GRID));
    map->add_action_bool( "snap-guide",              sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_GUIDE));

    // Not used in toolbar
    map->add_action_bool( "snap-path-mask",          sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_PATH_MASK));
    map->add_action_bool( "snap-path-clip",          sigc::bind<SPDocument*, int>(sigc::ptr_fun(&canvas_snapping_toggle),  document, SP_ATTR_INKSCAPE_SNAP_PATH_CLIP));
}

// We can't enable/disable action directly! (Gio::Action can "get" enabled value but can not "set" it! We need to cast to Gio::SimpleAction)
void
set_actions_canvas_snapping_helper (Glib::RefPtr<Gio::Action>& action, bool state, bool enabled)
{
    auto simple = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    simple->change_state(state);
    simple->set_enabled(enabled);
}

void
set_actions_canvas_snapping(SPDocument* document)
{
    Inkscape::XML::Node* repr = document->getReprNamedView();

    if (repr == nullptr) {
        std::cerr << "set_actions_canvas_snapping: namedview XML repr missing!" << std::endl;
        return;
    }

    // This is a bit ackward.
    SPObject* obj = document->getObjectByRepr(repr);
    SPNamedView* nv = dynamic_cast<SPNamedView *> (obj);

    if (nv == nullptr) {
        std::cerr << "set_actions_canvas_snapping: no namedview!" << std::endl;
        return;
    }

    Glib::RefPtr<Gio::SimpleActionGroup> map = document->getActionGroup();

    Glib::RefPtr<Gio::Action> action;

    bool global = nv->snap_manager.snapprefs.getSnapEnabledGlobally();       
    action = map->lookup_action( "snap-global-toggle");      action->change_state(global);

    bool bbox = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_BBOX_CATEGORY);
    action = map->lookup_action( "snap-bbox");               set_actions_canvas_snapping_helper(action, bbox, global);
    action = map->lookup_action( "snap-bbox-edge");          set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_EDGE),          global && bbox);
    action = map->lookup_action( "snap-bbox-corner");        set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_CORNER),        global && bbox);
    action = map->lookup_action( "snap-bbox-edge-midpoint"); set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_EDGE_MIDPOINT), global && bbox);
    action = map->lookup_action( "snap-bbox-center");        set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_MIDPOINT),      global && bbox);

    bool node = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_NODE_CATEGORY);
    action = map->lookup_action( "snap-node-category");      set_actions_canvas_snapping_helper(action, node, global);
    action = map->lookup_action( "snap-path");               set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH),               global && node);
    action = map->lookup_action( "snap-path-intersection");  set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_INTERSECTION),  global && node);
    action = map->lookup_action( "snap-node-cusp");          set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_CUSP),          global && node);
    action = map->lookup_action( "snap-node-smooth");        set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_SMOOTH),        global && node);
    action = map->lookup_action( "snap-line-midpoint");      set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_LINE_MIDPOINT),      global && node);

    bool other = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_OTHERS_CATEGORY);
    action = map->lookup_action( "snap-others");             set_actions_canvas_snapping_helper(action, other, global);
    action = map->lookup_action( "snap-object-midpoint");    set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_OBJECT_MIDPOINT),   global && other);
    action = map->lookup_action( "snap-rotation-center");    set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_ROTATION_CENTER),   global && other);
    action = map->lookup_action( "snap-text-baseline");      set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_TEXT_BASELINE),     global && other);

    action = map->lookup_action( "snap-page-border");        set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PAGE_BORDER),        global);
    action = map->lookup_action( "snap-grid");               set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GRID),               global);
    action = map->lookup_action( "snap-guide");              set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GUIDE),              global);

    action = map->lookup_action( "snap-path-mask");          set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_CLIP),          global);
    action = map->lookup_action( "snap-path-clip");          set_actions_canvas_snapping_helper(action, nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_MASK),          global);
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
