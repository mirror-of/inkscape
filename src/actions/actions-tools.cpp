// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for switching tools.
 *
 * Copyright (C) 2020 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include <iostream>
#include <map>
#include <chrono>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-tools.h"

#include "inkscape-application.h"
#include "inkscape-window.h"
#include "inkscape.h"
#include "message-context.h"

#include "object/box3d.h"
#include "object/sp-ellipse.h"
#include "object/sp-flowtext.h"
#include "object/sp-offset.h"
#include "object/sp-path.h"
#include "object/sp-rect.h"
#include "object/sp-spiral.h"
#include "object/sp-star.h"
#include "object/sp-text.h"

#include "ui/dialog/dialog-container.h"
#include "ui/dialog/dialog-manager.h"
#include "ui/dialog/inkscape-preferences.h"
#include "ui/tools/connector-tool.h"
#include "ui/tools/text-tool.h"

class ToolData {
public:
    int tool = TOOLS_INVALID; // TODO: Switch to named enum
    int pref = TOOLS_INVALID;
    Glib::ustring pref_path;
};

// clang-format off
static std::map<Glib::ustring, ToolData> tool_data =
{
    {"Select",       {TOOLS_SELECT,          PREFS_PAGE_TOOLS_SELECTOR,       "/tools/select",          }},
    {"Node",         {TOOLS_NODES,           PREFS_PAGE_TOOLS_NODE,           "/tools/nodes",           }},
    {"Rect",         {TOOLS_SHAPES_RECT,     PREFS_PAGE_TOOLS_SHAPES_RECT,    "/tools/shapes/rect",     }},
    {"Arc",          {TOOLS_SHAPES_ARC,      PREFS_PAGE_TOOLS_SHAPES_ELLIPSE, "/tools/shapes/arc",      }},
    {"Star",         {TOOLS_SHAPES_STAR,     PREFS_PAGE_TOOLS_SHAPES_STAR,    "/tools/shapes/star",     }},
    {"3DBox",        {TOOLS_SHAPES_3DBOX,    PREFS_PAGE_TOOLS_SHAPES_3DBOX,   "/tools/shapes/3dbox",    }},
    {"Spiral",       {TOOLS_SHAPES_SPIRAL,   PREFS_PAGE_TOOLS_SHAPES_SPIRAL,  "/tools/shapes/spiral",   }},
    {"Pencil",       {TOOLS_FREEHAND_PENCIL, PREFS_PAGE_TOOLS_PENCIL,         "/tools/freehand/pencil", }},
    {"Pen",          {TOOLS_FREEHAND_PEN,    PREFS_PAGE_TOOLS_PEN,            "/tools/freehand/pen",    }},
    {"Calligraphic", {TOOLS_CALLIGRAPHIC,    PREFS_PAGE_TOOLS_CALLIGRAPHY,    "/tools/calligraphic",    }},
    {"Text",         {TOOLS_TEXT,            PREFS_PAGE_TOOLS_TEXT,           "/tools/text",            }},
    {"Gradient",     {TOOLS_GRADIENT,        PREFS_PAGE_TOOLS_GRADIENT,       "/tools/gradient",        }},
    {"Mesh",         {TOOLS_MESH,            PREFS_PAGE_TOOLS, /* No Page */  "/tools/mesh",            }},
    {"Zoom",         {TOOLS_ZOOM,            PREFS_PAGE_TOOLS_ZOOM,           "/tools/zoom",            }},
    {"Measure",      {TOOLS_MEASURE,         PREFS_PAGE_TOOLS_MEASURE,        "/tools/measure",         }},
    {"Dropper",      {TOOLS_DROPPER,         PREFS_PAGE_TOOLS_DROPPER,        "/tools/dropper",         }},
    {"Tweak",        {TOOLS_TWEAK,           PREFS_PAGE_TOOLS_TWEAK,          "/tools/tweak",           }},
    {"Spray",        {TOOLS_SPRAY,           PREFS_PAGE_TOOLS_SPRAY,          "/tools/spray",           }},
    {"Connector",    {TOOLS_CONNECTOR,       PREFS_PAGE_TOOLS_CONNECTOR,      "/tools/connector",       }},
    {"PaintBucket",  {TOOLS_PAINTBUCKET,     PREFS_PAGE_TOOLS_PAINTBUCKET,    "/tools/paintbucket",     }},
    {"Eraser",       {TOOLS_ERASER,          PREFS_PAGE_TOOLS_ERASER,         "/tools/eraser",          }},
    {"LPETool",      {TOOLS_LPETOOL,         PREFS_PAGE_TOOLS, /* No Page */  "/tools/lpetool",         }}
};

