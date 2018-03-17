/**
 * @file
 * Dropper aux toolbar
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2011 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glibmm/i18n.h>

#include <gtkmm/label.h>
#include <gtkmm/toggletoolbutton.h>

#include "dropper-toolbar.h"
#include "document-undo.h"
#include "preferences.h"
#include "widgets/spinbutton-events.h"

using Inkscape::DocumentUndo;

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * @brief Default constructor
 */
DropperToolbar::DropperToolbar()
    : _pick_alpha_button(Gtk::manage(new Gtk::ToggleToolButton(_("Pick")))),
      _set_alpha_button(Gtk::manage(new Gtk::ToggleToolButton(_("Assign"))))
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    auto pickAlpha = prefs->getInt( "/tools/dropper/pick", 1 );

    /********************************************************/
    /***** Set up label to show at start of the toolbar *****/
    /********************************************************/
    auto opacity_label = Gtk::manage(new Gtk::Label(_("Opacity:")));
    opacity_label->set_margin_start(3);
    opacity_label->set_margin_end(3);
    auto opacity_ti    = Gtk::manage(new Gtk::ToolItem());
    opacity_ti->add(*opacity_label);

    /**************************************************/
    /***** Set up opacity selection toggle button *****/
    /**************************************************/
    _pick_alpha_button->set_tooltip_text(_("Pick both the color and the alpha (transparency) "
                                           "under cursor; otherwise, pick only the visible "
                                           "color premultiplied by alpha"));

    _pick_alpha_button->set_active(pickAlpha);

    // Connect handler for "toggle" signal
    auto pick_alpha_button_toggle_cb = sigc::mem_fun(*this, &DropperToolbar::on_pick_alpha_button_toggle);
    _pick_alpha_button->signal_toggled().connect(pick_alpha_button_toggle_cb);

    /***************************************************/
    /***** Set up opacity assignment toggle button *****/
    /***************************************************/
    _set_alpha_button->set_tooltip_text(_("If alpha was picked, assign it to selection as "
                                   "fill or stroke transparency"));

    _set_alpha_button->set_active(prefs->getBool( "/tools/dropper/setalpha", true) );
    _set_alpha_button->set_sensitive(pickAlpha);

    // Connect handler for "toggle" signal
    auto set_alpha_button_toggle_cb = sigc::mem_fun(*this, &DropperToolbar::on_set_alpha_button_toggle);
    _set_alpha_button->signal_toggled().connect(set_alpha_button_toggle_cb);


    /*****************************************/
    /***** Add all tool items to toolbar *****/
    /*****************************************/
    append(*opacity_ti);
    append(*_pick_alpha_button);
    append(*_set_alpha_button);
    show_all();
}

/**
 * @brief Create a new toolbar and return a pointer to its underlying GtkWidget
 */
GtkWidget *
DropperToolbar::create(SPDesktop *desktop)
{
    auto _dropper = Gtk::manage(new DropperToolbar());
    return GTK_WIDGET(_dropper->gobj());
}

/**
 * @brief Handler for "toggle" signal on pick_alpha button
 */
void
DropperToolbar::on_pick_alpha_button_toggle()
{
    bool active = _pick_alpha_button->get_active();

    auto prefs = Inkscape::Preferences::get();
    prefs->setInt( "/tools/dropper/pick", active);

    _set_alpha_button->set_sensitive(active);

    spinbutton_defocus(GTK_WIDGET(gobj()));
}

/**
 * @brief Handler for "toggle" signal on set_alpha button
 */
void
DropperToolbar::on_set_alpha_button_toggle()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setBool( "/tools/dropper/setalpha", _set_alpha_button->get_active() );
    spinbutton_defocus(GTK_WIDGET(gobj()));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
