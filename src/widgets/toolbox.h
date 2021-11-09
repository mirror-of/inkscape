// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_TOOLBOX_H
#define SEEN_TOOLBOX_H

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <glibmm/ustring.h>
#include <gtk/gtk.h>
#include <gtkmm/enums.h>

#include "preferences.h"

class SPDesktop;

namespace Inkscape {
namespace UI {

/**
 * Main toolbox source.
 */
class ToolboxFactory
{
public:
    static void setToolboxDesktop(GtkWidget *toolbox, SPDesktop *desktop);
    static void setOrientation(GtkWidget* toolbox, GtkOrientation orientation);
    static void showAuxToolbox(GtkWidget* toolbox);

    static GtkWidget *createToolToolbox();
    static GtkWidget *createAuxToolbox();
    static GtkWidget *createCommandsToolbox();
    static GtkWidget *createSnapToolbox();

    static int prefToPixelSize(Glib::ustring const& path);
    static Gtk::IconSize prefToSize_mm(Glib::ustring const &path, int base = 0);

    static void set_icon_size(GtkWidget* toolbox, int pixel_size);
    ToolboxFactory() = delete;

    static constexpr const char* tools_icon_size = "/toolbox/tools/iconsize";
    static constexpr const char* tools_visible_buttons = "/toolbox/tools/buttons"; 
    static constexpr const char* ctrlbars_icon_size = "/toolbox/controlbars/iconsize";
    static constexpr const char* snap_bar_simple = "/toolbox/simplesnap";
    static constexpr const int min_pixel_size = 16;
    static constexpr const int max_pixel_size = 48;
    static Glib::ustring get_tool_visible_buttons_path(const Glib::ustring& button_action_name);
};



} // namespace UI
} // namespace Inkscape

#endif /* !SEEN_TOOLBOX_H */

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