static std::map<Glib::ustring, Glib::ustring> tool_msg =
{
    {"Select",      N_("<b>Click</b> to Select and Transform objects, <b>Drag</b> to select many objects.")                                                                                                                   },
    {"Node",        N_("Modify selected path points (nodes) directly.")                                                                                                                                                       },
    {"Rect",        N_("<b>Drag</b> to create a rectangle. <b>Drag controls</b> to round corners and resize. <b>Click</b> to select.")                                                                                        },
    {"Arc",         N_("<b>Drag</b> to create an ellipse. <b>Drag controls</b> to make an arc or segment. <b>Click</b> to select.")                                                                                           },
    {"Star",        N_("<b>Drag</b> to create a star. <b>Drag controls</b> to edit the star shape. <b>Click</b> to select.")                                                                                                  },
    {"3DBox",       N_("<b>Drag</b> to create a 3D box. <b>Drag controls</b> to resize in perspective. <b>Click</b> to select (with <b>Ctrl+Alt</b> for single faces).")                                                      },
    {"Spiral",      N_("<b>Drag</b> to create a spiral. <b>Drag controls</b> to edit the spiral shape. <b>Click</b> to select.")                                                                                              },
    {"Pencil",      N_("<b>Drag</b> to create a freehand line. <b>Shift</b> appends to selected path, <b>Alt</b> activates sketch mode.")                                                                                     },
    {"Pen",         N_("<b>Click</b> or <b>click and drag</b> to start a path; with <b>Shift</b> to append to selected path. <b>Ctrl+click</b> to create single dots (straight line modes only).")                            },
    {"Calligraphic",N_("<b>Drag</b> to draw a calligraphic stroke; with <b>Ctrl</b> to track a guide path. <b>Arrow keys</b> adjust width (left/right) and angle (up/down).")                                                 },
    {"Text",        N_("<b>Click</b> to select or create text, <b>drag</b> to create flowed text; then type.")                                                                                                                },
    {"Gradient",    N_("<b>Drag</b> or <b>double click</b> to create a gradient on selected objects, <b>drag handles</b> to adjust gradients.")                                                                               },
    {"Mesh",        N_("<b>Drag</b> or <b>double click</b> to create a mesh on selected objects, <b>drag handles</b> to adjust meshes.")                                                                                      },
    {"Zoom",        N_("<b>Click</b> or <b>drag around an area</b> to zoom in, <b>Shift+click</b> to zoom out.")                                                                                                              },
    {"Measure",     N_("<b>Drag</b> to measure the dimensions of objects.")                                                                                                                                                   },
    {"Dropper",     N_("<b>Click</b> to set fill, <b>Shift+click</b> to set stroke; <b>drag</b> to average color in area; with <b>Alt</b> to pick inverse color; <b>Ctrl+C</b> to copy the color under mouse to clipboard")   },
    {"Tweak",       N_("To tweak a path by pushing, select it and drag over it.")                                                                                                                                             },
    {"Spray",       N_("<b>Drag</b>, <b>click</b> or <b>click and scroll</b> to spray the selected objects.")                                                                                                                 },
    {"Connector",   N_("<b>Click and drag</b> between shapes to create a connector.")                                                                                                                                         },
    {"PaintBucket", N_("<b>Click</b> to paint a bounded area, <b>Shift+click</b> to union the new fill with the current selection, <b>Ctrl+click</b> to change the clicked object's fill and stroke to the current setting.") },
    {"Eraser",      N_("<b>Drag</b> to erase.")                                                                                                                                                                               },
    {"LPETool",     N_("Choose a subtool from the toolbar")                                                                                                                                                                   } 
};
// clang-format on

