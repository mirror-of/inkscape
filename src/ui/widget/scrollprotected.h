// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_INKSCAPE_UI_WIDGET_SCROLLPROTECTED_H
#define SEEN_INKSCAPE_UI_WIDGET_SCROLLPROTECTED_H

/* Authors:
 *   Thomas Holder
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 2020-2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtkmm.h>

#include "scroll-utils.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * A class decorator which blocks the scroll event if the widget does not have
 * focus and any ancestor is a scrollable window, and SHIFT is not pressed.
 *
 * For custom scroll event handlers, derived classes must implement
 * on_safe_scroll_event instead of on_scroll_event. Directly connecting to
 * signal_scroll_event() will bypass the scroll protection.
 *
 * @tparam Base A subclass of Gtk::Widget
 */
template <typename Base>
class ScrollProtected : public Base
{
public:
    using Base::Base;
    using typename Base::BaseObjectType;
    ScrollProtected()
        : Base()
    {}
    ScrollProtected(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
        : Base(cobject){};
    ~ScrollProtected() override{};

protected:
    /**
     * Event handler for "safe" scroll events which are only triggered if:
     * - the widget has focus
     * - or the widget has no scrolled window ancestor
     * - or the Shift key is pressed
     */
    virtual bool on_safe_scroll_event(GdkEventScroll *event)
    { //
        return Base::on_scroll_event(event);
    }

    bool on_scroll_event(GdkEventScroll *event) final
    {
        if (!scrolling_allowed(this, event)) {
            return false;
        }
        return on_safe_scroll_event(event);
    }
};

/**
 * A class decorator for scroll widgets like scrolled window to transfer scroll to
 * any ancestor which is is a scrollable window when scroll reached end.
 *
 * For custom scroll event handlers, derived classes must implement
 * on_safe_scroll_event instead of on_scroll_event. Directly connecting to
 * signal_scroll_event() will bypass the scroll protection.
 *
 * @tparam Base A subclass of Gtk::Widget
 */
template <typename Base>
class ScrollTransfer : public Base
{
public:
    using Base::Base;
    using typename Base::BaseObjectType;
    ScrollTransfer()
        : Base()
    {}
    ScrollTransfer(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refGlade)
        : Base(cobject){};
    ~ScrollTransfer() override{};
protected:
    /**
     * Event handler for "safe" scroll events
     */
    virtual bool on_safe_scroll_event(GdkEventScroll *event)
    { //
        return Base::on_scroll_event(event);
    }

    bool on_scroll_event(GdkEventScroll *event) final
    {
        auto scrollable = dynamic_cast<Gtk::Widget *>(Inkscape::UI::Widget::get_scrollable_ancestor(this));
        auto adj = this->get_vadjustment();
        auto before = adj->get_value();
        bool result = on_safe_scroll_event(event);
        auto after = adj->get_value();
        if (scrollable && before == after) {
            return false;
        }

        return result;
    }
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif
// vim: filetype=cpp:expandtab:shiftwidth=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
