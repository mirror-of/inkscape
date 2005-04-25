/**
 * \brief This class implements the functionality of the window layout, menus,
 *        and signals.
 * 
 * This is a reimplementation into C++/Gtkmm of Sodipodi's SPDesktop class
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2004 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/action.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/accelmap.h>
#include <glibmm/i18n.h>

#include "path-prefix.h"
#include "edit.h"
#include "ui/stock.h"
#include "ui/stock-items.h"
#include "ui/icons.h"
#include "ui/widget/toolbox.h"

#include "display/sp-canvas.h"
#include "display/sp-canvas-util.h"
#include "display/canvas-arena.h"
#include "display/sodipodi-ctrlrect.h"
#include "display/gnome-canvas-acetate.h"

#include "document.h"
#include "sp-namedview.h"
#include "sp-item.h"
#include "event-context.h"

using namespace Inkscape::UI;
using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace View {

Edit::Edit()
    : _main_window_table(4),
      _viewport_table(3,3),
      _act_grp(Gtk::ActionGroup::create()),
      _ui_mgr(Gtk::UIManager::create())
{
    Icons::init();
    Stock::init();
    initActions();
    initAccelMap();
    initUIManager();
    initLayout();
}

Edit::~Edit()
{
}

void
Edit::initActions()
{
    initMenuActions();
    initToolbarActions();
}

void
Edit::initUIManager()
{
    _ui_mgr->insert_action_group(_act_grp);
    add_accel_group(_ui_mgr->get_accel_group());

    gchar *filename_utf8 = g_build_filename(INKSCAPE_UIDIR, "menus-bars.xml", NULL);
    if (_ui_mgr->add_ui_from_file(filename_utf8) == 0) {
        g_warning("Error merging ui from file '%s'", filename_utf8);
        // fixme-charset: What charset should we pass to g_warning?
    }
    g_free(filename_utf8);
}

void
Edit::initLayout()
{
    set_title("New document 1 - Inkscape");
    set_resizable();
    set_default_size(640, 480);

    // top level window into which all other portions of the UI get inserted
    add(_main_window_table);
    // attach box for horizontal toolbars
    _main_window_table.attach(_toolbars_vbox, 0, 1, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
    // attach sub-window for viewport and vertical toolbars
    _main_window_table.attach(_sub_window_hbox, 0, 1, 2, 3);
    // viewport table with 3 rows by 3 columns
    _sub_window_hbox.pack_end(_viewport_table);

    // Menus and Bars
    initMenuBar();
    initCommandsBar();
    initToolControlsBar();
    initUriBar();
    initToolsBar();

    // Canvas Viewport
    initLeftRuler();
    initTopRuler();
    initSvgCanvas();
    initBottomScrollbar();
    initRightScrollbar();

    // The statusbar comes last and appears at the bottom of _main_window_table
    initStatusbar();

    show_all_children();
}

void
Edit::onMenuItem()
{
    g_warning("onMenuItem called");
}

void
Edit::onActionFileNew()
{
    g_warning("onActionFileNew called");

    _dlg_mgr.hide_dialogs.emit();
}

void
Edit::onActionFileOpen()
{
    g_warning("onActionFileOpen called");

    _dlg_mgr.show_dialogs.emit();
}

void
Edit::onActionFileQuit()
{
    g_warning("onActionFileQuit");
}

void
Edit::onActionFilePrint()
{
    g_warning("onActionFilePrint");
}

void
Edit::onToolbarItem()
{
    g_warning("onToolbarItem called");
}

void
Edit::onSelectTool()
{
    _tool_ctrl->remove();
    _tool_ctrl->add(*_select_ctrl);
}

void
Edit::onNodeTool()
{
    _tool_ctrl->remove();
    _tool_ctrl->add(*_node_ctrl);
}

void
Edit::onDialogInkscapePreferences()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getInkscapePreferencesDialog();
    g_assert(dlg);

    dlg->show();
    dlg->raise();
}

void
Edit::onDialogAbout()
{
/*
    UI::Dialog::Dialog *dlg = _dlg_mgr.getAboutDialog();
    g_assert(dlg);

    dlg->show();
    dlg->raise();
*/
}

void
Edit::onDialogAlignAndDistribute()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getAlignAndDistributeDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogDocumentProperties()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getDocumentPreferencesDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogExport()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getExportDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogExtensionEditor()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getExtensionEditorDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogFillAndStroke()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getFillAndStrokeDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogFind()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getFindDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogLayerEditor()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getLayerEditorDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogMessages()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getMessagesDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogObjectProperties()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getObjectPropertiesDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void 
Edit::onDialogTextProperties()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getTextPropertiesDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onDialogTrace()
{
/*
    UI::Dialog::Dialog *dlg = _dlg_mgr.getTraceDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
*/
}

void 
Edit::onDialogTransformation()
{
    Gtk::Dialog *dlg = _dlg_mgr.getTransformationDialog();
    g_assert(dlg);

    dlg->show();
    dlg->raise();
}

void
Edit::onDialogXmlEditor()
{
    UI::Dialog::Dialog *dlg = _dlg_mgr.getXmlEditorDialog();
    g_assert(dlg);
    
    dlg->show();
    dlg->raise();
}