Glib::ustring
get_active_tool(InkscapeWindow *win)
{
    Glib::ustring state;

    auto action = win->lookup_action("tool-switch");
    if (!action) {
        std::cerr << "git_active_tool: action 'tool-switch' missing!" << std::endl;
        return state;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "git_active_tool: action 'tool-switch' not SimpleAction!" << std::endl;
        return state;
    }

    saction->get_state(state);

    return state;
}

int
get_active_tool_enum(InkscapeWindow *win)
{
    return tool_data[get_active_tool(win)].tool;
}

void tool_switch(Glib::ustring const &tool, InkscapeWindow *win);
void tool_preferences(Glib::ustring const &tool, InkscapeWindow *win);

void
set_active_tool(InkscapeWindow *win, Glib::ustring const &tool)
{
    // Seems silly to have a function to just flip argument order... but it's consistent with other
    // external functions.
    tool_switch(tool, win);
}

void
open_tool_preferences(InkscapeWindow *win, Glib::ustring const &tool)
{
    tool_preferences(tool, win);
}

/**
 * Set tool to appropriate one to edit 'item'.
 */
void
set_active_tool(InkscapeWindow *win, SPItem *item, Geom::Point const p)
{
    if (dynamic_cast<SPRect *>(item)) {
        tool_switch("Rect", win);
    } else if (dynamic_cast<SPGenericEllipse *>(item)) {
        tool_switch("Arc", win);
    } else if (dynamic_cast<SPStar *>(item)) {
        tool_switch("Star", win);
    } else if (dynamic_cast<SPBox3D *>(item)) {
        tool_switch("3DBox", win);
    } else if (dynamic_cast<SPSpiral *>(item)) {
        tool_switch("Spiral", win);
    } else if (dynamic_cast<SPPath *>(item)) {
        if (Inkscape::UI::Tools::cc_item_is_connector(item)) {
            tool_switch("Connector", win);
        }
        else {
            tool_switch("Node", win);
        }
    } else if (dynamic_cast<SPText *>(item) || dynamic_cast<SPFlowtext *>(item))  {
        tool_switch("Text", win);
        SPDesktop* dt = win->get_desktop();
        if (!dt) {
            std::cerr << "set_active_tool: no desktop!" << std::endl;
            return;
        }
        sp_text_context_place_cursor_at (SP_TEXT_CONTEXT(dt->event_context), item, p);
    } else if (dynamic_cast<SPOffset *>(item))  {
        tool_switch("Node", win);
    }
}

/**
 * Set display mode. Callback for 'tool-switch' action.
 */
