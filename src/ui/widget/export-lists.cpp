// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "export-lists.h"

#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <gtkmm.h>
#include <png.h>

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "extension/db.h"
#include "extension/output.h"
#include "file.h"
#include "helper/png-write.h"
#include "inkscape-window.h"
#include "inkscape.h"
#include "io/resource.h"
#include "io/sys.h"
#include "message-stack.h"
#include "object/object-set.h"
#include "object/sp-namedview.h"
#include "object/sp-page.h"
#include "object/sp-root.h"
#include "page-manager.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "ui/dialog-events.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog/filedialog.h"
#include "ui/icon-loader.h"
#include "ui/interface.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"

using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Dialog {

ExtensionList::ExtensionList()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _watch_pref = prefs->createObserver("/dialogs/export/show_all_extensions", [=]() { setup(); });
}

ExtensionList::ExtensionList(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
    : Inkscape::UI::Widget::ScrollProtected<Gtk::ComboBoxText>(cobject, refGlade)
{
    // This duplication is silly, but needed because C++ can't
    // both deligate the constructor, and construct for glade
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _watch_pref = prefs->createObserver("/dialogs/export/show_all_extensions", [=]() { setup(); });
}

void ExtensionList::setup()
{
    this->remove_all();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool export_all = prefs->getBool("/dialogs/export/show_all_extensions", false);

    Inkscape::Extension::DB::OutputList extensions;
    Inkscape::Extension::db.get_output_list(extensions);
    for (auto omod : extensions) {
        auto oid = Glib::ustring(omod->get_id());
        if (!export_all && !omod->is_raster() && !omod->is_exported())
            continue;
        // Comboboxes don't have a disabled row property
        if (omod->deactivated())
            continue;
        this->append(oid, omod->get_filetypename());
        // Record extensions map for filename-to-combo selections
        auto ext = omod->get_extension();
        if (!ext_to_mod[ext]) {
            // Some extensions have multiple of the same extension (for example PNG)
            // we're going to pick the first in the found list to back-link to.
            ext_to_mod[ext] = omod;
        }
    }
    this->set_active_id(SP_MODULE_KEY_RASTER_PNG);
}

/**
 * Returns the Output extension currently selected in this dropdown.
 */
Inkscape::Extension::Output *ExtensionList::getExtension()
{
    return dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(this->get_active_id().c_str()));
}

/**
 * Returns the file extension (file ending) of the currently selected extension.
 */
Glib::ustring ExtensionList::getFileExtension()
{
    if (auto ext = getExtension()) {
        return ext->get_extension();
    }
    return "";
}

/**
 * Removes the file extension, *if* it's one of the extensions in the list.
 */
void ExtensionList::removeExtension(Glib::ustring &filename)
{
    auto ext = Inkscape::IO::get_file_extension(filename);
    if (ext_to_mod[ext]) {
        filename.erase(filename.size()-ext.size());
    }
}

void ExtensionList::setExtensionFromFilename(Glib::ustring const &filename)
{
    auto ext = Inkscape::IO::get_file_extension(filename);
    if (auto omod = ext_to_mod[ext]) {
        this->set_active_id(omod->get_id());
    }
}

void ExportList::setup()
{
    if (_initialised) {
        return;
    }
    _initialised = true;
    prefs = Inkscape::Preferences::get();
    default_dpi = prefs->getDouble("/dialogs/export/defaultxdpi/value", DPI_BASE);

    Gtk::Button *add_button = Gtk::manage(new Gtk::Button());
    Glib::ustring label = _("Add Export");
    add_button->set_label(label);
    this->attach(*add_button, 0, 0, 4, 1);

    this->insert_row(0);

    Gtk::Label *suffix_label = Gtk::manage(new Gtk::Label(_("Suffix")));
    this->attach(*suffix_label, _suffix_col, 0, 1, 1);
    suffix_label->show();

    Gtk::Label *extension_label = Gtk::manage(new Gtk::Label(_("Format")));
    this->attach(*extension_label, _extension_col, 0, 1, 1);
    extension_label->show();

    Gtk::Label *dpi_label = Gtk::manage(new Gtk::Label(_("DPI")));
    this->attach(*dpi_label, _dpi_col, 0, 1, 1);
    dpi_label->show();

    append_row();

    add_button->signal_clicked().connect(sigc::mem_fun(*this, &ExportList::append_row));
    add_button->set_hexpand(true);
    add_button->show();

    this->set_row_spacing(5);
    this->set_column_spacing(2);
}