void
Edit::onUriChanged()
{
    g_message("onUriChanged called");
    
}

void
Edit::initMenuActions()
{
    _act_grp->add(Gtk::Action::create("MenuFile",   _("_File")));
    _act_grp->add(Gtk::Action::create("MenuEdit",   _("_Edit")));
    _act_grp->add(Gtk::Action::create("MenuView",   _("_View")));
    _act_grp->add(Gtk::Action::create("MenuLayer",  _("_Layer")));
    _act_grp->add(Gtk::Action::create("MenuObject", _("_Object")));
    _act_grp->add(Gtk::Action::create("MenuPath",   _("_Path")));
    _act_grp->add(Gtk::Action::create("MenuText",   _("_Text")));
    _act_grp->add(Gtk::Action::create("MenuHelp",   _("_Help")));

    // File menu
    _act_grp->add(Gtk::Action::create("New",
                                      Gtk::Stock::NEW, Glib::ustring(),
                                      _("New")),
                  sigc::mem_fun(*this, &Edit::onActionFileNew));

    _act_grp->add(Gtk::Action::create("Open",
                                      Gtk::Stock::OPEN, Glib::ustring(),
                                      _("Open...")),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("OpenRecent",
                                      Stock::OPEN_RECENT));

    _act_grp->add(Gtk::Action::create("Revert",
                                      Gtk::Stock::REVERT_TO_SAVED, Glib::ustring(),
                                      _("Revert to Saved")),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("Save",
                                      Gtk::Stock::SAVE, Glib::ustring(),
                                      _("Save")),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("SaveAs",
                                      Gtk::Stock::SAVE_AS, Glib::ustring(),
                                      _("Save As...")),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("Import",
                                      Stock::IMPORT, Glib::ustring(),
                                      _("Import...")),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("Export",
                                      Stock::EXPORT, Glib::ustring(),
                                      _("Export...")),
                  sigc::mem_fun(*this, &Edit::onDialogExport));

    _act_grp->add(Gtk::Action::create("Print",
                                      Gtk::Stock::PRINT, Glib::ustring(),
                                      _("Print...")),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("PrintPreview",
                                      Gtk::Stock::PRINT_PREVIEW),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("VacuumDefs",
                                      Stock::VACUUM_DEFS),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("DocumentProperties",
                                      Gtk::Stock::PROPERTIES, Glib::ustring(),
                                      _("Document properties (Shift+Ctrl+D)")),
                  sigc::mem_fun(*this, &Edit::onDialogDocumentProperties));

    _act_grp->add(Gtk::Action::create("InkscapePreferences",
                                      Gtk::Stock::PREFERENCES, Glib::ustring(),
                                      _("Global Inkscape preferences (Shift+Ctrl+P)")),
                  sigc::mem_fun(*this, &Edit::onDialogInkscapePreferences));

    _act_grp->add(Gtk::Action::create("Close",
                                      Gtk::Stock::CLOSE),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("Quit",
                                      Gtk::Stock::QUIT),
                  sigc::mem_fun(*this, &Edit::onActionFileOpen));

    // Edit menu
    _act_grp->add(Gtk::Action::create("Undo",
                                      Gtk::Stock::UNDO, Glib::ustring(),
                                      _("Undo")));

    _act_grp->add(Gtk::Action::create("Redo",
                                      Gtk::Stock::REDO, Glib::ustring(),
                                      _("Redo")));

    _act_grp->add(Gtk::Action::create("Cut",
                                      Gtk::Stock::CUT, Glib::ustring(),
                                      _("Cut")));

    _act_grp->add(Gtk::Action::create("Copy",
                                      Gtk::Stock::COPY, Glib::ustring(),
                                      _("Copy")));

    _act_grp->add(Gtk::Action::create("Paste",
                                      Gtk::Stock::PASTE, Glib::ustring(),
                                      _("Paste")));

    _act_grp->add(Gtk::Action::create("PasteInPlace",
                                      Stock::PASTE_IN_PLACE));

    _act_grp->add(Gtk::Action::create("PasteStyle",
                                      Stock::PASTE_STYLE));

    _act_grp->add(Gtk::Action::create("Find",
                                      Gtk::Stock::FIND),
                  sigc::mem_fun(*this, &Edit::onDialogFind));

    _act_grp->add(Gtk::Action::create("Duplicate",
                                      Stock::DUPLICATE, Glib::ustring(),
                                      _("Duplicate selected object(s) (Ctrl+D)")));

    _act_grp->add(Gtk::Action::create("Clone",
                                      Stock::CLONE, Glib::ustring(),
                                      _("Clone selected object (Alt+D)")));

    _act_grp->add(Gtk::Action::create("CloneUnlink",
                                      Stock::CLONE_UNLINK, Glib::ustring(),
                                      _("Unlink clone from its original (Shift+Alt+D)")));

    _act_grp->add(Gtk::Action::create("CloneSelectOrig",
                                      Stock::CLONE_SELECT_ORIG));

    _act_grp->add(Gtk::Action::create("MakeBitmap",
                                      Stock::MAKE_BITMAP));

    _act_grp->add(Gtk::Action::create("Tile",
                                     Stock::TILE));

    _act_grp->add(Gtk::Action::create("Untile",
                                      Stock::UNTILE));

    _act_grp->add(Gtk::Action::create("Delete",
                                      Gtk::Stock::DELETE));

    _act_grp->add(Gtk::Action::create("SelectAll",
                                      Stock::SELECT_ALL));

    _act_grp->add(Gtk::Action::create("SelectAllInAllLayers",
                                      Stock::SELECT_ALL_IN_ALL_LAYERS));

    _act_grp->add(Gtk::Action::create("SelectInvert",
                                      Stock::SELECT_INVERT));

    _act_grp->add(Gtk::Action::create("SelectNone",
                                      Stock::SELECT_NONE));

    _act_grp->add(Gtk::Action::create("XmlEditor",
                                      Stock::XML_EDITOR, Glib::ustring(),
                                      _("XML Editor dialog (Shift+Ctrl+X)")),
                  sigc::mem_fun(*this, &Edit::onDialogXmlEditor));

    // View menu
    _act_grp->add(Gtk::Action::create("Zoom",
                                      Stock::ZOOM));

    _act_grp->add(Gtk::Action::create("ZoomIn",
                                      Stock::ZOOM_IN, Glib::ustring(),
                                      _("Zoom in (+)")));

    _act_grp->add(Gtk::Action::create("ZoomOut",
                                      Stock::ZOOM_OUT, Glib::ustring(),
                                      _("Zoom out (-)")));

    _act_grp->add(Gtk::Action::create("Zoom100",
                                      Stock::ZOOM_100, Glib::ustring(),
                                      _("Zoom to 1:1 (100%) (1)")));

    _act_grp->add(Gtk::Action::create("Zoom50",
                                      Stock::ZOOM_50, Glib::ustring(),
                                      _("Zoom to 1:2 (50%) (2)")));

    _act_grp->add(Gtk::Action::create("Zoom200",
                                      Stock::ZOOM_200, Glib::ustring(),
                                      _("Zoom to 2:1 (200%) (0)")));

    _act_grp->add(Gtk::Action::create("ZoomSelection",
                                      Stock::ZOOM_SELECTION, Glib::ustring(),
                                      _("Zoom to fit selection in window (3)")));

    _act_grp->add(Gtk::Action::create("ZoomDrawing",
                                      Stock::ZOOM_DRAWING, Glib::ustring(),
                                      _("Zoom to fit drawing in window (4)")));

    _act_grp->add(Gtk::Action::create("ZoomPage",
                                      Stock::ZOOM_PAGE, Glib::ustring(),
                                      _("Zoom to fit page in window (5)")));

    _act_grp->add(Gtk::Action::create("ZoomWidth",
                                      Stock::ZOOM_WIDTH, Glib::ustring(),
                                      _("Zoom to fit page width in window (6)")));

    _act_grp->add(Gtk::Action::create("ZoomPrev",
                                      Stock::ZOOM_PREV, Glib::ustring(),
                                      _("Previous zoom (from history of zooms) (`)")));

    _act_grp->add(Gtk::Action::create("ZoomNext",
                                      Stock::ZOOM_NEXT, Glib::ustring(),
                                      _("Next zoom (from history of zooms) (Shift+`)")));

    _act_grp->add(Gtk::Action::create("ShowHide",
                                      Stock::SHOW_HIDE));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideCommandsBar",
                                            Stock::SHOW_HIDE_COMMANDS_BAR));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideToolControlsBar",
                                            Stock::SHOW_HIDE_TOOL_CONTROLS_BAR));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideToolsBar",
                                            Stock::SHOW_HIDE_TOOLS_BAR));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideRulers",
                                            Stock::SHOW_HIDE_RULERS));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideScrollbars",
                                            Stock::SHOW_HIDE_SCROLLBARS));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideStatusbar",
                                            Stock::SHOW_HIDE_STATUSBAR));

    _act_grp->add(Gtk::Action::create("ShowHideDialogs",
                                      Stock::SHOW_HIDE_DIALOGS));

    _act_grp->add(Gtk::Action::create("Grid",
                                      Stock::GRID));

    _act_grp->add(Gtk::Action::create("Guides",
                                      Stock::GUIDES));

    _act_grp->add(Gtk::Action::create("Fullscreen",
                                      Stock::FULLSCREEN));

    _act_grp->add(Gtk::Action::create("Messages",
                                      Stock::MESSAGES),
                  sigc::mem_fun(*this, &Edit::onDialogMessages));

    _act_grp->add(Gtk::Action::create("Scripts",
                                      Stock::SCRIPTS));

    _act_grp->add(Gtk::Action::create("WindowPrev",
                                      Stock::WINDOW_PREV));

    _act_grp->add(Gtk::Action::create("WindowNext",
                                      Stock::WINDOW_NEXT));

    _act_grp->add(Gtk::Action::create("WindowDuplicate",
                                      Stock::WINDOW_DUPLICATE));

    // Layer menu
    _act_grp->add(Gtk::Action::create("LayerNew",
                                      Stock::LAYER_NEW));

    _act_grp->add(Gtk::Action::create("LayerRename",
                                      Stock::LAYER_RENAME));

    _act_grp->add(Gtk::Action::create("LayerDuplicate",
                                      Stock::LAYER_DUPLICATE));

    _act_grp->add(Gtk::Action::create("LayerAnchor",
                                      Stock::LAYER_ANCHOR));

    _act_grp->add(Gtk::Action::create("LayerMergeDown",
                                      Stock::LAYER_MERGE_DOWN));

    _act_grp->add(Gtk::Action::create("LayerDelete",
                                      Stock::LAYER_DELETE));

    _act_grp->add(Gtk::Action::create("LayerSelectNext",
                                      Stock::LAYER_SELECT_NEXT));

    _act_grp->add(Gtk::Action::create("LayerSelectPrev",
                                      Stock::LAYER_SELECT_PREV));

    _act_grp->add(Gtk::Action::create("LayerSelectTop",
                                      Stock::LAYER_SELECT_TOP));

    _act_grp->add(Gtk::Action::create("LayerSelectBottom",
                                      Stock::LAYER_SELECT_BOTTOM));

    _act_grp->add(Gtk::Action::create("LayerRaise",
                                      Stock::LAYER_RAISE));

    _act_grp->add(Gtk::Action::create("LayerLower",
                                      Stock::LAYER_LOWER));

    _act_grp->add(Gtk::Action::create("LayerToTop",
                                      Stock::LAYER_TO_TOP));

    _act_grp->add(Gtk::Action::create("LayerToBottom",
                                      Stock::LAYER_TO_BOTTOM));

    // Object menu
    _act_grp->add(Gtk::Action::create("FillAndStroke",
                                      Stock::FILL_STROKE, Glib::ustring(),
                                      _("Fill and Stroke dialog (Shift+Ctrl+F)")),
                  sigc::mem_fun(*this, &Edit::onDialogFillAndStroke));

    _act_grp->add(Gtk::Action::create("ObjectProperties",
                                      Stock::OBJECT_PROPERTIES),
                  sigc::mem_fun(*this, &Edit::onDialogObjectProperties));

    _act_grp->add(Gtk::Action::create("Group",
                                      Stock::GROUP, Glib::ustring(),
                                      _("Group selected objects (Ctrl+G)")));

    _act_grp->add(Gtk::Action::create("Ungroup",
                                      Stock::UNGROUP, Glib::ustring(),
                                      _("Ungroup selected group(s) (Ctrl+U)")));

    _act_grp->add(Gtk::Action::create("Raise",
                                      Stock::RAISE, Glib::ustring(),
                                      _("Raise selection up one step (PgUp)")));

    _act_grp->add(Gtk::Action::create("Lower",
                                      Stock::LOWER, Glib::ustring(),
                                      _("Lower selection down one step (PgDn)")));

    _act_grp->add(Gtk::Action::create("RaiseToTop",
                                      Stock::RAISE_TO_TOP, Glib::ustring(),
                                      _("Raise selection to top (Home)")));

    _act_grp->add(Gtk::Action::create("LowerToBottom",
                                      Stock::LOWER_TO_BOTTOM, Glib::ustring(),
                                      _("Lower selection to bottom (End)")));

    _act_grp->add(Gtk::Action::create("MoveToNewLayer",
                                      Stock::MOVE_TO_NEW_LAYER, Glib::ustring(),
                                      _("Move selection to new layer")));

    _act_grp->add(Gtk::Action::create("MoveToNextLayer",
                                      Stock::MOVE_TO_NEXT_LAYER, Glib::ustring(),
                                      _("Move selection to next layer")));

    _act_grp->add(Gtk::Action::create("MoveToPrevLayer",
                                      Stock::MOVE_TO_PREV_LAYER, Glib::ustring(),
                                      _("Move selection to previous layer")));

    _act_grp->add(Gtk::Action::create("MoveToTopLayer",
                                      Stock::MOVE_TO_TOP_LAYER, Glib::ustring(),
                                      _("Move selection to top layer")));

    _act_grp->add(Gtk::Action::create("MoveToBottomLayer",
                                      Stock::MOVE_TO_BOTTOM_LAYER, Glib::ustring(),
                                      _("Move selection to bottom layer")));

    _act_grp->add(Gtk::Action::create("Rotate90CW",
                                      Stock::ROTATE_90_CW, Glib::ustring(),
                                      _("Rotate selection 90° clockwise (Shift+Ctrl+Right)")));

    _act_grp->add(Gtk::Action::create("Rotate90CCW",
                                      Stock::ROTATE_90_CCW, Glib::ustring(),
                                      _("Rotate selection 90° counter-clockwise (Shift+Ctrl+Left)")));

    _act_grp->add(Gtk::Action::create("FlipHoriz",
                                      Stock::FLIP_HORIZ, Glib::ustring(),
                                      _("Flip selection horizontally (H)")));

    _act_grp->add(Gtk::Action::create("FlipVert",
                                      Stock::FLIP_VERT, Glib::ustring(),
                                      _("Flip selection vertically (V)")));

    _act_grp->add(Gtk::Action::create("Transformation",
                                      Stock::TRANSFORMATION, Glib::ustring(),
                                      _("Transformation")),
                  sigc::mem_fun(*this, &Edit::onDialogTransformation));

    _act_grp->add(Gtk::Action::create("AlignAndDistribute",
                                      Stock::ALIGN_DISTRIBUTE, Glib::ustring(),
                                      _("Align and Distribute dialog (Shift+Ctrl+A)")),
                  sigc::mem_fun(*this, &Edit::onDialogAlignAndDistribute));

    // Path menu
    _act_grp->add(Gtk::Action::create("ObjectToPath",
                                      Stock::OBJECT_TO_PATH, Glib::ustring(),
                                      _("Convert selected object(s) to path(s) (Shift+Ctrl+C)")));

    _act_grp->add(Gtk::Action::create("StrokeToPath",
                                      Stock::STROKE_TO_PATH, Glib::ustring(),
                                      _("Convert selected stroke(s) to path(s) (Ctrl+Alt+C)")));

    _act_grp->add(Gtk::Action::create("Trace",
                                      Stock::TRACE),
                  sigc::mem_fun(*this, &Edit::onDialogTrace));

    _act_grp->add(Gtk::Action::create("Union",
                                      Stock::UNION));

    _act_grp->add(Gtk::Action::create("Difference",
                                      Stock::DIFFERENCE));

    _act_grp->add(Gtk::Action::create("Intersection",
                                      Stock::INTERSECTION));

    _act_grp->add(Gtk::Action::create("Exclusion",
                                      Stock::EXCLUSION));

    _act_grp->add(Gtk::Action::create("Division",
                                      Stock::DIVISION));

    _act_grp->add(Gtk::Action::create("CutPath",
                                      Stock::CUT_PATH));

    _act_grp->add(Gtk::Action::create("Combine",
                                      Stock::COMBINE));

    _act_grp->add(Gtk::Action::create("BreakApart",
                                      Stock::BREAK_APART));

    _act_grp->add(Gtk::Action::create("Inset",
                                      Stock::INSET));

    _act_grp->add(Gtk::Action::create("Outset",
                                      Stock::OUTSET));

    _act_grp->add(Gtk::Action::create("OffsetDynamic",
                                      Stock::OFFSET_DYNAMIC));

    _act_grp->add(Gtk::Action::create("OffsetLinked",
                                      Stock::OFFSET_LINKED));

    _act_grp->add(Gtk::Action::create("Simplify",
                                      Stock::SIMPLIFY));

    _act_grp->add(Gtk::Action::create("Reverse",
                                      Stock::REVERSE));

    _act_grp->add(Gtk::Action::create("Cleanup",
                                      Gtk::Stock::CLEAR,
                                      _("Cl_eanup")));

    // Text menu
    _act_grp->add(Gtk::Action::create("TextProperties",
                                      Gtk::Stock::SELECT_FONT, Glib::ustring(),
                                      _("Text and Font dialog (Shift+Ctrl+T)")),
                  sigc::mem_fun(*this, &Edit::onDialogTextProperties));

    _act_grp->add(Gtk::Action::create("PutOnPath",
                                      Stock::PUT_ON_PATH));

    _act_grp->add(Gtk::Action::create("RemoveFromPath",
                                      Stock::REMOVE_FROM_PATH));

    _act_grp->add(Gtk::Action::create("RemoveManualKerns",
                                      Stock::REMOVE_MANUAL_KERNS));

    // About menu
    _act_grp->add(Gtk::Action::create("KeysAndMouse",
                                      Stock::KEYS_MOUSE));

    _act_grp->add(Gtk::Action::create("Tutorials",
                                      Stock::TUTORIALS));

    _act_grp->add(Gtk::Action::create("About",
                                      Stock::ABOUT),
                  sigc::mem_fun(*this, &Edit::onDialogAbout));
}

