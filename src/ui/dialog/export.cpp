// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Peter Bostrom
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 1999-2007, 2012, 2021 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

// This has to be included prior to anything that includes setjmp.h, it croaks otherwise
#include "export.h"

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
#include "object/sp-root.h"
#include "object/sp-page.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "ui/dialog-events.h"
#include "ui/dialog/export-single.h"
#include "ui/dialog/export-batch.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog/filedialog.h"
#include "ui/interface.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"

#ifdef _WIN32

#endif

using Inkscape::Util::unit_table;
namespace Inkscape {
namespace UI {
namespace Dialog {

Export::Export()
    : DialogBase("/dialogs/export/", "Export")
{
    std::string gladefile = get_filename_string(Inkscape::IO::Resource::UIS, "dialog-export.glade");

    try {
        builder = Gtk::Builder::create_from_file(gladefile);
    } catch (const Glib::Error &ex) {
        g_error("Glade file loading failed for export screen");
        return;
    }

    prefs = Inkscape::Preferences::get();

    builder->get_widget("Export Dialog Box", container);
    add(*container);
    show_all_children();

    builder->get_widget("Export Notebook", export_notebook);

    // Initialise Single Export and its objects
    builder->get_widget_derived("Single Image", single_image);
    single_image->initialise(builder);

    // Initialise Batch Export and its objects
    builder->get_widget_derived("Batch Export", batch_export);
    batch_export->initialise(builder);

    container->signal_realize().connect([=]() {
        single_image->setup();
        batch_export->setup();
        setDefaultNotebookPage();
        notebook_signal = export_notebook->signal_switch_page().connect(sigc::mem_fun(*this, &Export::onNotebookPageSwitch));
    });
    container->signal_unrealize().connect([=]() {
        notebook_signal.disconnect();
    });
}

Export::~Export()
{
    single_image->setDocument(nullptr);
    single_image->setDesktop(nullptr);
    batch_export->setDocument(nullptr);
    batch_export->setDesktop(nullptr);
}

// Set current page based on preference/last visited page
void Export::setDefaultNotebookPage()
{
    pages[BATCH_EXPORT] = export_notebook->page_num(*batch_export);
    pages[SINGLE_IMAGE] = export_notebook->page_num(*single_image);
    export_notebook->set_current_page(pages[SINGLE_IMAGE]);
}

void Export::documentReplaced()
{
    single_image->setDocument(getDocument());
    batch_export->setDocument(getDocument());
}

void Export::desktopReplaced()
{
    single_image->setDesktop(getDesktop());
    single_image->setApp(getApp());
    batch_export->setDesktop(getDesktop());
    batch_export->setApp(getApp());
    // Called previously, but we need post-desktop call too
    documentReplaced();
}

void Export::selectionChanged(Inkscape::Selection *selection)
{
    auto current_page = export_notebook->get_current_page();
    if (current_page == pages[SINGLE_IMAGE]) {
        single_image->selectionChanged(selection);
    }
    if (current_page == pages[BATCH_EXPORT]) {
        batch_export->selectionChanged(selection);
    }
}
void Export::selectionModified(Inkscape::Selection *selection, guint flags)
{
    auto current_page = export_notebook->get_current_page();
    if (current_page == pages[SINGLE_IMAGE]) {
        single_image->selectionModified(selection, flags);
    }
    if (current_page == pages[BATCH_EXPORT]) {
        batch_export->selectionModified(selection, flags);
    }
}

void Export::onNotebookPageSwitch(Widget *page, guint page_number)
{
    auto desktop = getDesktop();
    if (desktop) {
        auto selection = desktop->getSelection();

        if (page_number == pages[SINGLE_IMAGE]) {
            single_image->selectionChanged(selection);
        }
        if (page_number == pages[BATCH_EXPORT]) {
            batch_export->selectionChanged(selection);
        }
    }
}

std::string Export::absolutizePath(SPDocument *doc, const std::string &filename)
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

bool Export::unConflictFilename(SPDocument *doc, Glib::ustring &filename, Glib::ustring const extension)
{
    std::string path = absolutizePath(doc, Glib::filename_from_utf8(filename));
    Glib::ustring test_filename = path + extension;
    if (!Inkscape::IO::file_test(test_filename.c_str(), G_FILE_TEST_EXISTS)) {
        filename = test_filename;
        return true;
    }
    for (int i = 1; i <= 100; i++) {
        test_filename = path + "_copy_" + std::to_string(i) + extension;
        if (!Inkscape::IO::file_test(test_filename.c_str(), G_FILE_TEST_EXISTS)) {
            filename = test_filename;
            return true;
        }
    }
    return false;
}

bool Export::exportRaster(
        Geom::Rect const &area, unsigned long int const &width, unsigned long int const &height,
        float const &dpi, Glib::ustring const &filename, bool overwrite,
        unsigned (*callback)(float, void *), ExportProgressDialog *&prog_dialog,
        Inkscape::Extension::Output *extension, std::vector<SPItem *> *items, int run)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return false;
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

    // Get the preferences by popping up the extension prefs
    if (run == 0 && !extension->prefs()) {
        return false; // cancel button
    }

    float pHYs = extension->get_param_float("png_phys", dpi);
    if (pHYs < 0.01) pHYs = dpi;

