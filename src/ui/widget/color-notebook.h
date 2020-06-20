// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * A notebook with RGB, CMYK, CMS, HSL, and Wheel pages
 *//*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Tomasz Boczkowski <penginsbacon@gmail.com> (c++-sification)
 *
 * Copyright (C) 2001-2014 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef SEEN_SP_COLOR_NOTEBOOK_H
#define SEEN_SP_COLOR_NOTEBOOK_H

#ifdef HAVE_CONFIG_H
# include "config.h"  // only include where actually required!
#endif

#include <boost/ptr_container/ptr_vector.hpp>
#include <gtkmm/grid.h>
#include <glib.h>

#include "color.h"
#include "ui/selected-color.h"

namespace Gtk {
    class Box;
    class Button;
    class EventBox;
    class Label;
    class Notebook;
    class RadioButton;
}

namespace Inkscape {
namespace UI {
namespace Widget {

class ColorNotebook
    : public Gtk::Grid
{
public:
    ColorNotebook(SelectedColor &color);

protected:
    struct Page {
        Page(Inkscape::UI::ColorSelectorFactory *selector_factory, bool enabled_full);

        Inkscape::UI::ColorSelectorFactory *selector_factory;
        bool enabled_full;
    };

    virtual void _initUI();
    void _addPage(Page &page);

    void _onButtonClicked(int page_num);
    void _onPickerClicked();
    void _onPageSwitched(Gtk::Widget *page, guint page_num);
    virtual void _onSelectedColorChanged();

    void _updateICCButtons();
    void _setCurrentPage(int i);

    Inkscape::UI::SelectedColor &_selected_color;
    gulong _entryId;
    Gtk::Notebook *_book;
    Gtk::Box *_buttonbox;
    std::vector<Gtk::RadioButton *> _buttons;
    Gtk::Label *_rgbal; /* RGBA entry */
#if defined(HAVE_LIBLCMS2)
    Gtk::EventBox *_box_outofgamut;
    Gtk::EventBox *_box_colormanaged;
    Gtk::EventBox *_box_toomuchink;
#endif // defined(HAVE_LIBLCMS2)
    Gtk::Button *_btn_picker;
    GtkWidget *_p; /* Color preview */
    boost::ptr_vector<Page> _available_pages;

private:
    // By default, disallow copy constructor and assignment operator
    ColorNotebook(const ColorNotebook &obj) = delete;
    ColorNotebook &operator=(const ColorNotebook &obj) = delete;
};
}
}
}
#endif // SEEN_SP_COLOR_NOTEBOOK_H
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

