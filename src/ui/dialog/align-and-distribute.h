// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief Align and Distribute widget
 */
/* Authors:
 *   Tavmjong Bah
 *
 *   Based on dialog by:
 *     Bryce W. Harrington <bryce@bryceharrington.org>
 *     Aubanel MONNIER <aubi@libertysurf.fr>
 *     Frank Felfe <innerspace@iname.com>
 *     Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2021 Tavmjong Bah
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_ALIGN_AND_DISTRIBUTE_H
#define INKSCAPE_UI_WIDGET_ALIGN_AND_DISTRIBUTE_H

#include <sigc++/connection.h>
#include <gtkmm.h>

class SPDesktop;

namespace Inkscape {
namespace UI {

namespace Tools {
class ToolBase;
}

namespace Dialog {
class DialogBase;

class AlignAndDistribute : public Gtk::Box
{
public:
    AlignAndDistribute(Inkscape::UI::Dialog::DialogBase* dlg);
    ~AlignAndDistribute() override = default;

    void desktop_changed(SPDesktop* desktop);
    void tool_changed(SPDesktop* desktop); // Need to show different widgets for node vs. other tools.
    void tool_changed_callback(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec);

private:

    // ********* Widgets ********** //

    Gtk::Box* align_and_distribute_box    = nullptr;
    Gtk::Box* align_and_distribute_object = nullptr; // Hidden when node tool active.
    Gtk::Box* align_and_distribute_node   = nullptr; // Visible when node tool active.

    // Align
    Gtk::ToggleButton* align_move_as_group      = nullptr;
    Gtk::ComboBox*     align_relative_object    = nullptr;
    Gtk::ComboBox*     align_relative_node      = nullptr;

    // Remove overlap
    Gtk::Button*       remove_overlap_button  = nullptr;
    Gtk::SpinButton*   remove_overlap_hgap    = nullptr;
    Gtk::SpinButton*   remove_overlap_vgap    = nullptr;


    // ********* Signal handlers ********** //

    void on_align_as_group_clicked();
    void on_align_relative_object_changed();
    void on_align_relative_node_changed();

    bool on_align_button_press_event(GdkEventButton* button_event, const std::string& align_to);
    bool on_remove_overlap_button_press_event(GdkEventButton* button_event);
    bool on_align_node_button_press_event(GdkEventButton* button_event, const std::string& align_to);

    sigc::connection tool_connection;

};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_ALIGN_AND_DISTRIBUTE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
