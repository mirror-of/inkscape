/**
 * \brief Application Implementation class declaration for Inkscape.  This
 *        class implements the functionality of the window layout, menus,
 *        and signals.
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

// TODO:  This stuff should mostly be moved into Inkscape::UI::View::Edit

#ifndef INKSCAPE_APPLICATION_EDITOR_IMPL_H
#define INKSCAPE_APPLICATION_EDITOR_IMPL_H

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/ruler.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/drawingarea.h>  // TODO: remove this when SVG Canvas is hooked in

#include "ui/dialog/dialog-manager.h"
#include "ui/widget/toolbox.h"

namespace Inkscape {
namespace Application {

class Editor;

class Editor::EditorImpl : public Gtk::Window {
public:
    EditorImpl();
    ~EditorImpl();

    // Initialization
    void initActions();
    void initUIManager();
    void initLayout();

    // Actions
    void onActionFileNew();
    void onActionFileOpen();
    void onActionFilePrint();
    void onActionFileQuit();
    void onToolbarItem();
    void onSelectTool();
    void onNodeTool();

    // Menus
    void onMenuItem();

    void onDialogAbout();
    void onDialogAlignAndDistribute();
    void onDialogInkscapePreferences();
    void onDialogDocumentProperties();
    void onDialogExport();
    void onDialogExtensionEditor();
    void onDialogFillAndStroke();
    void onDialogFind();
    void onDialogLayerEditor();
    void onDialogMessages();
    void onDialogObjectProperties();
    void onDialogTextProperties();
    void onDialogTransform();
    void onDialogTransformation();
    void onDialogTrace();
    void onDialogXmlEditor();

protected:
    // Child widgets:
    Gtk::Table           _main_window_table;
    Gtk::VBox            _toolbars_vbox;
    Gtk::HBox            _sub_window_hbox;
    Gtk::Table           _viewport_table;

    UI::Widget::Toolbox  *_tool_ctrl;
    Gtk::Toolbar         *_select_ctrl;
    Gtk::Toolbar         *_node_ctrl;

    Gtk::HRuler          _top_ruler;
    Gtk::VRuler          _left_ruler;
    Gtk::HScrollbar      _bottom_scrollbar;
    Gtk::VScrollbar      _right_scrollbar;
    Gtk::DrawingArea     _svg_canvas;
    Gtk::Statusbar       _statusbar;

    Glib::RefPtr<Gtk::ActionGroup>  _act_grp;
    Glib::RefPtr<Gtk::UIManager>    _ui_mgr;
    UI::Dialog::DialogManager       _dlg_mgr;

    void initMenuActions();
    void initToolbarActions();
    void initAccelMap();
    void initMenuBar();
    void initCommandsBar();
    void initToolControlsBar();
    void initToolsBar();
    void initBottomScrollbar();
    void initRightScrollbar();
    void initLeftRuler();
    void initTopRuler();
    void initSvgCanvas();
    void initStatusbar();
};

} // namespace Application
} // namespace Inkscape

#endif // INKSCAPE_APPLICATION_EDITOR_IMPL_H

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
