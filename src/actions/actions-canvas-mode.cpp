// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for changing the canvas display mode. Tied to a particular InkscapeWindow.
 *
 * Copyright (C) 2020 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "ui/interface.h"
#include "ui/uxmanager.h"
#include "ui/view/view.h"

#include "actions-canvas-mode.h"

#include "inkscape-application.h"
#include "inkscape-window.h"

#include "display/rendermode.h"
#include "display/drawing.h"  // Setting gray scale parameters.
#include "display/control/canvas-item-drawing.h"

#include "ui/widget/canvas.h"

// TODO: Use action state rather than set variable in Canvas (via Desktop).
// TODO: Move functions from Desktop to Canvas.
// TODO: Canvas actions should belong to canvas (not window)!

/**
 * Helper function to set display mode.
 */
void
canvas_set_display_mode(Inkscape::RenderMode value, InkscapeWindow *win, Glib::RefPtr<Gio::SimpleAction> saction)
{
    g_assert(value != Inkscape::RenderMode::size);
    saction->change_state((int)value);

    // Save value as a preference
    Inkscape::Preferences *pref = Inkscape::Preferences::get();
    pref->setInt("/options/displaymode", (int)value);

    SPDesktop* dt = win->get_desktop();
    auto canvas = dt->getCanvas();
    canvas->set_render_mode(Inkscape::RenderMode(value));
}

/**
 * Set display mode.
 */
void
canvas_display_mode(int value, InkscapeWindow *win)
{
    if (value < 0 || value >= (int)Inkscape::RenderMode::size) {
        std::cerr << "canvas_display_mode: value out of bound! : " << value << std::endl;
        return;
    }

    auto action = win->lookup_action("canvas-display-mode");
    if (!action) {
        std::cerr << "canvas_display_mode: action 'canvas-display-mode' missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_display_mode: action 'canvas-display-mode' not SimpleAction!" << std::endl;
        return;
    }

    canvas_set_display_mode(Inkscape::RenderMode(value), win, saction);
}

/**
 * Cycle between values.
 */
void
canvas_display_mode_cycle(InkscapeWindow *win)
{
    auto action = win->lookup_action("canvas-display-mode");
    if (!action) {
        std::cerr << "canvas_display_mode_cycle: action 'canvas-display-mode' missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_display_mode_cycle: action 'canvas-display-mode' not SimpleAction!" << std::endl;
        return;
    }

    int value = -1;
    saction->get_state(value);
    value++;
    value %= (int)Inkscape::RenderMode::size;

    canvas_set_display_mode((Inkscape::RenderMode)value, win, saction);
}


/**
 * Toggle between normal and last set other value.
 */
void
canvas_display_mode_toggle(InkscapeWindow *win)
{
    auto action = win->lookup_action("canvas-display-mode");
    if (!action) {
        std::cerr << "canvas_display_mode_toggle: action 'canvas-display-mode' missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_display_mode_toogle: action 'canvas-display-mode' not SimpleAction!" << std::endl;
        return;
    }

    static Inkscape::RenderMode old_value = Inkscape::RenderMode::OUTLINE;

    int value = -1;
    saction->get_state(value);
    if (value == (int)Inkscape::RenderMode::NORMAL) {
        canvas_set_display_mode(old_value, win, saction);
    } else {
        old_value = Inkscape::RenderMode(value);
        canvas_set_display_mode(Inkscape::RenderMode::NORMAL, win, saction);
    }
}


/**
 * Set split mode.
 */
void
canvas_split_mode(int value, InkscapeWindow *win)
{
    if (value < 0 || value >= (int)Inkscape::SplitMode::size) {
        std::cerr << "canvas_split_mode: value out of bound! : " << value << std::endl;
        return;
    }

    auto action = win->lookup_action("canvas-split-mode");
    if (!action) {
        std::cerr << "canvas_split_mode: action 'canvas-split-mode' missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_split_mode: action 'canvas-split-mode' not SimpleAction!" << std::endl;
        return;
    }

    // If split mode is already set to the requested mode, turn it off.
    int old_value = -1;
    saction->get_state(old_value);
    if (value == old_value) {
        value = (int)Inkscape::SplitMode::NORMAL;
    }

    saction->change_state(value);

    SPDesktop* dt = win->get_desktop();
    auto canvas = dt->getCanvas();
    canvas->set_split_mode(Inkscape::SplitMode(value));
}

/**
 * Set gray scale for canvas.
 */