void
tool_switch(Glib::ustring const &tool, InkscapeWindow *win)
{
    // Valid tool?
    auto tool_it = tool_data.find(tool);
    if (tool_it == tool_data.end()) {
        std::cerr << "tool-switch: invalid tool name: " << tool << std::endl;
        return;
    }

    // Have desktop?
    SPDesktop* dt = win->get_desktop();
    if (!dt) {
        std::cerr << "tool_switch: no desktop!" << std::endl;
        return;
    }

    auto action = win->lookup_action("tool-switch");
    if (!action) {
        std::cerr << "tool-switch: action 'tool-switch' missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "tool-switch: action 'tool-switch' not SimpleAction!" << std::endl;
        return;
    }

    // Get current state
    Glib::ustring current_tool;
    saction->get_state(current_tool);

    // Initialize time to zero.
    static std::chrono::time_point old_time = std::chrono::time_point<std::chrono::high_resolution_clock>();

    if (tool == current_tool) {
        /*
         * This happens under two circumstances:
         * 1. The user double clicks a tool. In this case we pop-up the Preference Dialog opened to the
         *    tool's page. (This only works if the Preference Dialog was not open.)
         * 2. The user is switching tools. This happens as a RadioButton triggers the action both when
         *    toggling on and when toggling off. We want to ignore the toggling off event.
         *    Note, if a user clicks on two different tool buttons quickly, it will trigger opening
         *    the first tool's preference page. If this is a problem, we could connect to
         *    the "button_pressed" signal of the buttons via code, but this would be messier.
         */
        auto current_time =  std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = current_time - old_time; // In seconds
        // std::cout << "  elapased time: " << elapsed_seconds.count() << std::endl;
        auto settings = Gtk::Settings::get_default();
        Glib::PropertyProxy<int> double_click_time = settings->property_gtk_double_click_time(); // In ms. Default: 400ms.
        if (elapsed_seconds.count() * 1000 < double_click_time.get_value()) {
            // User double clicked!
            tool_preferences(tool, win);
        }

        old_time = current_time; // So if tool is already open, double clicking will still work.
        return;
    }

    old_time = std::chrono::high_resolution_clock::now();

    // Update button states.
    saction->set_enabled(false); // Avoid infinite loop when called by tool_toogle().
    saction->change_state(tool);
    saction->set_enabled(true);

    // Switch to new tool. TODO: Clean this up. This should be one window function. Setting tool via preference path is a bit strange.
    dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, gettext( tool_msg[tool].c_str() ) );
    dt->setEventContext(tool_data[tool].pref_path);
    INKSCAPE.eventcontext_set(dt->getEventContext());
}

/**
 * Open preferences page for tool. Could be turned into actions if need be.
 */
void
tool_preferences(Glib::ustring const &tool, InkscapeWindow *win)
{
    // Valid tool?
    auto tool_it = tool_data.find(tool);
    if (tool_it == tool_data.end()) {
        std::cerr << "tool-preferences: invalid tool name: " << tool << std::endl;
        return;
    }

    // Have desktop?
    SPDesktop* dt = win->get_desktop();
    if (!dt) {
        std::cerr << "tool-preferences: no desktop!" << std::endl;
        return;
    }

    auto prefs = Inkscape::Preferences::get();
    prefs->setInt("/dialogs/preferences/page", tool_it->second.pref);
    Inkscape::UI::Dialog::DialogContainer* container = dt->getContainer();

    // Create dialog if it doesn't exist (also sets page if dialog not already in opened tab).
    container->new_floating_dialog("Preferences");

    // Find dialog and explicitly set page (in case not set in previous line).
    auto dialog = Inkscape::UI::Dialog::DialogManager::singleton().find_floating_dialog("Preferences");
    if (dialog) {
        auto pref_dialog = dynamic_cast<Inkscape::UI::Dialog::InkscapePreferences *>(dialog);
        if (pref_dialog) {
            pref_dialog->showPage(); // Switch to page indicated in preferences file (set above).
        }
    }
}

/**
 * Toggle between "Selector" and last used tool.
 */
void
tool_toggle(InkscapeWindow *win)
{
    SPDesktop* dt = win->get_desktop();
    if (!dt) {
        std::cerr << "tool_toggle: no desktop!" << std::endl;
        return;
    }

    auto action = win->lookup_action("tool-switch");
    if (!action) {
        std::cerr << "tool_toggle: action 'tool_switch' missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "tool_toogle: action 'tool_switch' not SimpleAction!" << std::endl;
        return;
    }

    static Glib::ustring old_tool = "Select";

    Glib::ustring tool;
    saction->get_state(tool);
    if (tool == "Select") {
        tool = old_tool;
    } else {
        old_tool = tool;
        tool = "Select";
    }

    tool_switch(tool, win);
}

Glib::ustring get_active_tool(SPDesktop *desktop)
{
    InkscapeWindow* win = desktop->getInkscapeWindow();
    return get_active_tool(win);
}

int get_active_tool_enum(SPDesktop *desktop)
{
    InkscapeWindow* win = desktop->getInkscapeWindow();
    return get_active_tool_enum(win);
}

void set_active_tool(SPDesktop *desktop, Glib::ustring const &tool)
{
    InkscapeWindow* win = desktop->getInkscapeWindow();
    set_active_tool(win, tool);
}