void ExportList::removeExtension(Glib::ustring &filename)
{
    ExtensionList *extension_cb = dynamic_cast<ExtensionList *>(this->get_child_at(_extension_col, 1));
    if (extension_cb) {
        extension_cb->removeExtension(filename);
        return;
    }
}

void ExportList::append_row()
{
    int current_row = _num_rows + 1; // because we have label row at top
    this->insert_row(current_row);

    Gtk::Entry *suffix = Gtk::manage(new Gtk::Entry());
    this->attach(*suffix, _suffix_col, current_row, 1, 1);
    suffix->set_width_chars(2);
    suffix->set_hexpand(true);
    suffix->set_placeholder_text(_("Suffix"));
    suffix->show();

    ExtensionList *extension = Gtk::manage(new ExtensionList());
    SpinButton *dpi_sb = Gtk::manage(new SpinButton());

    extension->setup();
    extension->show();
    this->attach(*extension, _extension_col, current_row, 1, 1);

    // Disable DPI when not using a raster image output
    extension->signal_changed().connect([=]() {
        if (auto ext = extension->getExtension()) {
            dpi_sb->set_sensitive(ext->is_raster());
        }
    });

    dpi_sb->set_digits(2);
    dpi_sb->set_increments(0.1, 1.0);
    dpi_sb->set_range(1.0, 100000.0);
    dpi_sb->set_value(default_dpi);
    dpi_sb->set_sensitive(true);
    dpi_sb->set_width_chars(6);
    dpi_sb->set_max_width_chars(6);
    dpi_sb->show();
    this->attach(*dpi_sb, _dpi_col, current_row, 1, 1);

    Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("window-close", Gtk::ICON_SIZE_SMALL_TOOLBAR));
    Gtk::Button *delete_btn = Gtk::manage(new Gtk::Button());
    delete_btn->set_relief(Gtk::RELIEF_NONE);
    delete_btn->add(*pIcon);
    delete_btn->show_all();
    delete_btn->set_no_show_all(true);
    this->attach(*delete_btn, _delete_col, current_row, 1, 1);
    delete_btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ExportList::delete_row), delete_btn));

    _num_rows++;
}

void ExportList::delete_row(Gtk::Widget *widget)
{
    if (widget == nullptr) {
        return;
    }
    if (_num_rows <= 1) {
        return;
    }
    int row = this->child_property_top_attach(*widget);
    this->remove_row(row);
    _num_rows--;
    if (_num_rows <= 1) {
        Gtk::Widget *d_button_0 = dynamic_cast<Gtk::Widget *>(this->get_child_at(_delete_col, 1));
        if (d_button_0) {
            d_button_0->hide();
        }
    }
}

Glib::ustring ExportList::get_suffix(int row)
{
    Glib::ustring suffix = "";
    Gtk::Entry *entry = dynamic_cast<Gtk::Entry *>(this->get_child_at(_suffix_col, row + 1));
    if (entry == nullptr) {
        return suffix;
    }
    suffix = entry->get_text();
    return suffix;
}
Inkscape::Extension::Output *ExportList::getExtension(int row)
{
    ExtensionList *extension_cb = dynamic_cast<ExtensionList *>(this->get_child_at(_extension_col, row + 1));
    return extension_cb ? extension_cb->getExtension() : nullptr;
}
double ExportList::get_dpi(int row)
{
    double dpi = default_dpi;
    SpinButton *spin_sb = dynamic_cast<SpinButton *>(this->get_child_at(_dpi_col, row + 1));
    if (spin_sb == nullptr) {
        return dpi;
    }
    dpi = spin_sb->get_value();
    return dpi;
}



} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
