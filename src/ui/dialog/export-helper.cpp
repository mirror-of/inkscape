// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "export-helper.h"

#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <gtkmm.h>
#include <png.h>

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "extension/db.h"
#include "file.h"
#include "helper/png-write.h"
#include "inkscape-window.h"
#include "inkscape.h"
#include "io/resource.h"
#include "io/sys.h"
#include "message-stack.h"
#include "object/object-set.h"
#include "object/sp-namedview.h"
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

#ifdef _WIN32

#endif

using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Dialog {

AdvanceOptions::AdvanceOptions()
    : row(0)
{
    this->set_label(_("Advance"));
    Gtk::Grid *grid = Gtk::manage(new Gtk::Grid());
    this->add(*grid);
    {
        interlacing.set_label(_("Use Interlacing"));
        grid->attach(interlacing, 0, row, 2, 1);
        row++;
    }
    {
        bit_depth_list.clear();
        bit_depth_list.insert(bit_depth_list.end(), {{_("Gray 1"), {1, 0}},
                                                     {_("Gray 2"), {2, 0}},
                                                     {_("Gray 4"), {4, 0}},
                                                     {_("Gray 8"), {8, 0}},
                                                     {_("Gray 16"), {16, 0}},
                                                     {_("RGB 8"), {8, 2}},
                                                     {_("RGB 16"), {16, 2}},
                                                     {_("GrayAlpha 8"), {8, 4}},
                                                     {_("GrayAlpha 16"), {16, 4}},
                                                     {_("RGBA 8"), {8, 6}},
                                                     {_("RGBA 16"), {16, 6}}});

        for (auto [label, depth] : bit_depth_list) {
            bit_depth_cb.append(label);
        }

        bit_depth_cb.set_active_text(_("RGBA 8"));
        bit_depth_cb.set_hexpand();
        Gtk::Label *bit_depth_label = Gtk::manage(new Gtk::Label(_("Bit Depth"), Gtk::ALIGN_START));
        grid->attach(*bit_depth_label, 0, row, 1, 1);
        grid->attach(bit_depth_cb, 1, row, 1, 1);
        row++;
    }
    {
        compression_list.clear();
        compression_list.insert(compression_list.end(), {{_("Z No Compression"), 0},
                                                         {_("Z Best Speed"), 1},
                                                         {_("2"), 2},
                                                         {_("3"), 3},
                                                         {_("4"), 4},
                                                         {_("5"), 5},
                                                         {_("Z Default Compression"), 6},
                                                         {_("7"), 7},
                                                         {_("8"), 8},
                                                         {_("Z Best Compression"), 9}});
        for (auto [label, compress] : compression_list) {
            compression_cb.append(label);
        }

        compression_cb.set_active_text(_("Z Default Compression"));
        Gtk::Label *compression_label = Gtk::manage(new Gtk::Label(_("Compression"), Gtk::ALIGN_START));
        grid->attach(*compression_label, 0, row, 1, 1);
        grid->attach(compression_cb, 1, row, 1, 1);
        row++;
    }

    {
        auto pHYs_adj = Gtk::Adjustment::create(0, 0, 100000, 0.1, 1.0, 0);
        pHYs_sb.set_adjustment(pHYs_adj);
        pHYs_sb.set_width_chars(7);
        pHYs_sb.set_digits(2);
        Gtk::Label *phys_dpi_label = Gtk::manage(new Gtk::Label(_("pHYs DPI"), Gtk::ALIGN_START));
        grid->attach(*phys_dpi_label, 0, row, 1, 1);
        grid->attach(pHYs_sb, 1, row, 1, 1);
        row++;
    }
    {
        anti_aliasing_list.clear();
        anti_aliasing_list.insert(anti_aliasing_list.end(), {{_("Cairo Antialias None"), 0},
                                                             {_("Cairo Antialias Fast"), 1},
                                                             {_("Cairo Antialias Good (Default)"), 2},
                                                             {_("Cairo Antialias Best"), 3}});

        for (auto [label, anti_alias] : anti_aliasing_list) {
            anti_aliasing_cb.append(label);
        }

        anti_aliasing_cb.set_active_text(_("Cairo Antialias Good (Default)"));
        Gtk::Label *anti_aliasing_label = Gtk::manage(new Gtk::Label(_("Anti Aliasing"), Gtk::ALIGN_START));
        grid->attach(*anti_aliasing_label, 0, row, 1, 1);
        grid->attach(anti_aliasing_cb, 1, row, 1, 1);
        row++;
    }
    grid->set_row_spacing(4);
    grid->set_column_spacing(5);
}

AdvanceOptions::~AdvanceOptions()
{
    ;
}

bool ExtensionList::list_created{false};
std::map<Glib::ustring, Inkscape::Extension::Output *> ExtensionList::valid_extensions{};
std::map<Glib::ustring, Inkscape::Extension::Output *> ExtensionList::all_extensions{};

void ExtensionList::setup()
{
    this->remove_all();
    createList();

    for (auto [key, omod] : valid_extensions) {
        this->append(key);
    }
    this->set_active_text(".png");
}
void ExtensionList::createList()
{
    if (list_created) {
        return;
    }
    Inkscape::Extension::DB::OutputList extensions;
    Inkscape::Extension::db.get_output_list(extensions);
    Glib::ustring extension;
    for (auto omod : extensions) {
        all_extensions[omod->get_extension()] = omod;

        // FIXME: would be nice to grey them out instead of not listing them
        if (omod->deactivated() || (omod->is_raster() != true))
            continue;

        extension = omod->get_extension();
        valid_extensions[extension] = omod;
    }

    // add extentions manually
    Inkscape::Extension::Output *manual_omod;
    manual_omod = dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG));
    extension = manual_omod->get_extension();
    valid_extensions[extension] = manual_omod;

    list_created = true;
}