    bool use_interlacing = extension->get_param_bool("png_interlacing", false);
    int antialiasing = extension->get_param_int("png_antialias", 2); // Cairo anti aliasing
    int zlib = extension->get_param_int("png_compression", 1); // Default is 6 for png, but 1 for non-png
    auto val = extension->get_param_int("png_bitdepth", 99); // corresponds to RGBA 8

    int bit_depth = pow(2, (val & 0x0F));
    int color_type = (val & 0xF0) >> 4;

    std::string path = absolutizePath(doc, Glib::filename_from_utf8(filename));
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

    auto bg_color = doc->getPageManager().background_color;
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
        // Don't ask for preferences on every run.
        try {
            extension->export_raster(doc, png_filename, path.c_str(), false);
        } catch (Inkscape::Extension::Output::save_failed &e) {
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

bool Export::exportVector(
        Inkscape::Extension::Output *extension, SPDocument *doc,
        Glib::ustring const &filename,
        bool overwrite, std::vector<SPItem *> *items, SPPage *page)
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

    std::string path = absolutizePath(doc, Glib::filename_from_utf8(filename));
    Glib::ustring dirname = Glib::path_get_dirname(path);
    Glib::ustring safeFile = Inkscape::IO::sanitizeString(path.c_str());
    Glib::ustring safeDir = Inkscape::IO::sanitizeString(dirname.c_str());

    if (dirname.empty() ||
        !Inkscape::IO::file_test(dirname.c_str(), (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
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
    auto copy_doc = doc->copy();
    copy_doc->ensureUpToDate();

    std::vector<SPItem *> objects = *items;
    std::set<std::string> page_ids;
    if (page) {
        // If page then our item set is limited to the overlapping items
        auto page_items = page->getOverlappingItems();

        if (items->size() == 0) {
            // Items is page_items, remove all items not in this page.
            objects = page_items;
        } else {
            for (auto &item : page_items) {
                item->getIds(page_ids);
            }
        }
    }

    // Save the page rect, must be done before disabledPages in case page is from copy doc.
    Geom::OptRect page_rect = page ? page->getDesktopRect() : Geom::OptRect();

    // We never export multiple pages here, must be done before fitToRect and fitCanvas
    copy_doc->getPageManager().disablePages();

    // Page export ALWAYS restricts, even if nothing would be on the page.
    if (objects.size() > 0 || page) {
        std::vector<SPObject *> objects_to_export;
        Inkscape::ObjectSet object_set(copy_doc.get());
        for (auto &object : objects) {
            auto _id = object->getId();
            if (!_id || (!page_ids.empty() && page_ids.find(_id) == page_ids.end())) {
                // This item is off the page so can be ignored for export
                continue;
            }

            SPObject *obj = copy_doc->getObjectById(_id);
            if (!obj) {
                Glib::ustring error = g_strdup_printf(_("Could not export to filename <b>%s</b>. (missing object)\n"), safeFile.c_str());

                desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error.c_str());
                sp_ui_error_dialog(error.c_str());

                return false;
            }
            copy_doc->ensureUpToDate();

            object_set.add(obj, true);
            objects_to_export.push_back(obj);
        }

        copy_doc->getRoot()->cropToObjects(objects_to_export);

        if (page) {
            // Resize to page here.
            copy_doc->fitToRect(*page_rect, true);
        } else {
            object_set.fitCanvas(true, true);
        }
    }

    // Remove all unused definitions
    copy_doc->vacuumDocument();

    try {
        Inkscape::Extension::save(dynamic_cast<Inkscape::Extension::Extension *>(extension), copy_doc.get(), path.c_str(),
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

    desktop->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE, _("Drawing exported to <b>%s</b>."),
                                    safeFile.c_str());
    return true;
}

std::string Export::filePathFromObject(SPDocument *doc, SPObject *obj, const Glib::ustring &file_entry_text)
{
    Glib::ustring id = _("bitmap");
    if (obj && obj->getId()) {
        id = obj->getId();
    }
    return filePathFromId(doc, id, file_entry_text);
}

std::string Export::filePathFromId(SPDocument *doc, Glib::ustring id, const Glib::ustring &file_entry_text)
{
    g_assert(!id.empty());

    std::string directory;

    if (!file_entry_text.empty()) {
        directory = Glib::path_get_dirname(Glib::filename_from_utf8(file_entry_text));
    }

    if (directory.empty()) {
        /* Grab document directory */
        const gchar *docFilename = doc->getDocumentFilename();
        if (docFilename) {
            directory = Glib::path_get_dirname(docFilename);
        }
    }

    if (directory.empty()) {
        directory = Inkscape::IO::Resource::homedir_path(nullptr);
    }

    return Glib::build_filename(directory, Glib::filename_from_utf8(id));
}

Glib::ustring Export::defaultFilename(SPDocument *doc, Glib::ustring &filename_entry_text, Glib::ustring extension)
{
    Glib::ustring filename;
    if (doc && doc->getDocumentFilename()) {
        filename = doc->getDocumentFilename();
        //appendExtensionToFilename(filename, extension);
    } else if (doc) {
        filename = filePathFromId(doc, _("bitmap"), filename_entry_text);
        filename = filename + extension;
    }
    return filename;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
