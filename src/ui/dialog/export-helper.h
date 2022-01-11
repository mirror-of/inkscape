// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SP_EXPORT_HELPER_H
#define SP_EXPORT_HELPER_H

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
#include "preferences.h"
#include "selection-chemistry.h"
#include "ui/dialog-events.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog/filedialog.h"
#include "ui/interface.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"

using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Dialog {

#define EXPORT_COORD_PRECISION 3
#define SP_EXPORT_MIN_SIZE 1.0
#define DPI_BASE Inkscape::Util::Quantity::convert(1, "in", "px")

// Class for storing and manipulating advance options.
class AdvanceOptions : public Gtk::Expander
{
public:
    AdvanceOptions();
    ~AdvanceOptions() override;

private:
    Gtk::CheckButton interlacing;

    std::vector<std::pair<Glib::ustring, std::pair<int, int>>> bit_depth_list;
    Inkscape::UI::Widget::ScrollProtected<Gtk::ComboBoxText> bit_depth_cb;

    std::vector<std::pair<Glib::ustring, int>> compression_list;
    Inkscape::UI::Widget::ScrollProtected<Gtk::ComboBoxText> compression_cb;

    Inkscape::UI::Widget::ScrollProtected<Gtk::SpinButton> pHYs_sb;
    Gtk::SpinButton a;

    std::vector<std::pair<Glib::ustring, int>> anti_aliasing_list;
    Inkscape::UI::Widget::ScrollProtected<Gtk::ComboBoxText> anti_aliasing_cb;

private:
    int row;

public:
    int get_color() { return bit_depth_list[bit_depth_cb.get_active_row_number()].second.second; }
    int get_bit_depth() { return bit_depth_list[bit_depth_cb.get_active_row_number()].second.first; }
    int get_compression() { return compression_list[compression_cb.get_active_row_number()].second; }
    int get_anti_aliasing() { return anti_aliasing_list[anti_aliasing_cb.get_active_row_number()].second; }
    bool get_interlacing() { return interlacing.get_active(); }
    double get_pHYs() { return pHYs_sb.get_value(); }
};

// Class for storing and manipulating extensions
class ExtensionList : public Inkscape::UI::Widget::ScrollProtected<Gtk::ComboBoxText>
{
public:
    ExtensionList(){};
    ExtensionList(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
        : Inkscape::UI::Widget::ScrollProtected<Gtk::ComboBoxText>(cobject, refGlade){};
    ~ExtensionList() override;

public:
    void setup();
    void setExtensionFromFilename(Glib::ustring const &filename);
    void appendExtensionToFilename(Glib::ustring &filename);
    static void createList();
    static bool list_created;
    static void appendExtensionToFilename(Glib::ustring &filename, Glib::ustring &extension);

public:
    static std::map<Glib::ustring, Inkscape::Extension::Output *> valid_extensions;
    static std::map<Glib::ustring, Inkscape::Extension::Output *> all_extensions;
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

class ExportList : public Gtk::Grid
{
public:
    ExportList(){};
    ExportList(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
        : Gtk::Grid(cobject){};
    ~ExportList() override;

public:
    void setup();
    void append_row();
    void delete_row(Gtk::Widget *widget);
    Glib::ustring get_suffix(int row);
    Glib::ustring get_extension(int row);
    double get_dpi(int row);
    int get_rows() { return _num_rows; }

private:
    typedef Inkscape::UI::Widget::ScrollProtected<Gtk::SpinButton> SpinButton;
    Inkscape::Preferences *prefs = nullptr;
    double default_dpi = 96.00;

private:
    bool _initialised = false;
    int _num_rows = 0;
    int _suffix_col = 0;
    int _extension_col = 1;
    int _dpi_col = 2;
    int _delete_col = 3;
};

float getValuePx(float value, Unit const *unit);
void setValuePx(Glib::RefPtr<Gtk::Adjustment> &adj, double val, Unit const *unit);
Glib::ustring get_default_filename(Glib::ustring &filename_entry_text, Glib::ustring &extension, SPDocument *doc);
std::string create_filepath_from_id(Glib::ustring id, const Glib::ustring &file_entry_text);
Glib::ustring get_ext_from_filename(Glib::ustring const &filename);
std::string absolutize_path_from_document_location(SPDocument *doc, const std::string &filename);

bool _export_raster(Geom::Rect const &area, unsigned long int const &width, unsigned long int const &height,
                    float const &dpi, Glib::ustring const &filename, bool overwrite,
                    unsigned (*callback)(float, void *), ExportProgressDialog *&prog_dialog,
                    Inkscape::Extension::Output *extension, std::vector<SPItem *> *items = nullptr,
                    AdvanceOptions *adv = nullptr);

bool _export_vector(Inkscape::Extension::Output *extension, SPDocument *doc, Glib::ustring const &filename,
                    bool overwrite, std::vector<SPItem *> *items = nullptr);

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