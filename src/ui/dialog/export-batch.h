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

#include "export-helper.h"
#include "extension/output.h"
#include "ui/widget/scroll-utils.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class BatchItem;

class BatchExport : public Gtk::Box
{
public:
    BatchExport(){};
    BatchExport(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
        : Gtk::Box(cobject){};
    ~BatchExport() override;

private:
    InkscapeApplication *_app;
    SPDesktop *_desktop = nullptr;

private:
    bool setupDone = false; // To prevent setup() call add connections again.

public:
    void setApp(InkscapeApplication *app) { _app = app; }
    void setDocument(SPDocument *document);
    void setDesktop(SPDesktop *desktop) { _desktop = desktop; }
    void selectionChanged(Inkscape::Selection *selection);
    void selectionModified(Inkscape::Selection *selection, guint flags);

private:
    enum selection_mode
    {
        SELECTION_LAYER = 0, // Default is alaways placed first
        SELECTION_SELECTION,
    };

private:
    typedef Inkscape::UI::Widget::ScrollProtected<Gtk::SpinButton> SpinButton;

    std::map<selection_mode, Gtk::RadioButton *> selection_buttons;
    Gtk::FlowBox *preview_container = nullptr;
    Gtk::CheckButton *show_preview = nullptr;
    Gtk::Label *num_elements = nullptr;
    Gtk::Box *adv_box = nullptr;
    Gtk::CheckButton *hide_all = nullptr;
    Gtk::Entry *filename_entry = nullptr;
    Gtk::Button *export_btn = nullptr;
    Gtk::ProgressBar *_prog = nullptr;
    ExportList *export_list = nullptr;

    AdvanceOptions advance_options;

private:
    // Store all items to be displayed in flowbox
    std::map<std::string, BatchItem *> current_items;

private:
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
    bool getNonConflictingFilename(Glib::ustring &filename, Glib::ustring const extension);

private:
    void setDefaultSelectionMode();
    void setDefaultFilename();

private:
    void onFilenameModified();
    void onAreaTypeToggle(selection_mode key);
    void onExport();
    void onBrowse(Gtk::EntryIconPosition pos, const GdkEventButton *ev);

public:
    void refresh()
    {
        refreshItems();
        refreshExportHints();
    };

private:
    void refreshPreview();
    void refreshItems();
    void refreshExportHints();

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

private:
    sigc::connection filenameConn;
    sigc::connection exportConn;
    sigc::connection browseConn;
    sigc::connection selectionModifiedConn;
    sigc::connection selectionChangedConn;
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