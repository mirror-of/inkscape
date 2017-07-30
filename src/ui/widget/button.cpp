/**
 * Generic button widget
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include "button.h"

#include <glibmm.h>

#include <gtkmm/image.h>

#include "helper/action-context.h"
#include "ui/interface.h"
#include "shortcuts.h"
#include "helper/action.h"

static void sp_button_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width);
static void sp_button_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height);

namespace Inkscape {
namespace UI {
namespace Widget {
Button::Button(Gtk::IconSize  size,
               ButtonType   type,
               SPAction      *action,
               SPAction      *doubleclick_action) :
    _action(nullptr),
    _doubleclick_action(nullptr),
    _type(type),
    _lsize(CLAMP(size, Gtk::ICON_SIZE_MENU, Gtk::ICON_SIZE_DIALOG)),
    _block_on_clicked(false)
{
    new (&_c_set_active) sigc::connection();
    new (&_c_set_sensitive) sigc::connection();

    set_action(action);

    if (doubleclick_action) {
        set_doubleclick_action(doubleclick_action);
    }

    // The Inkscape style is no-relief buttons
    set_relief(Gtk::RELIEF_NONE);
    set_border_width(0);
    set_can_focus(false);
    set_can_default(false);
}

Button::Button(Gtk::IconSize             size,
               ButtonType              type,
               Inkscape::UI::View::View *view,
               const gchar              *name,
               const gchar              *tip) :
    _type(type),
    _lsize(CLAMP(size, Gtk::ICON_SIZE_MENU, Gtk::ICON_SIZE_DIALOG)),
    _action(nullptr),
    _doubleclick_action(nullptr),
    _block_on_clicked(false)
{
    new (&_c_set_active) sigc::connection();
    new (&_c_set_sensitive) sigc::connection();

    auto action = sp_action_new(Inkscape::ActionContext(view), name, name, tip, name, 0);
    set_action(action);
    g_object_unref(action);

    // The Inkscape style is no-relief buttons
    set_relief(Gtk::RELIEF_NONE);
    set_border_width(0);
    set_can_focus(false);
    set_can_default(false);
}

static void sp_button_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));

    if (child) {
        gtk_widget_get_preferred_width(GTK_WIDGET(child), minimal_width, natural_width);
    } else {
        *minimal_width = 0;
        *natural_width = 0;
    }

    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    GtkBorder padding;
    GtkBorder border;

    gtk_style_context_get_padding(context, GTK_STATE_FLAG_NORMAL, &padding);
    gtk_style_context_get_border( context, GTK_STATE_FLAG_NORMAL, &border );

    *minimal_width += MAX(2, padding.left + padding.right + border.left + border.right);
    *natural_width += MAX(2, padding.left + padding.right + border.left + border.right);
}

static void sp_button_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));

    if (child) {
        gtk_widget_get_preferred_height(GTK_WIDGET(child), minimal_height, natural_height);
    } else {
        *minimal_height = 0;
        *natural_height = 0;
    }

    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    GtkBorder padding;
    GtkBorder border;

    gtk_style_context_get_padding(context, GTK_STATE_FLAG_NORMAL, &padding);
    gtk_style_context_get_border( context, GTK_STATE_FLAG_NORMAL, &border );

    *minimal_height += MAX(2, padding.top + padding.bottom + border.top + border.bottom);
    *natural_height += MAX(2, padding.top + padding.bottom + border.top + border.bottom);
}

void Button::clicked()
{
    if (_type == BUTTON_TYPE_TOGGLE) {
        Gtk::ToggleButton::clicked();
    }
}

bool Button::on_event(GdkEvent *event)
{
    // Run parent-class handler first
    Gtk::ToggleButton::on_event(event);

    switch (event->type) {
    case GDK_2BUTTON_PRESS:
        if (_doubleclick_action) {
            sp_action_perform(_doubleclick_action, NULL);
        }
        return true;
        break;
    default:
        break;
    }

    return false;
}

void Button::on_clicked()
{
    if (!_block_on_clicked) {
        Gtk::ToggleButton::on_clicked();

        if (_action) {
            sp_action_perform(_action, NULL);
        }
    }
}

void Button::toggle_set_down(bool down)
{
    g_return_if_fail(_type == BUTTON_TYPE_TOGGLE);
    _block_on_clicked = true;
    Gtk::ToggleButton::set_active(down);
    _block_on_clicked = false;
}

void Button::set_doubleclick_action(SPAction *action)
{
    if (_doubleclick_action) {
        g_object_unref(_doubleclick_action);
    }

    _doubleclick_action = action;
    if (action) {
        g_object_ref(action);
    }
}

void Button::set_action(SPAction *action)
{
    // First remove the old action if there is one
    if (_action) {
        _c_set_active.disconnect();
        _c_set_sensitive.disconnect();

        if (get_child()) {
            remove();
        }

        g_object_unref(_action);
    }

    _action = action;
    if (action) {
        g_object_ref(action);
        _c_set_active = action->signal_set_active.connect(
                sigc::mem_fun(this, &Button::action_set_active));
        _c_set_sensitive = action->signal_set_sensitive.connect(
                sigc::mem_fun(this, &Gtk::Widget::set_sensitive));
        if (action->image) {
            auto child = Gtk::manage(new Gtk::Image);
            child->set_from_icon_name(action->image, _lsize);
            child->show();
            add(*child);
        }
    }

    set_composed_tooltip(action);
}

void Button::action_set_active(bool active)
{
    if (_type != BUTTON_TYPE_TOGGLE) {
        return;
    }
}

void Button::set_composed_tooltip(SPAction *action)
{
    if (action) {
        unsigned int shortcut = sp_shortcut_get_primary(action->verb);
        if (shortcut != GDK_KEY_VoidSymbol) {
            // there's both action and shortcut

            gchar *key = sp_shortcut_get_label(shortcut);

            gchar *tip = g_strdup_printf("%s (%s)", action->tip, key);
            set_tooltip_text(tip);
            g_free(tip);
            g_free(key);
        } else {
            // action has no shortcut
            set_tooltip_text(action->tip);
        }
    } else {
        // no action
        set_tooltip_text(NULL);
    }
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
