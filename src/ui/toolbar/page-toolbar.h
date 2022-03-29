// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_PAGE_TOOLBAR_H
#define SEEN_PAGE_TOOLBAR_H

/**
 * @file
 * Page toolbar
 */
/* Authors:
 *   Martin Owens <doctormo@geek-2.com>
 *
 * Copyright (C) 2021 Martin Owens
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtkmm.h>

#include "toolbar.h"

class SPDesktop;
class SPDocument;
class SPPage;

namespace Inkscape {
class PaperSize;
namespace UI {
namespace Tools {
class ToolBase;
}
namespace Toolbar {

class PageToolbar : public Gtk::Toolbar
{
public:
    PageToolbar(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder, SPDesktop *desktop);
    ~PageToolbar() override;

    static GtkWidget *create(SPDesktop *desktop);

protected:
    void labelEdited();
    void sizeChoose();
    void sizeChanged();
    void setSizeText(SPPage *page = nullptr);

private:
    SPDesktop *_desktop;
    SPDocument *_document;

    void toolChanged(SPDesktop *desktop, Inkscape::UI::Tools::ToolBase *ec);
    void pagesChanged();
    void selectionChanged(SPPage *page);
    void on_parent_changed(Gtk::Widget *prev) override;

    sigc::connection _ec_connection;
    sigc::connection _pages_changed;
    sigc::connection _page_selected;
    sigc::connection _page_modified;

    bool was_referenced;
    Gtk::ComboBoxText *combo_page_sizes;
    Gtk::Entry *entry_page_sizes;
    Gtk::Entry *text_page_label;
    Gtk::Label *label_page_pos;
    Gtk::ToolButton *btn_page_backward;
    Gtk::ToolButton *btn_page_foreward;
    Gtk::ToolButton *btn_page_delete;
    Gtk::ToolButton *btn_move_toggle;
    Gtk::SeparatorToolItem *sep1;

    double _unit_to_size(std::string number, std::string unit_str, std::string backup);
};

} // namespace Toolbar
} // namespace UI
} // namespace Inkscape

#endif /* !SEEN_PAGE_TOOLBAR_H */

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
