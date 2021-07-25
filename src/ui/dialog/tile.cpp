// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * A simple dialog for creating grid type arrangements of selected objects
 *
 * Authors:
 *   Bob Jamison ( based off trace dialog)
 *   John Cliff
 *   Other dudes from The Inkscape Organization
 *   Abhishek Sharma
 *   Declara Denis
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "ui/dialog/grid-arrange-tab.h"
#include "ui/dialog/polar-arrange-tab.h"
#include "ui/dialog/align-and-distribute.h"
#include "ui/icon-names.h"

#include <glibmm/i18n.h>

#include "tile.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

Gtk::Box& create_tab_label(const char* label_text, const char* icon_name) {
    auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
    auto image = Gtk::make_managed<Gtk::Image>();
    image->set_from_icon_name(icon_name, Gtk::ICON_SIZE_MENU);
    auto label = Gtk::make_managed<Gtk::Label>(label_text, true);
    box->pack_start(*image, false, true);
    box->pack_start(*label, false, true);
    box->show_all();
    return *box;
}

ArrangeDialog::ArrangeDialog()
    : DialogBase("/dialogs/gridtiler", "AlignDistribute")
{
    _align_tab = Gtk::manage(new AlignAndDistribute(this));
    _arrangeBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    _notebook = Gtk::manage(new Gtk::Notebook());
    _gridArrangeTab = Gtk::manage(new GridArrangeTab(this));
    _polarArrangeTab = Gtk::manage(new PolarArrangeTab(this));

    _notebook->append_page(*_align_tab, create_tab_label(C_("Arrange dialog", "Align"), INKSCAPE_ICON("dialog-align-and-distribute")));
    // TRANSLATORS: "Grid" refers to grid (columns/rows) arrangement
    _notebook->append_page(*_gridArrangeTab, create_tab_label(C_("Arrange dialog", "Grid"), INKSCAPE_ICON("arrange-grid")));
    // TRANSLATORS: "Circular" refers to circular/radial arrangement
    _notebook->append_page(*_polarArrangeTab, create_tab_label(C_("Arrange dialog", "Circular"), INKSCAPE_ICON("arrange-circular")));
    _arrangeBox->pack_start(*_notebook);
    _notebook->signal_switch_page().connect([=](Widget*, guint page){
        update_arrange_btn();
    });
    pack_start(*_arrangeBox);

    // Add button
    _arrangeButton = Gtk::manage(new Gtk::Button(C_("Arrange dialog", "_Arrange")));
    _arrangeButton->signal_clicked().connect(sigc::mem_fun(*this, &ArrangeDialog::_apply));
    _arrangeButton->set_use_underline(true);
    _arrangeButton->set_tooltip_text(_("Arrange selected objects"));

    Gtk::ButtonBox *button_box = Gtk::manage(new Gtk::ButtonBox());
    button_box->set_layout(Gtk::BUTTONBOX_END);
    button_box->set_spacing(6);
    button_box->set_border_width(4);
    button_box->set_valign(Gtk::ALIGN_START);

    button_box->pack_end(*_arrangeButton);
    pack_end(*button_box);

    show();
    show_all_children();
    set_no_show_all();
    update_arrange_btn();
}

void ArrangeDialog::update_arrange_btn() {
    // "align" page doesn't use "Arrange" button
    if (_notebook->get_current_page() == 0) {
        _arrangeButton->hide();
    }
    else {
        _arrangeButton->show();
    }
}

ArrangeDialog::~ArrangeDialog()
{ }

void ArrangeDialog::_apply()
{
	switch(_notebook->get_current_page())
	{
	case 0:
        // not applicable to align panel
        break;
	case 1:
		_gridArrangeTab->arrange();
		break;
	case 2:
		_polarArrangeTab->arrange();
		break;
	}
}

void ArrangeDialog::desktopReplaced()
{
    _gridArrangeTab->setDesktop(getDesktop());
    _align_tab->desktopReplaced();
}

void ArrangeDialog::selectionChanged(Inkscape::Selection* sel) {
    _align_tab->selectionChanged(sel);
}

} //namespace Dialog
} //namespace UI
} //namespace Inkscape

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
