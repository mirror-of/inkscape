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

#ifndef SP_EXPORT_H
#define SP_EXPORT_H

#include <gtkmm.h>

#include "ui/dialog/dialog-base.h"
#include "ui/widget/scrollprotected.h"

class SPPage;

namespace Inkscape {
    class Preferences;
    namespace Util {
        class Unit;
    }
    namespace Extension {
        class Output;
    }

namespace UI {
namespace Dialog {
    class SingleExport;
    class BatchExport;

enum notebook_page
{
    SINGLE_IMAGE = 0,
    BATCH_EXPORT
};

class ExportProgressDialog : public Gtk::Dialog
{
private:
    Gtk::ProgressBar *_progress = nullptr;
    Gtk::Widget *_export_panel = nullptr;
    int _current = 0;
    int _total = 0; 
    bool _stopped = false;
  
public:
    ExportProgressDialog(const Glib::ustring &title, bool modal = false)
        : Gtk::Dialog(title, modal)
    {}
      
    inline void set_export_panel(const decltype(_export_panel) export_panel) { _export_panel = export_panel; }
    inline decltype(_export_panel) get_export_panel() const { return _export_panel; }
      
    inline void set_progress(const decltype(_progress) progress) { _progress = progress; }
    inline decltype(_progress) get_progress() const { return _progress; }
      
    inline void set_current(const int current) { _current = current; }
    inline int get_current() const { return _current; }
      
    inline void set_total(const int total) { _total = total; }
    inline int get_total() const { return _total; }
      
    inline bool get_stopped() const { return _stopped; }
    inline void set_stopped() { _stopped = true; }
};

class Export : public DialogBase
{
public:
    Export();
    ~Export() override = default;

    static Export &getInstance() { return *new Export(); }

private:
    Glib::RefPtr<Gtk::Builder> builder;
    Gtk::Box *container = nullptr;            // Main Container
    Gtk::Notebook *export_notebook = nullptr; // Notebook Container for single and batch export

    SingleExport *single_image = nullptr;
    BatchExport *batch_export = nullptr;

    Inkscape::Preferences *prefs = nullptr;

    // setup default values of widgets
    void setDefaultNotebookPage();
    std::map<notebook_page, int> pages;

    // signals callback
    void onRealize();
    void onNotebookPageSwitch(Widget *page, guint page_number);
    void documentReplaced() override;
    void desktopReplaced() override;
    void selectionChanged(Inkscape::Selection *selection) override;
    void selectionModified(Inkscape::Selection *selection, guint flags) override;

public:
    static std::string absolutizePath(SPDocument *doc, const std::string &filename);
    static bool unConflictFilename(SPDocument *doc, Glib::ustring &filename, Glib::ustring const extension);
    static std::string filePathFromObject(SPDocument *doc, SPObject *obj, const Glib::ustring &file_entry_text);
    static std::string filePathFromId(SPDocument *doc, Glib::ustring id, const Glib::ustring &file_entry_text);
    static Glib::ustring defaultFilename(SPDocument *doc, Glib::ustring &filename_entry_text, Glib::ustring extension);

    static bool exportRaster(
        Geom::Rect const &area, unsigned long int const &width, unsigned long int const &height,
        float const &dpi, Glib::ustring const &filename, bool overwrite,
        unsigned (*callback)(float, void *), ExportProgressDialog *&prog_dialog,
        Inkscape::Extension::Output *extension, std::vector<SPItem *> *items = nullptr, int run = 0);
  
    static bool exportVector(
        Inkscape::Extension::Output *extension, SPDocument *doc, Glib::ustring const &filename,
        bool overwrite, std::vector<SPItem *> *items = nullptr, SPPage *page = nullptr);

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
