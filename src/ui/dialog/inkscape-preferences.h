/**
 * \brief Inkscape Preferences dialog
 *
 * Authors:
 *   Carl Hetherington
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H
#define INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H

#include <gtkmm/notebook.h>
#include <gtkmm/adjustment.h>
#include <glibmm/i18n.h>

#include "dialog.h"
#include "ui/widget/notebook-page.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class InkscapePreferences : public Dialog {
public:
    InkscapePreferences();
    virtual ~InkscapePreferences();

    static InkscapePreferences *create() { return new InkscapePreferences(); }

protected:
    Gtk::Notebook    _notebook;
    Gtk::VBox        _page_mouse;
    Gtk::VBox        _page_scrolling;
    Gtk::VBox        _page_steps;
    Gtk::VBox        _page_tools;
    Gtk::VBox        _page_windows;
    Gtk::VBox        _page_clones;
    Gtk::VBox        _page_transforms;
    Gtk::VBox        _page_misc;

    Gtk::Adjustment  _page_mouse_adj_sensitivity;
    Gtk::Adjustment  _page_mouse_adj_threshold;
    Gtk::Adjustment  _page_scrolling_adj_wheel;
    Gtk::Adjustment  _page_scrolling_adj_ctrl_scroll;
    Gtk::Adjustment  _page_scrolling_adj_ctrl_accel;
    Gtk::Adjustment  _page_scrolling_adj_auto_speed;
    Gtk::Adjustment  _page_scrolling_adj_auto_threshold;
    Gtk::Adjustment  _page_steps_adj_move;
    Gtk::Adjustment  _page_steps_adj_scale;
    Gtk::Adjustment  _page_steps_adj_inset;
    Gtk::Adjustment  _page_steps_adj_zoom;

    void initPageMouse();
    void initPageScrolling();
    void initPageSteps();
    void initPageTools();
    void initPageWindows();
    void initPageClones();
    void initPageTransforms();
    void initPageMisc();

private:
    InkscapePreferences(InkscapePreferences const &d);
    InkscapePreferences operator=(InkscapePreferences const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
