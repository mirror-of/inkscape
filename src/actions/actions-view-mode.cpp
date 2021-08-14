// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * 
 * Actions related to View mode
 *
 * Authors:
 *   Sushant A A <sushant.co19@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <giomm.h> 
#include <glibmm/i18n.h>

#include "ui/interface.h"
#include "ui/uxmanager.h"
#include "ui/view/view.h"

#include "actions-view-mode.h"

#include "inkscape-application.h"
#include "inkscape-window.h"

#include "ui/widget/canvas.h"

void
canvas_show_grid_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-show-grid");
    if (!action) {
        std::cerr << "canvas_show_grid_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_show_grid_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleGrids();
}

void
canvas_show_guides_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-show-guides");
    if (!action) {
        std::cerr << "canvas_show_guides_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_show_guides_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    SPDocument* doc = dt->getDocument();
    sp_namedview_toggle_guides(doc, dt->namedview);
}

void
canvas_commands_bar_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-commands-bar");
    if (!action) {
        std::cerr << "canvas_commands_bar_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_commands_bar_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleToolbar("commands");
}

void
canvas_snap_controls_bar_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-snap-controls-bar");
    if (!action) {
        std::cerr << "canvas_snap_controls_bar_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_snap_controls_bar_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleToolbar("snaptoolbox");
}

void
canvas_tool_control_bar_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-tool-control-bar");
    if (!action) {
        std::cerr << "canvas_tool_control_bar_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_tool_control_bar_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleToolbar("toppanel");
}

void
canvas_toolbox_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-toolbox");
    if (!action) {
        std::cerr << "canvas_toolbox_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_toolbox_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleToolbar("toolbox");
}

void
canvas_rulers_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-rulers");
    if (!action) {
        std::cerr << "canvas_rulers_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_rulers_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleToolbar("rulers");
}

