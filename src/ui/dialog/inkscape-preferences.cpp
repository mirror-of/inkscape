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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/frame.h>
#include <gtkmm/comboboxtext.h>

#include "inkscape-preferences.h"
#include "ui/widget/labelled.h"
#include "ui/widget/scalar.h"
#include "verbs.h"

// NOTE: this is currently a placeholder. The real dialog is in src/dialogs/display-settings.cpp.
// The below code never runs, Please remove this notice when you gtkmmify this dialog and remove
// the file in src/dialogs.

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

InkscapePreferences::InkscapePreferences()
    : Dialog ("dialogs.preferences", SP_VERB_DIALOG_DISPLAY),
      _page_mouse_adj_sensitivity(0, 0, 30, 1, 1, 1), // TODO: set up initial values
      _page_mouse_adj_threshold(0, 0, 20, 1, 1, 1),
      _page_scrolling_adj_wheel(0, 0, 1000, 1, 1, 1),
      _page_scrolling_adj_ctrl_scroll(0, 0, 1000, 1, 1, 1),
      _page_scrolling_adj_ctrl_accel(0, 0, 5, 0.01, 1, 1),
      _page_scrolling_adj_auto_speed(0, 0, 5, 0.01, 1, 1),
      _page_scrolling_adj_auto_threshold(0, -600, 600, 1, 1, 1),
      _page_steps_adj_move(0, 0, 3000, 0.01, 1, 1),
      _page_steps_adj_scale(0, 0, 3000, 0.01, 1, 1),
      _page_steps_adj_inset(0, 0, 3000, 0.01, 1, 1),
      _page_steps_adj_zoom(0, 101, 500, 1, 1, 1)
{
    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);

    // Notebook for individual transformations
    vbox->pack_start(_notebook, true, true);

    _notebook.append_page(_page_mouse,      _("Mouse"));
    _notebook.append_page(_page_scrolling,  _("Scrolling"));
    _notebook.append_page(_page_steps,      _("Steps"));
    _notebook.append_page(_page_tools,      _("Tools"));
    _notebook.append_page(_page_windows,    _("Windows"));
    _notebook.append_page(_page_clones,     _("Clones"));
    _notebook.append_page(_page_transforms, _("Transforms"));
    _notebook.append_page(_page_misc,       _("Misc"));

    initPageMouse();
    initPageScrolling();
    initPageSteps();
    initPageTools();
    initPageWindows();
    initPageClones();
    initPageTransforms();
    initPageMisc();

    show_all_children();
}

InkscapePreferences::~InkscapePreferences()
{
}

void InkscapePreferences::initPageMouse()
{
/*
// example:
    _page_mouse.pack_start(*Gtk::manage(new Scalar(_("Grab sensitivity"), _("tooltip")
                                                   _page_mouse_adj_sensitivity, 0,
                                                   _("px"))),
                           false, false, 0);
*/
}

void InkscapePreferences::initPageScrolling()
{
/*
// example:
    Gtk::Frame *f = new Gtk::Frame(_("Ctrl+arrows"));
    _page_scrolling.pack_start(*Gtk::manage(f));
    Gtk::Box *v = new Gtk::VBox;
    f->add(*v);

    f = new Gtk::Frame(_("Autoscrolling"));
    _page_scrolling.pack_start(*Gtk::manage(f));
    v = new Gtk::VBox;
    f->add(*v);
*/
}

void InkscapePreferences::initPageSteps()
{
/*
// example:
    Gtk::ComboBoxText *c = new Gtk::ComboBoxText;
    c->append_text("90");
    c->append_text("60");
    c->append_text("45");
    c->append_text("30");
    c->append_text("15");
    c->append_text("10");
    c->append_text("7.5");
    c->append_text("6");
    c->append_text("3");
    c->append_text("2");
    c->append_text("1");
    c->append_text(_("None"));
*/
}

void InkscapePreferences::initPageTools()
{
}

void InkscapePreferences::initPageWindows()
{
}

void InkscapePreferences::initPageClones()
{
}

void InkscapePreferences::initPageTransforms()
{
}

void InkscapePreferences::initPageMisc()
{
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
