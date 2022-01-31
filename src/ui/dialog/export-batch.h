// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 1999-2007, 2021 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SP_EXPORT_BATCH_H
#define SP_EXPORT_BATCH_H

#include <gtkmm.h>

#include "ui/widget/scrollprotected.h"

class InkscapeApplication;
class SPDocument;
class SPDesktop;

namespace Inkscape {
    class PageManager;
    class Preferences;
    class Selection;

namespace UI {
namespace Dialog {

class ExportList;
class BatchItem;
class ExportProgressDialog;

class BatchExport : public Gtk::Box
{
public:
    BatchExport() {};
    BatchExport(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
        : Gtk::Box(cobject){};
    ~BatchExport() override = default;

private:
    InkscapeApplication *_app;
    SPDesktop *_desktop = nullptr;
    SPDocument *_document = nullptr;
    PageManager *_page_manager = nullptr;

private:
    bool setupDone = false; // To prevent setup() call add connections again.

public:
    void setApp(InkscapeApplication *app) { _app = app; }
    void setDocument(SPDocument *document);
    void setDesktop(SPDesktop *desktop) { _desktop = desktop; }
    void selectionChanged(Inkscape::Selection *selection);
    void selectionModified(Inkscape::Selection *selection, guint flags);
    void pagesChanged();

private:
    enum selection_mode
    {
        SELECTION_LAYER = 0, // Default is alaways placed first
        SELECTION_SELECTION,
        SELECTION_PAGE,
    };

private:
    typedef Inkscape::UI::Widget::ScrollProtected<Gtk::SpinButton> SpinButton;

    std::map<selection_mode, Gtk::RadioButton *> selection_buttons;
    Gtk::FlowBox *preview_container = nullptr;
    Gtk::CheckButton *show_preview = nullptr;
    Gtk::Label *num_elements = nullptr;
    Gtk::CheckButton *hide_all = nullptr;
    Gtk::Entry *filename_entry = nullptr;
    Gtk::Button *export_btn = nullptr;
    Gtk::ProgressBar *_prog = nullptr;
    ExportList *export_list = nullptr;

    // Store all items to be displayed in flowbox
    std::map<std::string, BatchItem *> current_items;

    bool filename_modified;
    Glib::ustring original_name;
    Glib::ustring doc_export_name;

    Inkscape::Preferences *prefs = nullptr;
    std::map<selection_mode, Glib::ustring> selection_names;
    selection_mode current_key;

public:
    // initialise variables from builder
    void initialise(const Glib::RefPtr<Gtk::Builder> &builder);
    void setup();

private:
    void setDefaultSelectionMode();
    void onFilenameModified();
    void onAreaTypeToggle(selection_mode key);
    void onExport();
    void onBrowse(Gtk::EntryIconPosition pos, const GdkEventButton *ev);

    void refreshPreview();
    void refreshItems();
    void loadExportHints();

public:
    void refresh()
    {
        refreshItems();
        loadExportHints();
    };

private:
    void setExporting(bool exporting, Glib::ustring const &text = "");
    ExportProgressDialog *create_progress_dialog(Glib::ustring progress_text);
    /**
     * Callback to be used in for loop to update the progress bar.
     *
     * @param value number between 0 and 1 indicating the fraction of progress (0.17 = 17 % progress)
     * @param dlg void pointer to the Gtk::Dialog progress dialog
     */
    static unsigned int onProgressCallback(float value, void *dlg);

    /**
     * Callback for pressing the cancel button.
     */
    void onProgressCancel();

    /**
     * Callback invoked on closing the progress dialog.
     */
    bool onProgressDelete(GdkEventAny *event);

private:
    ExportProgressDialog *prog_dlg = nullptr;
    bool interrupted;

    // Gtk Signals
    sigc::connection filenameConn;
    sigc::connection exportConn;
    sigc::connection browseConn;
    sigc::connection selectionModifiedConn;
    sigc::connection selectionChangedConn;
    // SVG Signals
    sigc::connection _pages_changed_connection;
};
} // namespace Dialog
} // namespace UI
} // namespace Inkscape
#endif

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
