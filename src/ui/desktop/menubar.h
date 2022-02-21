// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_DESKTOP_MENUBAR_H
#define SEEN_DESKTOP_MENUBAR_H

/**
 * @file
 * Desktop main menu bar code.
 */
/*
 * Authors:
 *   Tavmjong Bah
 *   Sushant A.A.
 *
 * Copyright (C) 2018 Authors
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 * Read the file 'COPYING' for more information.
 *
 */

#include <gtkmm.h> // GTK_CHECK_VERSION

void build_menu();

enum class UseIcons {
    never = -1, // Match existing preference numbering.
    as_requested,
    always,
};

// Rebuild menu with icons enabled or disabled. Recursive.
#if GTK_CHECK_VERSION(4, 0, 0)
void rebuild_menu (std::shared_ptr<Gio::MenuModel> menu, std::shared_ptr<Gio::Menu> menu_copy, UseIcons useIcons, Glib::Quark quark, Glib::RefPtr<Gio::Menu>& recent_files);
#else
void rebuild_menu (Glib::RefPtr<Gio::MenuModel>    menu, Glib::RefPtr<Gio::Menu>    menu_copy, UseIcons useIcons, Glib::Quark quark, Glib::RefPtr<Gio::Menu>& recent_files);
#endif

#endif // SEEN_DESKTOP_MENUBAR_H

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