ExtensionList::~ExtensionList()
{
    ;
}
void ExtensionList::setExtensionFromFilename(Glib::ustring const &filename)
{
    Glib::ustring extension = get_ext_from_filename(filename);
    if (valid_extensions[extension]) {
        this->set_active_text(extension);
        return;
    }
}
void ExtensionList::appendExtensionToFilename(Glib::ustring &filename)
{
    Glib::ustring filename_extension = get_ext_from_filename(filename);
    Glib::ustring active_extension = this->get_active_text();
    if (active_extension == filename_extension) {
        return;
    }
    if (valid_extensions[filename_extension]) {
        auto extension_point = filename.rfind(filename_extension);
        filename.erase(extension_point);
    }
    filename = filename + active_extension;
    return;
}
void ExtensionList::appendExtensionToFilename(Glib::ustring &filename, Glib::ustring &extension)
{
    createList();
    Glib::ustring filename_extension = get_ext_from_filename(filename);
    Glib::ustring active_extension = extension;
    if (all_extensions[filename_extension]) {
        auto extension_point = filename.rfind(filename_extension);
        filename.erase(extension_point);
    }
    if (valid_extensions[filename_extension]) {
        active_extension = filename_extension;
    }
    // We use ".png" as default extension. Change it to get extension from module.
    if (!valid_extensions[active_extension]) {
        active_extension = ".png";
    }
    filename = filename + active_extension;
    return;
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
    Glib::ustring label = "Add Export";
    add_button->set_label(label);
    this->attach(*add_button, 0, 0, 4, 1);

    this->insert_row(0);

    Gtk::Label *suffix_label = Gtk::manage(new Gtk::Label("Suffix"));
    this->attach(*suffix_label, _suffix_col, 0, 1, 1);
    suffix_label->show();

    Gtk::Label *extension_label = Gtk::manage(new Gtk::Label("Format"));
    this->attach(*extension_label, _extension_col, 0, 1, 1);
    extension_label->show();

    Gtk::Label *dpi_label = Gtk::manage(new Gtk::Label("DPI"));
    this->attach(*dpi_label, _dpi_col, 0, 1, 1);
    dpi_label->show();

    append_row();

    add_button->signal_clicked().connect(sigc::mem_fun(*this, &ExportList::append_row));
    add_button->set_hexpand(true);
    add_button->show();

    this->set_row_spacing(5);
    this->set_column_spacing(2);
}

ExportList::~ExportList()
{
    ;
}