void
Edit::initToolbarActions()
{
    // Tools bar
    Gtk::RadioAction::Group tools;

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolSelect",
                                           Stock::TOOL_SELECT, Glib::ustring(),
                                           _("Select tool")),
                  sigc::mem_fun(*this, &Edit::onSelectTool));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolNode",
                                           Stock::TOOL_NODE, Glib::ustring(),
                                           _("Node tool")),
                  sigc::mem_fun(*this, &Edit::onNodeTool));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolZoom",
                                           Stock::TOOL_ZOOM, Glib::ustring(),
                                           _("Zoom tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolRect",
                                           Stock::TOOL_RECT, Glib::ustring(),
                                           _("Rectangle tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolArc",
                                           Stock::TOOL_ARC, Glib::ustring(),
                                           _("Arc tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolStar",
                                           Stock::TOOL_STAR, Glib::ustring(),
                                           _("Star tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolSpiral",
                                           Stock::TOOL_SPIRAL, Glib::ustring(),
                                           _("Spiral tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolFreehand",
                                           Stock::TOOL_FREEHAND, Glib::ustring(),
                                           _("Freehand tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolPen",
                                           Stock::TOOL_PEN, Glib::ustring(),
                                           _("Pen tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolDynaDraw",
                                           Stock::TOOL_DYNADRAW, Glib::ustring(),
                                           _("Calligraphy tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolText",
                                           Stock::TOOL_TEXT, Glib::ustring(),
                                           _("Text tool")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolDropper",
                                           Stock::TOOL_DROPPER, Glib::ustring(),
                                           _("Dropper tool")));

    // Select Controls bar
    _act_grp->add(Gtk::ToggleAction::create("TransformStroke",
                                            Stock::TRANSFORM_STROKE, Glib::ustring(),
                                            _("When scaling objects, scale stroke width by same proportion")));

    _act_grp->add(Gtk::ToggleAction::create("TransformCorners",
                                            Stock::TRANSFORM_CORNERS, Glib::ustring(),
                                            _("When scaling rectangles, scale radii of rounded corners")));

    _act_grp->add(Gtk::ToggleAction::create("TransformGradient",
                                            Stock::TRANSFORM_GRADIENT, Glib::ustring(),
                                            _("Transform gradients (in fill or stroke) along with objects")));

    _act_grp->add(Gtk::ToggleAction::create("TransformPattern",
                                            Stock::TRANSFORM_PATTERN, Glib::ustring(),
                                            _("Transform patterns (in fill or stroke) along with objects")));

    // Node Controls bar
    _act_grp->add(Gtk::Action::create("NodeInsert",
                                      Stock::NODE_INSERT, Glib::ustring(),
                                      _("Insert new nodes into selected segments")));

    _act_grp->add(Gtk::Action::create("NodeDelete",
                                      Stock::NODE_DELETE, Glib::ustring(),
                                      _("Delete selected nodes")));

    _act_grp->add(Gtk::Action::create("NodeJoin",
                                      Stock::NODE_JOIN, Glib::ustring(),
                                      _("Join paths at selected nodes")));

    _act_grp->add(Gtk::Action::create("NodeJoinSegment",
                                      Stock::NODE_JOIN_SEGMENT, Glib::ustring(),
                                      _("Join paths at selected nodes with new segment")));

    _act_grp->add(Gtk::Action::create("NodeDeleteSegment",
                                      Stock::NODE_DELETE_SEGMENT, Glib::ustring(),
                                      _("Delete segment between two nodes")));

    _act_grp->add(Gtk::Action::create("NodeBreak",
                                      Stock::NODE_BREAK, Glib::ustring(),
                                      _("Break path at selected nodes")));

    _act_grp->add(Gtk::Action::create("NodeCorner",
                                      Stock::NODE_CORNER, Glib::ustring(),
                                      _("Make selected nodes corner")));

    _act_grp->add(Gtk::Action::create("NodeSmooth",
                                      Stock::NODE_SMOOTH, Glib::ustring(),
                                      _("Make selected nodes smooth")));

    _act_grp->add(Gtk::Action::create("NodeSymmetric",
                                      Stock::NODE_SYMMETRIC, Glib::ustring(),
                                      _("Make selected nodes symmetric")));

    _act_grp->add(Gtk::Action::create("NodeLine",
                                      Stock::NODE_LINE, Glib::ustring(),
                                      _("Make selected segments lines")));

    _act_grp->add(Gtk::Action::create("NodeCurve",
                                      Stock::NODE_CURVE, Glib::ustring(),
                                      _("Make selected segments curves")));
}

void
Edit::initAccelMap()
{
    gchar *filename = g_build_filename(INKSCAPE_UIDIR, "keybindings.rc", NULL);
    Gtk::AccelMap::load(filename);
    g_free(filename);
}

void
Edit::initMenuBar()
{
    g_assert(_ui_mgr);
    Gtk::MenuBar *menu = static_cast<Gtk::MenuBar*>(_ui_mgr->get_widget("/MenuBar"));
    g_assert(menu != NULL);
    _main_window_table.attach(*Gtk::manage(menu), 0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
}

void
Edit::initCommandsBar()
{
    g_assert(_ui_mgr);
    Toolbox *bar = new Toolbox(static_cast<Gtk::Toolbar*>(_ui_mgr->get_widget("/CommandsBar")),
                               Gtk::TOOLBAR_ICONS);
    g_assert(bar != NULL);
    _toolbars_vbox.pack_start(*Gtk::manage(bar), Gtk::PACK_SHRINK);
}

void
Edit::initToolControlsBar()
{
    // TODO: Do UIManager controlled widgets need to be deleted?
    _select_ctrl = static_cast<Gtk::Toolbar*>(_ui_mgr->get_widget("/SelectControlsBar"));
    _node_ctrl = static_cast<Gtk::Toolbar*>(_ui_mgr->get_widget("/NodeControlsBar"));

    _tool_ctrl = new Toolbox(_select_ctrl, Gtk::TOOLBAR_ICONS);

    _toolbars_vbox.pack_start(*Gtk::manage(_tool_ctrl), Gtk::PACK_SHRINK);
}

void
Edit::initUriBar()
{
    // TODO:  Create an Inkscape::UI::Widget::UriBar class (?)

    _uri_ctrl = new Gtk::Toolbar();

    _uri_label.set_label(_("URI:"));
    _uri_ctrl->add(_uri_label);
    _uri_ctrl->add(_uri_entry);

    _uri_entry.signal_activate()
        .connect_notify(sigc::mem_fun(*this, &Edit::onUriChanged));

    _toolbars_vbox.pack_start(*Gtk::manage(_uri_ctrl), Gtk::PACK_SHRINK);
}

void
Edit::initToolsBar()
{
    Toolbox *bar = new Toolbox(static_cast<Gtk::Toolbar*>(_ui_mgr->get_widget("/ToolsBar")),
                               Gtk::TOOLBAR_ICONS,
                               Gtk::ORIENTATION_VERTICAL);
    g_assert(bar != NULL);
    _sub_window_hbox.pack_start(*Gtk::manage(bar), Gtk::PACK_SHRINK);
}

void
Edit::initTopRuler()
{
    _viewport_table.attach(_top_ruler,  1, 2, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
    _top_ruler.set_metric(Gtk::PIXELS);
    _top_ruler.set_range(0, 1000, 500, 1000);
}

void
Edit::initLeftRuler()
{
    _viewport_table.attach(_left_ruler, 0, 1, 1, 2, Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);
    _left_ruler.set_metric(Gtk::PIXELS);
    _left_ruler.set_range(0,1000,500,1000);
}

void
Edit::initBottomScrollbar()
{
    _viewport_table.attach(_bottom_scrollbar, 1, 2, 2, 3, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
}

void
Edit::initRightScrollbar()
{
    _viewport_table.attach(_right_scrollbar, 2, 3, 1, 2, Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);
}

int editor_root_handler(SPCanvasItem *item, GdkEvent *event, Edit *editor) {
    return sp_event_context_root_handler(editor->event_context, event);
}

int editor_item_handler(SPCanvasItem *item, GdkEvent *event, gpointer data) {
    gpointer ddata = gtk_object_get_data (GTK_OBJECT (item->canvas), "Editor");
    g_return_val_if_fail (ddata != NULL, FALSE);

    Edit *editor = (Edit*)ddata;

    return sp_event_context_item_handler (editor->event_context, SP_ITEM (data), event);
}

int editor_arena_handler(SPCanvasItem *arena, NRArenaItem *ai, GdkEvent *event, Edit *editor) {
    if (ai) {
        SPItem *spi = (SPItem*)NR_ARENA_ITEM_GET_DATA (ai);
        return sp_event_context_item_handler (editor->event_context, spi, event);
    } else {
        return sp_event_context_root_handler (editor->event_context, event);
    }
}

void editor_namedview_modified(SPNamedView *nv, guint flags, Edit *editor) {
    /* TODO 
    if (flags & SP_OBJECT_MODIFIED_FLAG) {

        // Recalculate snap distances
        sp_dt_update_snap_distances (desktop);

        // Show/hide page background
        if (nv->pagecolor & 0xff) {
            sp_canvas_item_show (desktop->table);
            sp_ctrlrect_set_color ((SPCtrlRect *) desktop->table, 0x00000000,
                                   TRUE, nv->pagecolor);
            sp_canvas_item_move_to_z (desktop->table, 0);
        } else {
            sp_canvas_item_hide (desktop->table);
        }
        // Show/hide page border
        if (nv->showborder) {
            // show
            sp_canvas_item_show (desktop->page_border);
            // set color and shadow
            sp_ctrlrect_set_color ((SPCtrlRect *) desktop->page_border,
                                   nv->bordercolor, FALSE, 0x00000000);
            if (nv->pageshadow)
                sp_ctrlrect_set_shadow ((SPCtrlRect *)desktop->page_border,
                                        nv->pageshadow, nv->bordercolor);
            // place in the z-order stack
            if (nv->borderlayer == SP_BORDER_LAYER_BOTTOM) {
                sp_canvas_item_move_to_z (desktop->page_border, 2);
            } else {
                int order = sp_canvas_item_order (desktop->page_border);
                int morder = sp_canvas_item_order (desktop->drawing);
                if (morder > order) sp_canvas_item_raise (desktop->page_border,
                                                          morder - order);
            }
        } else {
            sp_canvas_item_hide (desktop->page_border);
            if (nv->pageshadow)
                sp_ctrlrect_set_shadow ((SPCtrlRect *)desktop->page, 0,
                                        0x00000000);
        }

        // Show/hide page shadow
        if (nv->showpageshadow && nv->pageshadow) {
            // show
            sp_ctrlrect_set_shadow ((SPCtrlRect *)desktop->page_border,
                                    nv->pageshadow, nv->bordercolor);
        } else {
            // hide
            sp_ctrlrect_set_shadow ((SPCtrlRect *)desktop->page_border, 0,
                                    0x00000000);
        }
    }
    */
}


/** Replaces sp_desktop_init() and sp_desktop_new() from the Gtk+ codebase
 */
void
Edit::initSvgCanvas()
{
    GtkWidget* canvas = (GtkWidget*)gtk_type_new (sp_canvas_get_type ());
    _svg_canvas = Glib::wrap(canvas);
    _svg_canvas->set_flags(Gtk::CAN_FOCUS);

    // Set background to white
    Glib::RefPtr<Gtk::Style> style = _svg_canvas->get_style();
    style->set_bg(Gtk::STATE_NORMAL, style->get_white());
    _svg_canvas->set_style(style);

    // Connect document
    setDocument(_document);
    _namedview = sp_document_namedview(_document, NULL);
    
/* TODO - Gtkmmify
    g_signal_connect(G_OBJECT(_namedview), "modified", G_CALLBACK(sp_dt_namedview_modified), desktop);
*/
    _number = sp_namedview_viewcount(_namedview);

    SPCanvasGroup *root = sp_canvas_root((SPCanvas*)canvas);

    // Setup the infinite-dimensioned 'Acetate' object (a SPCanvasItem)
    _acetate = sp_canvas_item_new(root, GNOME_TYPE_CANVAS_ACETATE, NULL);
    g_signal_connect(G_OBJECT(_acetate), "event", G_CALLBACK(editor_root_handler), this);
    _main = (SPCanvasGroup *) sp_canvas_item_new(root, SP_TYPE_CANVAS_GROUP, NULL);
/*
    g_signal_connect(G_OBJECT(_main), "event", G_CALLBACK(sp_desktop_root_handler), desktop);
*/

    // Set up the 'table' SPCanvasItem for outside-of-page background
    _table = sp_canvas_item_new (_main, SP_TYPE_CTRLRECT, NULL);
    sp_ctrlrect_set_area (SP_CTRLRECT (_table), -15000.0, -15000.0, 15000.0, 15000.0);
    sp_ctrlrect_set_color (SP_CTRLRECT (_table), 0x00000000, TRUE, 0x00000000);
    sp_canvas_item_move_to_z (_table, 0);

    // Set up the page for the inside-of-page background
    _page = sp_canvas_item_new (_main, SP_TYPE_CTRLRECT, NULL);
    sp_ctrlrect_set_color ((SPCtrlRect *) _page, 0x00000000, FALSE, 0x00000000);
    _page_border = sp_canvas_item_new (_main, SP_TYPE_CTRLRECT, NULL);

    // Set up the drawing SPCanvasItem
    _drawing = sp_canvas_item_new (_main, SP_TYPE_CANVAS_ARENA, NULL);
/* Gtkmmify
    g_signal_connect (G_OBJECT (_drawing), "arena_event", G_CALLBACK (arena_handler), desktop);
*/
    // Set up the grid, guides, sketc, and controls
    _grid = (SPCanvasGroup *) sp_canvas_item_new (_main, SP_TYPE_CANVAS_GROUP, NULL);
    _guides = (SPCanvasGroup *) sp_canvas_item_new (_main, SP_TYPE_CANVAS_GROUP, NULL);
    _sketch = (SPCanvasGroup *) sp_canvas_item_new (_main, SP_TYPE_CANVAS_GROUP, NULL);
    _controls = (SPCanvasGroup *) sp_canvas_item_new (_main, SP_TYPE_CANVAS_GROUP, NULL);

/*
    // Create the Selection object
    _selection = new Inkscape::Selection (desktop);

    // Push select tool to the bottom of stack
    // FIXME: this is the only call to this.  Everything else seems to just
    // call "set" instead of "push".  Can we assume that there is only one
    // context ever?
    sp_desktop_push_event_context (desktop, SP_TYPE_SELECT_CONTEXT, "tools.select", SP_EVENT_CONTEXT_STATIC);
*/

    // display rect and zoom are now handled in sp_desktop_widget_realize()
    sp_ctrlrect_set_area (SP_CTRLRECT (_page), 0.0, 0.0, sp_document_width (_document), sp_document_height (_document));
    sp_ctrlrect_set_area (SP_CTRLRECT (_page_border), 0.0, 0.0, sp_document_width (_document), sp_document_height (_document));

    // the following sets the page shadow on the canvas
    // It was originally set to 5, which is really cheesy!
    // It now is an attribute in the document's namedview. If a value of
    // 0 is used, then the constructor for a shadow is not initialized.
    if ( _namedview->pageshadow != 0 &&
         _namedview->showpageshadow ) {
        sp_ctrlrect_set_shadow (SP_CTRLRECT (_page_border),
                                _namedview->pageshadow, 0x3f3f3fff);
    }

/** TODO
    // Connect event for page resize
    _doc2dt[5] = sp_document_height (document);
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (_drawing), _doc2dt);

    _sel_modified_connection.disconnect();
    _sel_modified_connection = _selection->connectModified(
        sigc::bind(
            sigc::ptr_fun(&sp_desktop_selection_modified),
            desktop
            )
        );

    _sel_changed_connection.disconnect();
    _sel_changed_connection = desktop->selection->connectChanged(
        sigc::bind(
            sigc::ptr_fun(&SPDesktop::_selection_changed),
            desktop
            )
        );
*/

    // Display the arena portion of the document
    NRArenaItem *ai = sp_item_invoke_show (SP_ITEM (sp_document_root (_document)),
                                           SP_CANVAS_ARENA (_drawing)->arena, _dkey, SP_ITEM_SHOW_DISPLAY);
    if (ai) {
        nr_arena_item_add_child (SP_CANVAS_ARENA (_drawing)->root, ai, NULL);
        nr_arena_item_unref (ai);
    }

/*
    sp_namedview_show (_namedview, desktop);

    // Ugly hack
    sp_desktop_activate_guides (desktop, TRUE);
    // Ugly hack
    sp_dt_namedview_modified (_namedview, SP_OBJECT_MODIFIED_FLAG, desktop);

    // ?
    // sp_active_desktop_set (desktop);
    inkscape_add_desktop (desktop);

*/

    _viewport_table.attach(*_svg_canvas, 1, 2, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);

}

void
Edit::initStatusbar()
{
    _main_window_table.attach(_statusbar, 0, 1, 3, 4, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
}

} // namespace View
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