void
canvas_color_mode_gray(InkscapeWindow *win)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble r = prefs->getDoubleLimited("/options/rendering/grayscale/red-factor",   0.21,  0.0, 1.0);
    gdouble g = prefs->getDoubleLimited("/options/rendering/grayscale/green-factor", 0.72,  0.0, 1.0);
    gdouble b = prefs->getDoubleLimited("/options/rendering/grayscale/blue-factor",  0.072, 0.0, 1.0);
    gdouble grayscale_value_matrix[20] =
        { r, g, b, 0, 0,
          r, g, b, 0, 0,
          r, g, b, 0, 0,
          0, 0, 0, 1, 0 };
    SPDesktop* dt = win->get_desktop();
    dt->getCanvasDrawing()->get_drawing()->setGrayscaleMatrix(grayscale_value_matrix);
}

/**
 * Toggle Gray scale on/off.
 */
void
canvas_color_mode_toggle(InkscapeWindow *win)
{
    auto action = win->lookup_action("canvas-color-mode");
    if (!action) {
        std::cerr << "canvas_color_mode_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_color_mode_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    if (state) {
        // Set gray scale parameters.
        canvas_color_mode_gray(win);
    }

    SPDesktop* dt = win->get_desktop();
    auto canvas = dt->getCanvas();
    canvas->set_color_mode(state ? Inkscape::ColorMode::GRAYSCALE : Inkscape::ColorMode::NORMAL);
}


/**
 * Toggle Color management on/off.
 */
void
canvas_color_manage_toggle(InkscapeWindow *win)
{
    auto action = win->lookup_action("canvas-color-manage");
    if (!action) {
        std::cerr << "canvas_color_manage_toggle: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "canvas_color_manage_toggle: action not SimpleAction!" << std::endl;
        return;
    }

    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    // Save value as a preference
    Inkscape::Preferences *pref = Inkscape::Preferences::get();
    pref->setBool("/options/displayprofile/enable", state);

    SPDesktop* dt = win->get_desktop();
    auto canvas = dt->getCanvas();
    canvas->set_cms_active(state);
    canvas->redraw_all();
}

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
window_new(InkscapeWindow *win)
{
    sp_ui_new_view();
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
    // call later, crashes during startup if called directly
    g_idle_add(sync_menubar, nullptr);
#endif

    // Message FIXME having some error 
    // dt->tipsMessageContext()->clear();
    // dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, gettext( tip.c_str() )  );
    // similar =  dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, gettext( tool_msg[tool].c_str() ) );
    
}

std::vector<std::vector<Glib::ustring>> raw_data_canvas_mode =
{
    // clang-format off
    {"win.canvas-display-mode(0)",      N_("Display Mode: Normal"),       "Canvas Display",   N_("Use normal rendering mode")                         },
    {"win.canvas-display-mode(1)",      N_("Display Mode: Outline"),      "Canvas Display",   N_("Show only object outlines")                         },
    {"win.canvas-display-mode(2)",      N_("Display Mode: No Filters"),   "Canvas Display",   N_("Do not render filters (for speed)")                 },
    {"win.canvas-display-mode(3)",      N_("Display Mode: Hairlines"),    "Canvas Display",   N_("Render thin lines visibly")                         },
    {"win.canvas-display-mode-cycle",   N_("Display Mode Cycle"),         "Canvas Display",   N_("Cycle through display modes")                       },
    {"win.canvas-display-mode-toggle",  N_("Display Mode Toggle"),        "Canvas Display",   N_("Toggle between normal and last non-normal mode")    },

    {"win.canvas-split-mode(0)",        N_("Split Mode: Normal"),         "Canvas Display",   N_("Do not split canvas")                               },
    {"win.canvas-split-mode(1)",        N_("Split Mode: Split"),          "Canvas Display",   N_("Render part of the canvas in outline mode")         },
    {"win.canvas-split-mode(2)",        N_("Split Mode: X-Ray"),          "Canvas Display",   N_("Render a circular area in outline mode")            },

    {"win.canvas-color-mode",           N_("Color Mode"),                 "Canvas Display",   N_("Toggle between normal and grayscale modes")         },
    {"win.canvas-color-manage",         N_("Color Managed Mode"),         "Canvas Display",   N_("Toggle between normal and color managed modes")     },
    
    {"win.canvas-show-grid",            N_("Page Grid"),                  "Canvas Display",   N_("Show or hide the page grid")     },
    
    {"win.canvas-show-guides",          N_("Guides"),                     "Canvas Display",   N_("Show or hide guides (drag from a ruler to create a guide)")     },
    {"win.canvas-commands-bar",         N_("Commands Bar"),               "Canvas Display",   N_("Show or hide the Commands bar (under the menu)")     },
    {"win.canvas-snap-controls-bar",    N_("Snap Controls Bar"),          "Canvas Display",   N_("Show or hide the snapping controls")},
    {"win.canvas-tool-control-bar",     N_("Tool Controls Bar"),          "Canvas Display",   N_("Show or hide the Tool Controls bar")},
    {"win.canvas-toolbox",              N_("Toolbox"),                    "Canvas Display",   N_("Show or hide the main toolbox (on the left)")},
    {"win.canvas-rulers",               N_("Rulers"),                     "Canvas Display",   N_("Show or hide the canvas rulers")},
    {"win.canvas-scroll-bars",          N_("Scroll bars"),                "Canvas Display",   N_("Show or hide the canvas scrollbars")},
    {"win.canvas-palette",              N_("Palette"),                    "Canvas Display",   N_("Show or hide the color palette")},
    {"win.canvas-statusbar",            N_("Statusbar"),                  "Canvas Display",   N_("Show or hide the statusbar (at the bottom of the window)")},
    
    {"win.canvas-command-palette",      N_("Command Palette"),            "Canvas Display",   N_("Show or hide the on-canvas command palette")},
    {"win.window-new",                  N_("Duplicate Window"),           "Canvas Display",   N_("Open a new window with the same document")},
    {"win.view-fullscreen",             N_("Fullscreen"),                 "Canvas Display",   N_("Stretch this document window to full screen")},
    
    {"win.canvas-interface-mode(0)",      N_("Default"),       "Canvas Display",   N_("Default interface setup")                         },
    {"win.canvas-interface-mode(1)",      N_("Custom"),      "Canvas Display",   N_("Setup for custom task")                         },
    {"win.canvas-interface-mode(2)",      N_("Wide"),   "Canvas Display",   N_("Setup for widescreen work")                 }
    
    // clang-format on
};