void ExportList::append_row()
{
    int current_row = _num_rows + 1; // because we have label row at top
    this->insert_row(current_row);

    Gtk::Entry *suffix = Gtk::manage(new Gtk::Entry());
    this->attach(*suffix, _suffix_col, current_row, 1, 1);
    suffix->set_width_chars(2);
    suffix->set_hexpand(true);
    suffix->set_placeholder_text("Suffix");
    suffix->show();

    ExtensionList *extension = Gtk::manage(new ExtensionList());
    extension->setup();
    this->attach(*extension, _extension_col, current_row, 1, 1);
    extension->show();

    SpinButton *dpi_sb = Gtk::manage(new SpinButton());
    dpi_sb->set_digits(2);
    dpi_sb->set_increments(0.1, 1.0);
    dpi_sb->set_range(1.0, 100000.0);
    dpi_sb->set_value(default_dpi);
    dpi_sb->set_sensitive(true);
    dpi_sb->set_width_chars(6);
    dpi_sb->set_max_width_chars(6);
    this->attach(*dpi_sb, _dpi_col, current_row, 1, 1);
    dpi_sb->show();

    Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("window-close", Gtk::ICON_SIZE_SMALL_TOOLBAR));
    Gtk::Button *delete_btn = Gtk::manage(new Gtk::Button());
    delete_btn->set_relief(Gtk::RELIEF_NONE);
    delete_btn->set_no_show_all(true);
    if (_num_rows != 0) {
        Gtk::Widget *d_button_0 = dynamic_cast<Gtk::Widget *>(this->get_child_at(_delete_col, 1));
        if (d_button_0) {
            d_button_0->show();
        }
        delete_btn->show();
    }
    pIcon->show();
    delete_btn->add(*pIcon);
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
Glib::ustring ExportList::get_extension(int row)
{
    Glib::ustring extension = "";
    ExtensionList *extension_cb = dynamic_cast<ExtensionList *>(this->get_child_at(_extension_col, row + 1));
    if (extension_cb == nullptr) {
        return extension;
    }
    extension = extension_cb->get_active_text();
    return extension;
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

/*
 ******************************************
 * HELPER FUNCTIONS NOT SPECIF TO CLASSES *
 ******************************************
 */

float getValuePx(float value, Unit const *unit)
{
    return Inkscape::Util::Quantity::convert(value, unit, "px");
}

void setValuePx(Glib::RefPtr<Gtk::Adjustment> &adj, double val, Unit const *unit)
{
    auto value = Inkscape::Util::Quantity::convert(val, "px", unit);
    adj->set_value(value);
    return;
}

// We Create filename by removing already present extension in document name and replacing it with extension passed
// as parameter if exxtension is not valid. If document doesn't have a name we use bitmap as defalt name.
Glib::ustring get_default_filename(Glib::ustring &filename_entry_text, Glib::ustring &extension, SPDocument *doc)
{
    Glib::ustring filename;
    if (doc && doc->getDocumentFilename()) {
        filename = doc->getDocumentFilename();
        ExtensionList::appendExtensionToFilename(filename, extension);
    } else if (doc) {
        filename = create_filepath_from_id(_("bitmap"), filename_entry_text);
        filename = filename + extension;
    }
    return filename;
}

std::string create_filepath_from_id(Glib::ustring id, const Glib::ustring &file_entry_text)
{
    if (id.empty()) { /* This should never happen */
        id = "bitmap";
    }

    std::string directory;

    if (!file_entry_text.empty()) {
        directory = Glib::path_get_dirname(Glib::filename_from_utf8(file_entry_text));
    }

    if (directory.empty()) {
        /* Grab document directory */
        const gchar *docFilename = SP_ACTIVE_DOCUMENT->getDocumentFilename();
        if (docFilename) {
            directory = Glib::path_get_dirname(docFilename);
        }
    }

    if (directory.empty()) {
        directory = Inkscape::IO::Resource::homedir_path(nullptr);
    }

    return Glib::build_filename(directory, Glib::filename_from_utf8(id));
}

Glib::ustring get_ext_from_filename(Glib::ustring const &filename)
{
    Glib::ustring extension = "";
    if (!filename.empty()) {
        auto extension_point = filename.rfind('.');
        if (extension_point != Glib::ustring::npos) {
            extension = filename.substr(extension_point);
        }
    }
    return extension;
}

std::string absolutize_path_from_document_location(SPDocument *doc, const std::string &filename)
{
    std::string path;
    // Make relative paths go from the document location, if possible:
    if (!Glib::path_is_absolute(filename) && doc->getDocumentFilename()) {
        auto dirname = Glib::path_get_dirname(doc->getDocumentFilename());
        if (!dirname.empty()) {
            path = Glib::build_filename(dirname, filename);
        }
    }
    if (path.empty()) {
        path = filename;
    }
    return path;
}

bool _export_raster(Geom::Rect const &area, unsigned long int const &width, unsigned long int const &height,
                    float const &dpi, Glib::ustring const &filename, bool overwrite,
                    unsigned (*callback)(float, void *), ExportProgressDialog *&prog_dialog,
                    Inkscape::Extension::Output *extension, std::vector<SPItem *> *items, AdvanceOptions *adv)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return false;
    SPNamedView *nv = desktop->getNamedView();
    SPDocument *doc = desktop->getDocument();

    if (area.hasZeroArea() || width == 0 || height == 0) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("The chosen area to be exported is invalid."));
        sp_ui_error_dialog(_("The chosen area to be exported is invalid"));
        return false;
    }
    if (filename.empty()) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("You have to enter a filename."));
        sp_ui_error_dialog(_("You have to enter a filename"));
        return false;
    }

    if (!extension || !extension->is_raster()) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Raster Export Error"));
        sp_ui_error_dialog(_("Raster export Method is used for NON RASTER EXTENSION"));
        return false;
    }

    // Advance Parameters default value. We will change them later if adv dialog is provided.
    bool use_interlacing = false; // Maybe use prefs here?
    float pHYs = dpi;             // default is dpi.
    int bit_depth = 8;            // corresponds to RGBA 8
    int color_type = 6;           // corresponds to RGBA 8
    int zlib = 6;                 // Z_DEFAULT_COMPRESSION
    int antialiasing = 2;         // Cairo anti aliasing

    if (adv) {
        use_interlacing = adv->get_interlacing();
        if (adv->get_pHYs() > 0.01) {
            pHYs = adv->get_pHYs();
        }
        bit_depth = adv->get_bit_depth();
        color_type = adv->get_color();
        zlib = adv->get_compression();
        antialiasing = adv->get_anti_aliasing();
    }

    std::string path = absolutize_path_from_document_location(doc, Glib::filename_from_utf8(filename));
    Glib::ustring dirname = Glib::path_get_dirname(path);

    if (dirname.empty() ||
        !Inkscape::IO::file_test(dirname.c_str(), (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
        Glib::ustring safeDir = Inkscape::IO::sanitizeString(dirname.c_str());
        Glib::ustring error =
            g_strdup_printf(_("Directory <b>%s</b> does not exist or is not a directory.\n"), safeDir.c_str());

        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error.c_str());
        sp_ui_error_dialog(error.c_str());
        return false;
    }

    // Do the over-write protection now, since the png is just a temp file.
    if (!overwrite && !sp_ui_overwrite_file(path.c_str())) {
        return false;
    }

    auto fn = Glib::path_get_basename(path);
    auto png_filename = path;
    {
        // Select the extension and set the filename to a temporary file
        int tempfd_out = Glib::file_open_tmp(png_filename, "ink_ext_");
        close(tempfd_out);
    }

    // Export Start Here
    std::vector<SPItem *> selected;
    if (items && items->size() > 0) {
        selected = *items;
    }

    auto bg_color = nv->getPageManager()->background_color;
    ExportResult result = sp_export_png_file(desktop->getDocument(), png_filename.c_str(), area, width, height, pHYs,
                                             pHYs, // previously xdpi, ydpi.
                                             bg_color, callback, (void *)prog_dialog, true, selected,
                                             use_interlacing, color_type, bit_depth, zlib, antialiasing);

    bool failed = result == EXPORT_ERROR || prog_dialog->get_stopped();
    delete prog_dialog;
    prog_dialog = nullptr;
    if (failed) {
        Glib::ustring safeFile = Inkscape::IO::sanitizeString(path.c_str());
        Glib::ustring error = g_strdup_printf(_("Could not export to filename <b>%s</b>.\n"), safeFile.c_str());

        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error.c_str());
        sp_ui_error_dialog(error.c_str());
        return false;
    } else if (result == EXPORT_OK) {
        if (extension->prefs()) {
            try {
                extension->export_raster(doc, png_filename, path.c_str(), false);
            } catch (Inkscape::Extension::Output::save_failed &e) {
                return false;
            }
        } else {
            return false;
        }

    } else {
        // Extensions have their own error popup, so this only tracks failures in the png step
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Export aborted."));
        return false;
    }

    auto recentmanager = Gtk::RecentManager::get_default();
    if (recentmanager && Glib::path_is_absolute(path)) {
        Glib::ustring uri = Glib::filename_to_uri(path);
        recentmanager->add_item(uri);
    }

    Glib::ustring safeFile = Inkscape::IO::sanitizeString(path.c_str());
    desktop->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE, _("Drawing exported to <b>%s</b>."),
                                    safeFile.c_str());

    unlink(png_filename.c_str());
    return true;
}

