/**
 * \brief StockItems for Inkscape-specific menu/button labels and key
 *        accelerators.
 *
 * Author:
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Derek P. Moore
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <gtkmm/stock.h>
#include <gtkmm/stockitem.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>

#include "stock-items.h"
#include "stock.h"

namespace Inkscape {
namespace UI {
namespace Stock {

void
init()
{
    using namespace Gtk::Stock;

    // File menu
    add(Gtk::StockItem(OPEN, _("Open...")));
    add(Gtk::StockItem(OPEN_RECENT, _("Open _Recent")));
    add(Gtk::StockItem(SAVE_AS, _("Save _As...")));
    add(Gtk::StockItem(IMPORT, _("_Import...")));
    add(Gtk::StockItem(EXPORT, _("_Export...")));
    add(Gtk::StockItem(PRINT, _("_Print...")));
    add(Gtk::StockItem(PRINT_PREVIEW, _("Print Previe_w")));
    add(Gtk::StockItem(VACUUM_DEFS, _("Vac_uum Defs")));
    add(Gtk::StockItem(PROPERTIES, _("_Document Properties")));
    add(Gtk::StockItem(PREFERENCES, _("In_kscape Preferences")));

    // Edit menu
    add(Gtk::StockItem(PASTE_IN_PLACE, _("Paste _In Place")));
    add(Gtk::StockItem(PASTE_STYLE, _("Paste _Style")));
    add(Gtk::StockItem(DUPLICATE, _("Duplic_ate")));
    add(Gtk::StockItem(CLONE, _("Clo_ne")));
    add(Gtk::StockItem(CLONE_UNLINK, _("Unlin_k Clone")));
    add(Gtk::StockItem(CLONE_SELECT_ORIG, _("Select _Original Clone")));
    add(Gtk::StockItem(MAKE_BITMAP, _("_Make a Bitmap Copy")));
    add(Gtk::StockItem(TILE, _("Tile")));
    add(Gtk::StockItem(UNTILE, _("Untile")));
    add(Gtk::StockItem(SELECT_ALL, _("Select A_ll")));
    add(Gtk::StockItem(SELECT_ALL_IN_ALL_LAYERS, _("Select All in All La_yers")));
    add(Gtk::StockItem(SELECT_INVERT, _("In_vert Selection")));
    add(Gtk::StockItem(SELECT_NONE, _("Select Non_e")));
    add(Gtk::StockItem(XML_EDITOR, _("_XML Editor...")));

    // View menu
    add(Gtk::StockItem(ZOOM, _("_Zoom")));
    add(Gtk::StockItem(ZOOM_IN, _("Zoom _In")));
    add(Gtk::StockItem(ZOOM_OUT, _("Zoom _Out")));
    add(Gtk::StockItem(ZOOM_100, _("1:_1 (100%)")));
    add(Gtk::StockItem(ZOOM_50, _("1:_2 (50%)")));
    add(Gtk::StockItem(ZOOM_200, _("2:1 (2_00%)")));
    add(Gtk::StockItem(ZOOM_SELECTION, _("_Selection")));
    add(Gtk::StockItem(ZOOM_DRAWING, _("_Drawing")));
    add(Gtk::StockItem(ZOOM_PAGE, _("_Page")));
    add(Gtk::StockItem(ZOOM_WIDTH, _("_Width")));
    add(Gtk::StockItem(ZOOM_PREV, _("Pre_vious")));
    add(Gtk::StockItem(ZOOM_NEXT, _("Nex_t")));
    add(Gtk::StockItem(SHOW_HIDE, _("_Show/Hide")));
    add(Gtk::StockItem(SHOW_HIDE_COMMANDS_BAR, _("_Commands bar")));
    add(Gtk::StockItem(SHOW_HIDE_TOOL_CONTROLS_BAR, _("Tool Co_ntrols bar")));
    add(Gtk::StockItem(SHOW_HIDE_TOOLS_BAR, _("_Tools bar")));
    add(Gtk::StockItem(SHOW_HIDE_RULERS, _("_Rulers")));
    add(Gtk::StockItem(SHOW_HIDE_SCROLLBARS, _("Scroll_bars")));
    add(Gtk::StockItem(SHOW_HIDE_STATUSBAR, _("_Statusbar")));
    add(Gtk::StockItem(SHOW_HIDE_DIALOGS, _("Show/_Hide Dialogs")));
    add(Gtk::StockItem(GRID, _("_Grid")));
    add(Gtk::StockItem(GUIDES, _("G_uides")));
    add(Gtk::StockItem(FULLSCREEN, _("_Fullscreen")));
    add(Gtk::StockItem(MESSAGES, _("_Messages...")));
    add(Gtk::StockItem(SCRIPTS, _("S_cripts...")));
    add(Gtk::StockItem(WINDOW_PREV, _("P_revious Window")));
    add(Gtk::StockItem(WINDOW_NEXT, _("N_ext Window")));
    add(Gtk::StockItem(WINDOW_DUPLICATE, _("D_uplicate Window")));

    // Layer menu
    add(Gtk::StockItem(LAYER_NEW, _("_New Layer...")));
    add(Gtk::StockItem(LAYER_RENAME, _("R_ename Layer...")));
    add(Gtk::StockItem(LAYER_DUPLICATE, _("D_uplicate Layer")));
    add(Gtk::StockItem(LAYER_ANCHOR, _("_Anchor Layer")));
    add(Gtk::StockItem(LAYER_MERGE_DOWN, _("Merge Do_wn")));
    add(Gtk::StockItem(LAYER_DELETE, _("_Delete Layer")));
    add(Gtk::StockItem(LAYER_SELECT_NEXT, _("Select Ne_xt Layer")));
    add(Gtk::StockItem(LAYER_SELECT_PREV, _("Select Pre_vious Layer")));
    add(Gtk::StockItem(LAYER_SELECT_TOP, _("Select To_p Layer")));
    add(Gtk::StockItem(LAYER_SELECT_BOTTOM, _("Select Botto_m Layer")));
    add(Gtk::StockItem(LAYER_RAISE, _("_Raise Layer")));
    add(Gtk::StockItem(LAYER_LOWER, _("_Lower Layer")));
    add(Gtk::StockItem(LAYER_TO_TOP, _("Layer to _Top")));
    add(Gtk::StockItem(LAYER_TO_BOTTOM, _("Layer to _Bottom")));

    // Object menu
    add(Gtk::StockItem(FILL_STROKE, _("_Fill and Stroke...")));
    add(Gtk::StockItem(OBJECT_PROPERTIES, _("_Object Properties...")));
    add(Gtk::StockItem(GROUP, _("_Group")));
    add(Gtk::StockItem(UNGROUP, _("_Ungroup")));
    add(Gtk::StockItem(RAISE, _("_Raise")));
    add(Gtk::StockItem(LOWER, _("_Lower")));
    add(Gtk::StockItem(RAISE_TO_TOP, _("Raise to _Top")));
    add(Gtk::StockItem(LOWER_TO_BOTTOM, _("Lower to _Bottom")));
    add(Gtk::StockItem(MOVE_TO_NEW_LAYER, _("Move to Ne_w Layer")));
    add(Gtk::StockItem(MOVE_TO_NEXT_LAYER, _("Move to Ne_xt Layer")));
    add(Gtk::StockItem(MOVE_TO_PREV_LAYER, _("Move to Pre_vious Layer")));
    add(Gtk::StockItem(MOVE_TO_TOP_LAYER, _("Move to To_p Layer")));
    add(Gtk::StockItem(MOVE_TO_BOTTOM_LAYER, _("Move to B_ottom Layer")));
    add(Gtk::StockItem(ROTATE_90_CW, _("Rotate _90° CW")));
    add(Gtk::StockItem(ROTATE_90_CCW, _("Rotate 9_0° CCW")));
    add(Gtk::StockItem(FLIP_HORIZ, _("Flip _Horizontally")));
    add(Gtk::StockItem(FLIP_VERT, _("Flip _Vertically")));
    add(Gtk::StockItem(TRANSFORM, _("Transfor_m...")));
    add(Gtk::StockItem(TRANSFORMATION, _("Transfor_m...")));
    add(Gtk::StockItem(ALIGN_DISTRIBUTE, _("_Align and Distribute...")));

    // Path menu
    add(Gtk::StockItem(OBJECT_TO_PATH, _("_Object to Path")));
    add(Gtk::StockItem(STROKE_TO_PATH, _("_Stroke to Path")));
    add(Gtk::StockItem(TRACE, _("_Trace Bitmap...")));
    add(Gtk::StockItem(UNION, _("_Union")));
    add(Gtk::StockItem(DIFFERENCE, _("_Difference")));
    add(Gtk::StockItem(INTERSECTION, _("_Intersection")));
    add(Gtk::StockItem(EXCLUSION, _("E_xclusion")));
    add(Gtk::StockItem(DIVISION, _("Di_vision")));
    add(Gtk::StockItem(CUT_PATH, _("Cut _Path")));
    add(Gtk::StockItem(COMBINE, _("_Combine")));
    add(Gtk::StockItem(BREAK_APART, _("Break _Apart")));
    add(Gtk::StockItem(INSET, _("I_nset")));
    add(Gtk::StockItem(OUTSET, _("Ou_tset")));
    add(Gtk::StockItem(OFFSET_DYNAMIC, _("D_ynamic Offset")));
    add(Gtk::StockItem(OFFSET_LINKED, _("_Linked Offset")));
    add(Gtk::StockItem(SIMPLIFY, _("Si_mplify")));
    add(Gtk::StockItem(REVERSE, _("_Reverse")));
    //add(Gtk::StockItem(CLEANUP, _("Cl_eanup"))); (using Gtk::Stock::CLEAR)

    // Text menu
    add(Gtk::StockItem(SELECT_FONT, _("_Text and Font...")));
    add(Gtk::StockItem(PUT_ON_PATH, _("_Put Text on Path")));
    add(Gtk::StockItem(REMOVE_FROM_PATH, _("_Remove Text from Path")));
    add(Gtk::StockItem(REMOVE_MANUAL_KERNS, _("Remove Manual _Kerns")));

    // About menu
    add(Gtk::StockItem(KEYS_MOUSE, _("_Keys and Mouse")));
    add(Gtk::StockItem(TUTORIALS, _("_Tutorials")));
    add(Gtk::StockItem(ABOUT, _("_About Inkscape")));

    // Tools toolbox
    add(Gtk::StockItem(TOOL_SELECT, _("Select")));
    add(Gtk::StockItem(TOOL_NODE, _("Node")));
    add(Gtk::StockItem(TOOL_ZOOM, _("Zoom")));
    add(Gtk::StockItem(TOOL_RECT, _("Rectangle")));
    add(Gtk::StockItem(TOOL_ARC, _("Arc")));
    add(Gtk::StockItem(TOOL_STAR, _("Star")));
    add(Gtk::StockItem(TOOL_SPIRAL, _("Spiral")));
    add(Gtk::StockItem(TOOL_FREEHAND, _("Freehand")));
    add(Gtk::StockItem(TOOL_PEN, _("Pen")));
    add(Gtk::StockItem(TOOL_DYNADRAW, _("DynaDraw")));
    add(Gtk::StockItem(TOOL_TEXT, _("Text")));
    add(Gtk::StockItem(TOOL_DROPPER, _("Dropper")));

    // Select Tool controls
    add(Gtk::StockItem(TRANSFORM_STROKE, _("Stroke")));
    add(Gtk::StockItem(TRANSFORM_CORNERS, _("Corners")));
    add(Gtk::StockItem(TRANSFORM_GRADIENT, _("Gradient")));
    add(Gtk::StockItem(TRANSFORM_PATTERN, _("Pattern")));

    // Node Tool controls
    add(Gtk::StockItem(NODE_INSERT, _("Insert")));
    add(Gtk::StockItem(NODE_DELETE, _("Delete")));
    add(Gtk::StockItem(NODE_JOIN, _("Join")));
    add(Gtk::StockItem(NODE_JOIN_SEGMENT, _("Join Segment")));
    add(Gtk::StockItem(NODE_DELETE_SEGMENT, _("Delete Segment")));
    add(Gtk::StockItem(NODE_BREAK, _("Break")));
    add(Gtk::StockItem(NODE_CORNER, _("Corner")));
    add(Gtk::StockItem(NODE_SMOOTH, _("Smooth")));
    add(Gtk::StockItem(NODE_SYMMETRIC, _("Symmetric")));
    add(Gtk::StockItem(NODE_LINE, _("Line")));
    add(Gtk::StockItem(NODE_CURVE, _("Curve")));
}

} // namespace Stock
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
