// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for switching tools. Also includes functions to set and get active tool.
 *
 * Copyright (C) 2020 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#ifndef INK_ACTIONS_TOOLS_H
#define INK_ACTIONS_TOOLS_H

#include <glibmm.h>
#include <2geom/point.h>

enum tools_enum {
  TOOLS_INVALID,
  TOOLS_SELECT,
  TOOLS_NODES,
  TOOLS_TWEAK,
  TOOLS_SPRAY,
  TOOLS_SHAPES_RECT,
  TOOLS_SHAPES_3DBOX,
  TOOLS_SHAPES_ARC,
  TOOLS_SHAPES_STAR,
  TOOLS_SHAPES_SPIRAL,
  TOOLS_FREEHAND_PENCIL,
  TOOLS_FREEHAND_PEN,
  TOOLS_CALLIGRAPHIC,
  TOOLS_TEXT,
  TOOLS_GRADIENT,
  TOOLS_MESH,
  TOOLS_ZOOM,
  TOOLS_MEASURE,
  TOOLS_DROPPER,
  TOOLS_CONNECTOR,
  TOOLS_PAINTBUCKET,
  TOOLS_ERASER,
  TOOLS_LPETOOL
};

class InkscapeWindow;
class SPDesktop;
class SPItem;

Glib::ustring get_active_tool(InkscapeWindow *win);
int get_active_tool_enum(InkscapeWindow *win);

void set_active_tool(InkscapeWindow* win, Glib::ustring const &tool);
void set_active_tool(InkscapeWindow* win, SPItem *item, Geom::Point const p);

void open_tool_preferences(InkscapeWindow* win, Glib::ustring const &tool);

// Deprecated: Long term goal to remove SPDesktop.
Glib::ustring get_active_tool(SPDesktop *desktop);
int get_active_tool_enum(SPDesktop *desktop);

void set_active_tool(SPDesktop *desktop, Glib::ustring const &tool);
void set_active_tool(SPDesktop *desktop, SPItem *item, Geom::Point const p);

// Standard function to add actions.
void add_actions_tools(InkscapeWindow* win);


#endif // INK_ACTIONS_TOOLS_H

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