void
canvas_scroll_bars(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-scroll-bars");
    if (!action) {
        std::cerr << "canvas_scroll_bars: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_scroll_bars: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleToolbar("scrollbars");
}

void
canvas_palette_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-palette");
    if (!action) {
        std::cerr << "canvas_palette_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_palette_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleToolbar("panels");
}

void
canvas_statusbar_toggle(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("canvas-statusbar");
    if (!action) {
        std::cerr << "canvas_statusbar_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_statusbar_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Toggle Action
    SPDesktop* dt = win->get_desktop();
    dt->toggleToolbar("statusbar");
}

void
canvas_command_palette(InkscapeWindow *win)
{
    SPDesktop* dt = win->get_desktop();
    dt->toggleCommandPalette();
}

void
view_fullscreen(InkscapeWindow *win)
{
    SPDesktop* dt = win->get_desktop();
    dt->fullscreen();
}

void
canvas_interface_mode(int value, InkscapeWindow *win)
{
    if (value < 0 || value >= 3) {
        std::cerr << "canvas_interface_mode: value out of bound! : " << value << std::endl;
        return;
    }

    auto action = win->lookup_action("canvas-interface-mode");
    if (!action) {
        std::cerr << "canvas_interface_mode: action 'canvas-interface-mode' missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_interface_mode: action 'canvas-interface-mode' not SimpleAction!" << std::endl;
        return;
    }

    // Setting Message
    Glib::ustring tip;
    if (value == 0) {
        tip = _("Default interface setup");  
    }
    else if (value == 1) {
        tip = _("Setup for custom task");  
    }
    else if (value == 2) {
        tip = _("Setup for widescreen work"); 
    }
    
    // Change state
    saction->change_state(value);

    // Set Interface
    SPDesktop* dt = win->get_desktop();
    Inkscape::UI::UXManager::getInstance()->setTask(dt, value);
    
#ifdef GDK_WINDOWING_QUARTZ
    // TODO uncomment this or figure out what to do with it.
    //  this is just to be able to build successfuly for mac.
    // call later, crashes during startup if called directly
    // g_idle_add(sync_menubar, nullptr);
#endif

    // Message FIXME having some error 
    // dt->tipsMessageContext()->clear();
    // dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, gettext( tip.c_str() )  );
    // similar =  dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, gettext( tool_msg[tool].c_str() ) );
    
}

std::vector<std::vector<Glib::ustring>> raw_data_view_mode =
{
    // clang-format off 
    {"win.canvas-show-grid",                N_("Page Grid"),                "Canvas Display",   N_("Show or hide the page grid")},
    
    {"win.canvas-show-guides",              N_("Guides"),                   "Canvas Display",   N_("Show or hide guides (drag from a ruler to create a guide)")},
    {"win.canvas-commands-bar",             N_("Commands Bar"),             "Canvas Display",   N_("Show or hide the Commands bar (under the menu)")},
    {"win.canvas-snap-controls-bar",        N_("Snap Controls Bar"),        "Canvas Display",   N_("Show or hide the snapping controls")},
    {"win.canvas-tool-control-bar",         N_("Tool Controls Bar"),        "Canvas Display",   N_("Show or hide the Tool Controls bar")},
    {"win.canvas-toolbox",                  N_("Toolbox"),                  "Canvas Display",   N_("Show or hide the main toolbox (on the left)")},
    {"win.canvas-rulers",                   N_("Rulers"),                   "Canvas Display",   N_("Show or hide the canvas rulers")},
    {"win.canvas-scroll-bars",              N_("Scroll bars"),              "Canvas Display",   N_("Show or hide the canvas scrollbars")},
    {"win.canvas-palette",                  N_("Palette"),                  "Canvas Display",   N_("Show or hide the color palette")},
    {"win.canvas-statusbar",                N_("Statusbar"),                "Canvas Display",   N_("Show or hide the statusbar (at the bottom of the window)")},
    
    {"win.canvas-command-palette",          N_("Command Palette"),          "Canvas Display",   N_("Show or hide the on-canvas command palette")},
    {"win.view-fullscreen",                 N_("Fullscreen"),               "Canvas Display",   N_("Stretch this document window to full screen")},
    
    {"win.canvas-interface-mode(0)",        N_("Default"),                  "Canvas Display",   N_("Default interface setup")},
    {"win.canvas-interface-mode(1)",        N_("Custom"),                   "Canvas Display",   N_("Setup for custom task")},
    {"win.canvas-interface-mode(2)",        N_("Wide"),                     "Canvas Display",   N_("Setup for widescreen work")}
    // clang-format on
};

void
add_actions_view_mode(InkscapeWindow* win)
{
    auto prefs = Inkscape::Preferences::get();
    SPDesktop* dt = win->get_desktop();
  
    if (!dt) {
        std::cerr << "add_actions_view_mode: no desktop!" << std::endl;
    }

    // clang-format off
    
    // Initial States of Actions
    bool commands_toggle    = prefs->getBool("/window/commands/state", true);
    bool snaptoolbox_toggle = prefs->getBool("/window/snaptoolbox/state", true);
    bool toppanel_toggle    = prefs->getBool("/window/toppanel/state", true);
    bool toolbox_toggle     = prefs->getBool("/window/toolbox/state", true);
    bool panels_toggle      = prefs->getBool("/window/panels/state", true);
    bool statusbar_toggle   = prefs->getBool("/window/statusbar/state", true);
    bool scrollbars_toggle  = prefs->getBool("/window/scrollbars/state", true);
    bool rulers_toggle      = prefs->getBool("/window/rulers/state", true);
    bool guides_toggle      = win->get_desktop()->namedview->getRepr()->getAttributeBoolean("showguides", true);    // Should set it true or retrive the state (every time it set to true on restart)
    int  interface_mode     = Inkscape::UI::UXManager::getInstance()->getDefaultTask(dt);

    win->add_action_bool(          "canvas-show-grid",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_show_grid_toggle),            win));
    win->add_action_bool(          "canvas-show-guides",            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_show_guides_toggle),          win), guides_toggle);
    win->add_action_bool(          "canvas-commands-bar",           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_commands_bar_toggle),         win), commands_toggle);
    win->add_action_bool(          "canvas-snap-controls-bar",      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_snap_controls_bar_toggle),    win), snaptoolbox_toggle);
    win->add_action_bool(          "canvas-tool-control-bar",       sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_tool_control_bar_toggle),     win), toppanel_toggle);
    win->add_action_bool(          "canvas-toolbox",                sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_toolbox_toggle),              win), toolbox_toggle);
    win->add_action_bool(          "canvas-rulers",                 sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_rulers_toggle),               win), rulers_toggle);
    win->add_action_bool(          "canvas-scroll-bars",            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_scroll_bars),                 win), scrollbars_toggle);
    win->add_action_bool(          "canvas-palette",                sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_palette_toggle),              win), panels_toggle);
    win->add_action_bool(          "canvas-statusbar",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_statusbar_toggle),            win), statusbar_toggle);
    win->add_action(               "canvas-command-palette",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_command_palette),             win));
    win->add_action(               "view-fullscreen",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&view_fullscreen),                    win));
    win->add_action_radio_integer ("canvas-interface-mode",         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_interface_mode),              win), interface_mode);
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_view_mode: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_view_mode);
}