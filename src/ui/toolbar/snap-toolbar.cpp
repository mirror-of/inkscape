// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * @file
 * Inkscape Snap toolbar
 *
 * @authors Inkscape Authors
 * Copyright (C) 1999-2019 authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#include "snap-toolbar.h"

#include <glibmm/i18n.h>

#include "attributes.h"
#include "desktop.h"
#include "verbs.h"

#include "object/sp-namedview.h"
#include "helper/action.h"

#include "ui/icon-names.h"

namespace Inkscape {
namespace UI {
namespace Toolbar {

Gtk::ToggleToolButton *
SnapToolbar::add_toggle_snap_verb(int verb_id)
{
    auto verb = Inkscape::Verb::get(verb_id);
    auto button = add_toggle_button(verb->get_name(), verb->get_tip());
    button->set_icon_name(verb->get_image());
    button->signal_toggled().connect(
        sigc::bind(sigc::mem_fun(*this, &SnapToolbar::on_snap_toggled_verb), verb_id));
    return button;
}

SnapToolbar::SnapToolbar(SPDesktop *desktop)
    : Toolbar(desktop),
    _freeze(false)
{
    // Global snapping control
    _snap_global_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_GLOBAL);
    add_separator();
    _snap_from_bbox_corner_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_BBOX);
    _snap_to_bbox_path_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_BBOX_EDGE);
    _snap_to_bbox_node_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_BBOX_CORNER);
    _snap_to_from_bbox_edge_midpoints_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_BBOX_EDGE_MIDPOINT);
    _snap_to_from_bbox_edge_centers_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_BBOX_MIDPOINT);
    add_separator();
    _snap_from_node_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_NODE);
    _snap_to_item_path_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_PATH);
    _snap_to_path_intersections_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_PATH_INTERSECTION);
    _snap_to_item_node_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_NODE_CUSP);
    _snap_to_smooth_nodes_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_NODE_SMOOTH);
    _snap_to_from_line_midpoints_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_LINE_MIDPOINT);
    add_separator();
    _snap_from_others_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_OTHERS);
    _snap_to_from_object_centers_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_OBJECT_MIDPOINT);
    _snap_to_from_rotation_center_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_ROTATION_CENTER);
    _snap_to_from_text_baseline_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_TEXT_BASELINE);
    add_separator();
    _snap_to_page_border_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_PAGE_BORDER);
    _snap_to_grids_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_GRID);
    _snap_to_guides_item = this->add_toggle_snap_verb(SP_VERB_TOGGLE_SNAP_GUIDE);
    show_all();
}

GtkWidget *
SnapToolbar::create(SPDesktop *desktop)
{
    auto tb = Gtk::manage(new SnapToolbar(desktop));
    return GTK_WIDGET(tb->gobj());
}

void
SnapToolbar::update(SnapToolbar *tb)
{
    auto nv = tb->_desktop->getNamedView();

    if (nv == nullptr) {
        g_warning("Namedview cannot be retrieved (in updateSnapToolbox)!");
        return;
    }

    // The ..._set_active calls below will toggle the buttons, but this shouldn't lead to
    // changes in our document because we're only updating the UI;
    // Setting the "freeze" parameter to true will block the code in toggle_snap_callback()
    tb->_freeze = true;

    bool const c1 = nv->snap_manager.snapprefs.getSnapEnabledGlobally();
    tb->_snap_global_item->set_active(c1);

    bool const c2 = nv->snap_manager.snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CATEGORY);
    tb->_snap_from_bbox_corner_item->set_active(c2);
    tb->_snap_from_bbox_corner_item->set_sensitive(c1);

    tb->_snap_to_bbox_path_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(SNAPTARGET_BBOX_EDGE));
    tb->_snap_to_bbox_path_item->set_sensitive(c1 && c2);

    tb->_snap_to_bbox_node_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(SNAPTARGET_BBOX_CORNER));
    tb->_snap_to_bbox_node_item->set_sensitive(c1 && c2);

    tb->_snap_to_from_bbox_edge_midpoints_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(SNAPTARGET_BBOX_EDGE_MIDPOINT));
    tb->_snap_to_from_bbox_edge_midpoints_item->set_sensitive(c1 && c2);
    tb->_snap_to_from_bbox_edge_centers_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(SNAPTARGET_BBOX_MIDPOINT));
    tb->_snap_to_from_bbox_edge_centers_item->set_sensitive(c1 && c2);

    bool const c3 = nv->snap_manager.snapprefs.isTargetSnappable(SNAPTARGET_NODE_CATEGORY);
    tb->_snap_from_node_item->set_active(c3);
    tb->_snap_from_node_item->set_sensitive(c1);

    tb->_snap_to_item_path_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH));
    tb->_snap_to_item_path_item->set_sensitive(c1 && c3);
    tb->_snap_to_path_intersections_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_INTERSECTION));
    tb->_snap_to_path_intersections_item->set_sensitive(c1 && c3);
    tb->_snap_to_item_node_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_CUSP));
    tb->_snap_to_item_node_item->set_sensitive(c1 && c3);
    tb->_snap_to_smooth_nodes_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_SMOOTH));
    tb->_snap_to_smooth_nodes_item->set_sensitive(c1 && c3);
    tb->_snap_to_from_line_midpoints_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_LINE_MIDPOINT));
    tb->_snap_to_from_line_midpoints_item->set_sensitive(c1 && c3);

    bool const c5 = nv->snap_manager.snapprefs.isTargetSnappable(SNAPTARGET_OTHERS_CATEGORY);
    tb->_snap_from_others_item->set_active(c5);
    tb->_snap_from_others_item->set_sensitive(c1);
    tb->_snap_to_from_object_centers_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_OBJECT_MIDPOINT));
    tb->_snap_to_from_object_centers_item->set_sensitive(c1 && c5);
    tb->_snap_to_from_rotation_center_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_ROTATION_CENTER));
    tb->_snap_to_from_rotation_center_item->set_sensitive(c1 && c5);
    tb->_snap_to_from_text_baseline_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_TEXT_BASELINE));
    tb->_snap_to_from_text_baseline_item->set_sensitive(c1 && c5);
    tb->_snap_to_page_border_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PAGE_BORDER));
    tb->_snap_to_page_border_item->set_sensitive(c1);
    tb->_snap_to_grids_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GRID));
    tb->_snap_to_grids_item->set_sensitive(c1);
    tb->_snap_to_guides_item->set_active(nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GUIDE));
    tb->_snap_to_guides_item->set_sensitive(c1);

    tb->_freeze = false;
}

void
SnapToolbar::on_snap_toggled_verb(int verb_id)
{
    if(_freeze) return;

    Inkscape::Verb *verb = Inkscape::Verb::get( verb_id );
    g_assert( verb != NULL );
    SPAction *action = verb->get_action((Inkscape::UI::View::View *) _desktop);
    sp_action_perform (action, NULL);
}

}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
