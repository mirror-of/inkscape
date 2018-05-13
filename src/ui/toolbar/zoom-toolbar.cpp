/**
 * @file
 * Zoom aux toolbar
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
#include "config.h"
#endif

#include "zoom-toolbar.h"

#include <gtkmm/separatortoolitem.h>

#include "desktop.h"
#include "helper/action.h"
#include "verbs.h"

//########################
//##    Zoom Toolbox    ##
//########################

namespace Inkscape {
namespace UI {
namespace Toolbar {
ZoomToolbar::ZoomToolbar(SPDesktop *desktop)
    : _desktop(desktop)
{
    auto context = Inkscape::ActionContext(_desktop);
    auto zoom_in_btn  = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_IN,  context);
    auto zoom_out_btn = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_OUT, context);
    auto zoom_1_1_btn = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_1_1, context);
    auto zoom_1_2_btn = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_1_2, context);
    auto zoom_2_1_btn = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_2_1, context);

    auto zoom_selection_btn  = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_SELECTION,  context);
    auto zoom_drawing_btn    = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_DRAWING,    context);
    auto zoom_page_btn       = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_PAGE,       context);
    auto zoom_page_width_btn = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_PAGE_WIDTH, context);

    auto zoom_prev_btn = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_PREV, context);
    auto zoom_next_btn = SPAction::create_toolbutton_for_verb(SP_VERB_ZOOM_NEXT, context);

    add(*zoom_in_btn);
    add(*zoom_out_btn);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*zoom_1_1_btn);
    add(*zoom_1_2_btn);
    add(*zoom_2_1_btn);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*zoom_selection_btn);
    add(*zoom_drawing_btn);
    add(*zoom_page_btn);
    add(*zoom_page_width_btn);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*zoom_prev_btn);
    add(*zoom_next_btn);
}

GtkWidget *
ZoomToolbar::create(SPDesktop *desktop)
{
    auto toolbar = Gtk::manage(new ZoomToolbar(desktop));
    return GTK_WIDGET(toolbar->gobj());
}
}
}
}
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
