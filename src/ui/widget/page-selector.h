// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape::UI::Widget::PageSelector - page selector widget
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_WIDGETS_PAGE_SELECTOR
#define SEEN_INKSCAPE_WIDGETS_PAGE_SELECTOR

#include <gtkmm/box.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/treemodel.h>
#include <sigc++/slot.h>

#include "object/sp-page.h"

class SPDesktop;
class SPDocument;
class SPPage;

namespace Inkscape {
namespace UI {
namespace Widget {

// class DocumentTreeModel;

class PageSelector : public Gtk::Box
{
public:
    PageSelector(SPDesktop *desktop = nullptr);
    ~PageSelector() override;

private:
    class PageModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Gtk::TreeModelColumn<SPPage *> object;

        PageModelColumns() { add(object); }
    };

    SPDesktop *_desktop;
    SPDocument *_document;

    Gtk::ComboBox _selector;
    Gtk::Button _prev_button;
    Gtk::Button _next_button;

    PageModelColumns _model_columns;
    Gtk::CellRendererText _label_renderer;
    Glib::RefPtr<Gtk::ListStore> _page_model;

    sigc::connection _selector_changed_connection;
    sigc::connection _pages_changed_connection;
    sigc::connection _page_selected_connection;
    sigc::connection _doc_replaced_connection;

    void setDocument(SPDocument *document);
    void pagesChanged();
    void selectonChanged(SPPage *page);

    void renderPageLabel(Gtk::TreeModel::const_iterator const &row);
    void setSelectedPage();
    void nextPage();
    void prevPage();
};

} // namespace Widget
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
