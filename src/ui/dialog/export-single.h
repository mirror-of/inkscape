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

#ifndef SP_EXPORT_SINGLE_H
#define SP_EXPORT_SINGLE_H

#include "ui/widget/scrollprotected.h"

class InkscapeApplication;
class SPDesktop;
class SPDocument;
class SPObject;

namespace Inkscape {
    class Selection;
    class Preferences;

namespace Util {
    class Unit;
}
namespace UI {
    namespace Widget {
        class UnitMenu;
    }
namespace Dialog {
    class ExportPreview;
    class ExtensionList;
    class ExportProgressDialog;

class SingleExport : public Gtk::Box
{
public:
    SingleExport(){};
    SingleExport(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
        : Gtk::Box(cobject){};
    ~SingleExport() override{};

private:
    InkscapeApplication *_app = nullptr;
    SPDesktop *_desktop = nullptr;
    SPDocument *_document = nullptr;

private:
    bool setupDone = false; // To prevent setup() call add connections again.

public:
    void setApp(InkscapeApplication *app) { _app = app; }
    void setDocument(SPDocument *document);
    void setDesktop(SPDesktop *desktop);
    void selectionChanged(Inkscape::Selection *selection);
    void selectionModified(Inkscape::Selection *selection, guint flags);

private:
    enum sb_type
    {
        SPIN_X0 = 0,
        SPIN_X1,
        SPIN_Y0,
        SPIN_Y1,
        SPIN_WIDTH,
        SPIN_HEIGHT,
        SPIN_BMWIDTH,
        SPIN_BMHEIGHT,
        SPIN_DPI
    };

    enum selection_mode
    {
        SELECTION_PAGE = 0, // Default is alaways placed first
        SELECTION_SELECTION,
        SELECTION_DRAWING,
        SELECTION_CUSTOM,
    };

private:
    typedef Inkscape::UI::Widget::ScrollProtected<Gtk::SpinButton> SpinButton;

    std::map<sb_type, SpinButton *> spin_buttons;
    std::map<sb_type, Gtk::Label *> spin_labels;
    std::map<selection_mode, Gtk::RadioButton *> selection_buttons;

    Gtk::Box *si_units_row = nullptr;
    Gtk::CheckButton *show_export_area = nullptr;
    Inkscape::UI::Widget::UnitMenu *units = nullptr;
    Gtk::Label *si_name_label = nullptr;

    Gtk::CheckButton *si_hide_all = nullptr;

    Gtk::CheckButton *si_show_preview = nullptr;
    ExportPreview *preview = nullptr;

    ExtensionList *si_extension_cb = nullptr;
    Gtk::Entry *si_filename_entry = nullptr;
    Gtk::Button *si_export = nullptr;
    Gtk::Box *adv_box = nullptr;
    Gtk::ProgressBar *_prog = nullptr;
    Gtk::Button *page_prev = nullptr;
    Gtk::Button *page_next = nullptr;

    bool filename_modified;
    Glib::ustring original_name;
    Glib::ustring doc_export_name;

    Inkscape::Preferences *prefs = nullptr;
    std::map<selection_mode, Glib::ustring> selection_names;
    selection_mode current_key = (selection_mode)0;

public:
    // initialise variables from builder
    void initialise(const Glib::RefPtr<Gtk::Builder> &builder);
    void setup();

private:
    void setupUnits();
    void setupExtensionList();
    void setupSpinButtons();
    void toggleSpinButtonVisibility();
    void refreshPreview();

    // change range and callbacks to spinbuttons
    template <typename T>
    void setupSpinButton(Gtk::SpinButton *sb, double val, double min, double max, double step, double page, int digits,
                         bool sensitive, void (SingleExport::*cb)(T), T param);

    void setDefaultSelectionMode();
    void onAreaXChange(sb_type type);
    void onAreaYChange(sb_type type);
    void onDpiChange(sb_type type);
    void onAreaTypeToggle(selection_mode key);
    void onUnitChanged();
    void onFilenameModified();
    void onExtensionChanged();
    void onExport();
    void onBrowse(Gtk::EntryIconPosition pos, const GdkEventButton *ev);
    void on_inkscape_selection_modified(Inkscape::Selection *selection, guint flags);
    void on_inkscape_selection_changed(Inkscape::Selection *selection);

public:
    void refresh()
    {
        refreshArea();
        refreshPage();
        loadExportHints();
    };

private:
    void refreshArea();
    void refreshPage();
    void loadExportHints();
    void saveExportHints(SPObject *target);
    void areaXChange(sb_type type);
    void areaYChange(sb_type type);
    void dpiChange(sb_type type);
    void setArea(double x0, double y0, double x1, double y1);
    void blockSpinConns(bool status);

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
    // Gtk Signals
    std::vector<sigc::connection> spinButtonConns;
    sigc::connection filenameConn;
    sigc::connection extensionConn;
    sigc::connection exportConn;
    sigc::connection browseConn;
    // Document Signals
    sigc::connection _page_selected_connection;
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