void
add_actions_canvas_mode(InkscapeWindow* win)
{
    // Sync action with desktop variables. TODO: Remove!
    auto prefs = Inkscape::Preferences::get();
    SPDesktop* dt = win->get_desktop();

    // Initial States of Actions
    int  display_mode       = prefs->getIntLimited("/options/displaymode", 0, 0, 4);  // Default, minimum, maximum
    bool color_manage       = prefs->getBool("/options/displayprofile/enable");
    bool commands_toggle    = prefs->getBool("/window/commands/state", true);
    bool snaptoolbox_toggle = prefs->getBool("/window/snaptoolbox/state", true);
    bool toppanel_toggle    = prefs->getBool("/window/toppanel/state", true);
    bool toolbox_toggle     = prefs->getBool("/window/toolbox/state", true);
    bool panels_toggle      = prefs->getBool("/window/panels/state", true);
    bool statusbar_toggle   = prefs->getBool("/window/statusbar/state", true);
    bool scrollbars_toggle  = prefs->getBool("/window/scrollbars/state", true);
    bool rulers_toggle      = prefs->getBool("/window/rulers/state", true);
    bool guides_toggle      = win->get_desktop()->namedview->getRepr()->getAttributeBoolean("showguides", true);    // Should set it true or retrive the state (every time it set to true on restart)
    int interface_mode      = Inkscape::UI::UXManager::getInstance()->getDefaultTask(dt);

    if (dt) {
        auto canvas = dt->getCanvas();
        canvas->set_render_mode(Inkscape::RenderMode(display_mode));
        canvas->set_cms_active(color_manage);
    } else {
        std::cerr << "add_actions_canvas_mode: no desktop!" << std::endl;
    }

    // clang-format off
    win->add_action_radio_integer ("canvas-display-mode",           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_display_mode),                win), display_mode);
    win->add_action(               "canvas-display-mode-cycle",     sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_display_mode_cycle),          win));
    win->add_action(               "canvas-display-mode-toggle",    sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_display_mode_toggle),         win));
    win->add_action_radio_integer ("canvas-split-mode",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_split_mode),                  win), (int)Inkscape::SplitMode::NORMAL);
    win->add_action_bool(          "canvas-color-mode",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_color_mode_toggle),           win));
    win->add_action_bool(          "canvas-color-manage",           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_color_manage_toggle),         win), color_manage);
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
    win->add_action(               "window-new",                    sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&window_new),                         win));
    win->add_action(               "view-fullscreen",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&view_fullscreen),                    win));
    win->add_action_radio_integer ("canvas-interface-mode",         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_interface_mode),              win), interface_mode);
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_canvas_mode: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_canvas_mode);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
