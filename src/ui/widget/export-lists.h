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

#include "2geom/rect.h"
#include "preferences.h"
#include "ui/widget/scrollprotected.h"

class SPDocument;
class SPItem;
class SPPage;

namespace Inkscape {
    namespace Util {
        class Unit;
    }
    namespace Extension {
        class Output;
    }
namespace UI {
namespace Dialog {

#define EXPORT_COORD_PRECISION 3
#define SP_EXPORT_MIN_SIZE 1.0
#define DPI_BASE Inkscape::Util::Quantity::convert(1, "in", "px")

// Class for storing and manipulating extensions
class ExtensionList : public Inkscape::UI::Widget::ScrollProtected<Gtk::ComboBoxText>
{
public:
    ExtensionList();
    ExtensionList(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade);
    ~ExtensionList() override {};

public:
    void setup();
    Glib::ustring getFileExtension();
    void setExtensionFromFilename(Glib::ustring const &filename);
    void removeExtension(Glib::ustring &filename);
    void createList();
    Inkscape::Extension::Output *getExtension();

private:
    PrefObserver _watch_pref;
    std::map<std::string, Inkscape::Extension::Output *> ext_to_mod;
};

class ExportList : public Gtk::Grid
{
public:
    ExportList(){};
    ExportList(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
        : Gtk::Grid(cobject){};
    ~ExportList() override = default;

public:
    void setup();
    void append_row();
    void delete_row(Gtk::Widget *widget);
    Glib::ustring get_suffix(int row);
    Inkscape::Extension::Output *getExtension(int row);
    void removeExtension(Glib::ustring &filename);
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