void set_active_tool(SPDesktop *desktop, SPItem *item, Geom::Point const p)
{
    InkscapeWindow* win = desktop->getInkscapeWindow();
    set_active_tool(win, item, p);
}

std::vector<std::vector<Glib::ustring>> raw_data_tools =
{
    // clang-format off
    {"win.tool-switch('Select')",       N_("Tool: Select"),       "Tool Switch",   N_("Select and transform objects.")                  },
    {"win.tool-switch('Node')",         N_("Tool: Node"),         "Tool Switch",   N_("Edit paths by nodes.")                           },

    {"win.tool-switch('Rect')",         N_("Tool: Rectangle"),    "Tool Switch",   N_("Create rectangles and squares.")                 },
    {"win.tool-switch('Arc')",          N_("Tool: Circle/Arc"),   "Tool Switch",   N_("Create circles, ellipses and arcs.")             },
    {"win.tool-switch('Star')",         N_("Tool: Star/Polygon"), "Tool Switch",   N_("Create stars and polygons.")                     },
    {"win.tool-switch('3DBox')",        N_("Tool: 3D Box"),       "Tool Switch",   N_("Create 3D Boxes.")                               },
    {"win.tool-switch('Spiral')",       N_("Tool: Spiral"),       "Tool Switch",   N_("Create spirals.")                                },

    {"win.tool-switch('Pen')",          N_("Tool: Pen"),          "Tool Switch",   N_("Draw Bezier curves and straight lines.")         },
    {"win.tool-switch('Pencil')",       N_("Tool: Pencil"),       "Tool Switch",   N_("Draw freehand lines.")                           },
    {"win.tool-switch('Calligraphic')", N_("Tool: Calligraphy"),  "Tool Switch",   N_("Draw calligraphic or brush strokes.")            },
    {"win.tool-switch('Text')",         N_("Tool: Text"),         "Tool Switch",   N_("Create and edit text objects.")                  },

    {"win.tool-switch('Gradient')",     N_("Tool: Gradient"),     "Tool Switch",   N_("Create and edit gradients.")                     },
    {"win.tool-switch('Mesh')",         N_("Tool: Mesh"),         "Tool Switch",   N_("Create and edit meshes.")                        },
    {"win.tool-switch('Dropper')",      N_("Tool: Dropper"),      "Tool Switch",   N_("Pick colors from image.")                        },
    {"win.tool-switch('PaintBucket')",  N_("Tool: Paint Bucket"), "Tool Switch",   N_("Fill bounded areas.")                            },

    {"win.tool-switch('Tweak')",        N_("Tool: Tweak"),        "Tool Switch",   N_("Tweak objects by sculpting or painting.")        },
    {"win.tool-switch('Spray')",        N_("Tool: Spray"),        "Tool Switch",   N_("Spray objects by sculpting or painting.")        },
    {"win.tool-switch('Eraser')",       N_("Tool: Eraser"),       "Tool Switch",   N_("Erase objects or paths.")                        },
    {"win.tool-switch('Connector')",    N_("Tool: Connector"),    "Tool Switch",   N_("Create diagram connectors.")                     },
    {"win.tool-switch('LPETool')",      N_("Tool: LPE Tool"),     "Tool Switch",   N_("Do geometric constructions.")                    },

    {"win.tool-switch('Zoom')",         N_("Tool: Zoom"),         "Tool Switch",   N_("Zoom in or out.")                                },
    {"win.tool-switch('Measure')",      N_("Tool: Measure"),      "Tool Switch",   N_("Measure objects.")                               },

    {"win.tool-toggle",                 N_("Tool Toggle"),        "Tool Switch",   N_("Toggle between Select tool and last used tool.") },
    // clang-format on
};


void
add_actions_tools(InkscapeWindow* win)
{
    // clang-format off
    win->add_action_radio_string ( "tool-switch",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&tool_switch),  win), "Select");
    win->add_action(               "tool-toggle",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&tool_toggle),  win)          );
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_tools: no app!" << std::endl;
        return;
    }

    app->get_action_extra_data().add_data(raw_data_tools);
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