bool _export_vector(Inkscape::Extension::Output *extension, SPDocument *doc, Glib::ustring const &filename,
                    bool overwrite, std::vector<SPItem *> *items)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return false;

    if (filename.empty()) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("You have to enter a filename."));
        sp_ui_error_dialog(_("You have to enter a filename"));
        return false;
    }

    if (!extension || extension->is_raster()) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Vector Export Error"));
        sp_ui_error_dialog(_("Vector export Method is used for RASTER EXTENSION"));
        return false;
    }

    std::string path = absolutize_path_from_document_location(doc, Glib::filename_from_utf8(filename));
    Glib::ustring dirname = Glib::path_get_dirname(path);

    if (dirname.empty() ||
        !Inkscape::IO::file_test(dirname.c_str(), (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
        Glib::ustring safeDir = Inkscape::IO::sanitizeString(dirname.c_str());
        Glib::ustring error =
            g_strdup_printf(_("Directory <b>%s</b> does not exist or is not a directory.\n"), safeDir.c_str());

        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error.c_str());
        sp_ui_error_dialog(error.c_str());

        return false;
    }

    // Do the over-write protection now
    if (!overwrite && !sp_ui_overwrite_file(path.c_str())) {
        return false;
    }
    doc->ensureUpToDate();
    SPDocument *copy_doc = (doc->copy()).get();
    copy_doc->ensureUpToDate();

    if (items && items->size() > 0) {
        std::vector<SPObject *> objects_to_export;
        std::vector<SPItem *> objects = *items;
        Inkscape::ObjectSet s(copy_doc);
        for (auto &object : objects) {
            SPObject *temp = dynamic_cast<SPObject *>(object);

            if (!temp) {
                Glib::ustring safeFile = Inkscape::IO::sanitizeString(path.c_str());
                Glib::ustring error = g_strdup_printf(_("Could not export to filename <b>%s</b>.\n"), safeFile.c_str());

                desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error.c_str());
                sp_ui_error_dialog(error.c_str());
                return false;
            }
            SPObject *obj = copy_doc->getObjectById(temp->getId());
            if (!obj) {
                Glib::ustring safeFile = Inkscape::IO::sanitizeString(path.c_str());
                Glib::ustring error = g_strdup_printf(_("Could not export to filename <b>%s</b>.\n"), safeFile.c_str());

                desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error.c_str());
                sp_ui_error_dialog(error.c_str());

                return false;
            }
            copy_doc->ensureUpToDate();

            s.add(obj, true);
            objects_to_export.push_back(obj);
        }
        copy_doc->getRoot()->cropToObjects(objects_to_export);
        s.fitCanvas(true, true);
    }

    copy_doc->vacuumDocument();
    try {
        Inkscape::Extension::save(dynamic_cast<Inkscape::Extension::Extension *>(extension), copy_doc, path.c_str(),
                                  false, false, Inkscape::Extension::FILE_SAVE_METHOD_SAVE_COPY);
    } catch (Inkscape::Extension::Output::save_failed &e) {
        Glib::ustring safeFile = Inkscape::IO::sanitizeString(path.c_str());
        Glib::ustring error = g_strdup_printf(_("Could not export to filename <b>%s</b>.\n"), safeFile.c_str());

        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error.c_str());
        sp_ui_error_dialog(error.c_str());

        return false;
    }

    auto recentmanager = Gtk::RecentManager::get_default();
    if (recentmanager && Glib::path_is_absolute(path)) {
        Glib::ustring uri = Glib::filename_to_uri(path);
        recentmanager->add_item(uri);
    }

    Glib::ustring safeFile = Inkscape::IO::sanitizeString(path.c_str());
    desktop->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE, _("Drawing exported to <b>%s</b>."),
                                    safeFile.c_str());
    return true;
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
